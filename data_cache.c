#ifndef MEMSIZE
#include "./regfile.c"
#endif
#include <math.h> // log2()
#include <stdbool.h>
#define DC_NUM_SETS 4     // 4组
#define DC_SET_SIZE 2     // 2路组相联
#define DC_BLOCK_SIZE 16  // 块大小=4*sizeof(float)
#define DC_WR_BUFF_SIZE 4 // 写缓冲槽位数
#define DC_INVALID 0
#define DC_VALID 1
#define DC_DIRTY 2
#define MEM_RD_LATENCY 75        // 读主存
#define MEM_WR_LATENCY 50        // 写主存
#define WR_BUFF_WR_LATENCY 1     // 写入写缓冲
#define WR_BUFF_FLUSH_LATENCY 20 // 写缓冲中的数据写到主存

extern int pipeline_cycle_count;
int lruTime;
int rd_cnt, wr_cnt, rd_hit, wr_hit;
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
  // printf("time=%d, Opcode=WR, Latency=%d, hit=%d\n", time, latency, found);
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
  // 判断是否命中，同时查找用于替换的LRU行
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
  // 处理命中与否的逻辑
  int local_trdy = 0;
  if (hit) {
    struct cacheBlk *dcBlock = &(dCache[index][slot]);
    dcBlock->trdy = pipeline_cycle_count;
    if (opcode == 41 ||
        opcode == 49) // lw(41) and loadf(49) 读命中 视为没有开销
    {
      rd_cnt++;
      rd_hit++;
    } else if (opcode == 57) // storef(57) 写命中 视为没有开销
    {
      wr_cnt++;
      wr_hit++;
      dcBlock->status = DC_DIRTY;
    } else {
      printf("Error: unknown cache-access op opcode: %d :hit\n", opcode);
      exit(-1);
    }
  } else {
    if (opcode == 41 || opcode == 49) // lw(41) and loadf(49) 读不命中
    {
      rd_cnt++;
      struct cacheBlk *dcBlock = &(dCache[index][lruSlot]);
      local_trdy = MEM_RD_LATENCY;
      if (dcBlock->status == DC_DIRTY)   // 如果被换出的块为脏块
        local_trdy += wrBack(tag, time); // 计算换出、写回的开销
      // local_trdy += WR_BUFF_FLUSH_LATENCY;
      dcBlock->tag = tag;
      dcBlock->trdy = time + local_trdy;
      dcBlock->status = DC_VALID;
    } else if (opcode == 57) // storef(57) 写不命中
    {
      wr_cnt++;
      struct cacheBlk *dcBlock = &(dCache[index][lruSlot]);
      local_trdy = 0;
      if (dcBlock->status == DC_DIRTY)  /* Must remote write-back old data */
        local_trdy = wrBack(tag, time); // 需要计算写回开销
      else
        dcBlock->status = DC_DIRTY; // 直接将此块覆盖，并打上DIRTY标签
      /* Read in cache line we wish to update */
      local_trdy += MEM_RD_LATENCY; // 从内存中读出此块
      dcBlock->tag = tag;
      dcBlock->trdy = time + local_trdy;
    } else {
      printf("Error: unknown cache-access op opcode: %d :miss\n", opcode);
      exit(-1);
    }
  }
  char loadf_str[] = "loadf";
  char storef_str[] = "storef";
  char *opcode_str;
  if (opcode == 49)
    opcode_str = loadf_str;
  else if (opcode == 57)
    opcode_str = storef_str;
  // printf("time=%d\tOp=%s\tLatency=%d\thit=%d\n", time, opcode_str,
  // local_trdy,
  //  hit);
  return local_trdy;
}
