// #include "./regfile.c"
#include <math.h> // log2()
#include <stdbool.h>
#define DC_NUM_SETS 8
#define DC_SET_SIZE 2
#define DC_BLOCK_SIZE 32
#define DC_WR_BUFF_SIZE 4
#define DC_INVALID 0
#define DC_VALID 1
#define DC_DIRTY 2
#define MEM_RD_LATENCY 25
#define MEM_WR_LATENCY 50
#define WR_BUFF_WR_LATENCY 5
#define WR_BUFF_FLUSH_LATENCY 50

extern int pipeline_cycle_count;
int lruTime = 0;

struct cacheBlk {
  int tag;
  int status;
  int trdy;
} dCache[DC_NUM_SETS][DC_SET_SIZE];

struct writeBufferCell {
  int tag;
  int trdy;
} dcWrBuff[DC_WR_BUFF_SIZE];

int wrBack(uqword tag, int time) {
  int latency = 0;
  bool found = false;

  // 检查写缓冲区是否命中
  for (int i = 0; i < DC_WR_BUFF_SIZE; i++) {
    if (dcWrBuff[i].tag == tag) {
      // 写缓冲命中，开销为0
      found = true;
      latency = 0;
      break;
    }
  }

  // 若未命中，寻找空闲空间或清空写缓冲
  if (!found) {
    bool spaceFound = false;

    for (int i = 0; i < DC_WR_BUFF_SIZE; i++) {
      if (dcWrBuff[i].tag == -1) { // -1 表示此位置空闲
        // 存储到空闲位置
        dcWrBuff[i].tag = tag;
        dcWrBuff[i].trdy = time + WR_BUFF_WR_LATENCY;
        latency = WR_BUFF_WR_LATENCY;
        spaceFound = true;
        break;
      }
    }

    // 若写缓冲已满，则清空缓冲，并写入新的数据
    if (!spaceFound) {
      // 先将写缓冲的内容全部写回主存
      for (int i = 0; i < DC_WR_BUFF_SIZE; i++) {
        dcWrBuff[i].tag = -1;
      }
      // 添加清空的延迟
      latency = WR_BUFF_FLUSH_LATENCY + WR_BUFF_WR_LATENCY;

      // 将待写入内容写入写缓冲
      dcWrBuff[0].tag = tag;
      dcWrBuff[0].trdy = time + latency;
    }
  }

  return latency;
}

int accessDCache(int opcode, int addr, int time) {
  uqword blkOffsetBits = (uqword)log2(DC_BLOCK_SIZE);
  uqword indexMask = (unsigned)(DC_NUM_SETS - 1);
  uqword tagMask = ~indexMask;
  uqword blk = ((unsigned)addr) >> blkOffsetBits;
  uqword index = (int)(blk & indexMask);
  uqword tag = (int)(blk & tagMask);

  int slot, lruSlot;
  lruTime = time;

  bool hit = false;
  for (int i = 0; i < DC_SET_SIZE; i++) {
    if ((dCache[index][i].tag == tag) &&
        (dCache[index][i].status != DC_INVALID)) {
      slot = i;
      hit = true;
      break;
    } else /* Find a possible replacement line */
      if (dCache[index][i].trdy < lruTime) {
        lruTime = dCache[index][i].trdy;
        lruSlot = i;
      }
  }
  int trdy;
  if (hit) {
    struct cacheBlk *dcBlock = &(dCache[index][slot]);
    dcBlock->trdy = pipeline_cycle_count;
    if (opcode == 41 || opcode == 49) // lw(41) and loadf(49)
    {
      ;
    } else if (opcode == 57) // storef(57)
    {
      dcBlock->status = DC_DIRTY;
    } else {
      printf("Error: unknown cache-access op opcode: %d :hit\n", opcode);
      exit(-1);
    }
  } else {
    if (opcode == 41 || opcode == 49) // lw(41) and loadf(49) 读不命中
    {
      struct cacheBlk *dcBlock = &(dCache[index][lruSlot]);
      trdy = MEM_RD_LATENCY;
      if (dcBlock->status == DC_DIRTY) // 如果被换出的块为脏块
        // trdy += wrBack(tag, time);
        trdy += WR_BUFF_FLUSH_LATENCY;
      dcBlock->tag = tag;
      dcBlock->trdy = time + trdy;
      dcBlock->status = DC_VALID;
    } else if (opcode == 57) // storef(57) // 写不命中
    {
      struct cacheBlk *dcBlock = &(dCache[index][lruSlot]);
      trdy = 0;
      if (dcBlock->status == DC_DIRTY) /* Must remote write-back old data */
        trdy = wrBack(tag, time);
      else
        dcBlock->status = DC_DIRTY;
      /* Read in cache line we wish to update */
      trdy += MEM_RD_LATENCY;
      dcBlock->tag = tag;
      dcBlock->trdy = time + trdy;
    } else {
      printf("Error: unknown cache-access op opcode: %d :miss\n", opcode);
      exit(-1);
    }
  }
  return trdy;
}
