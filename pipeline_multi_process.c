
#include "pipeline_multi_process.h"
#include "data_cache.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
// 全局变量
dword cur_instr_code;
struct instr cur_instr;
State current_state = IF;
int ins_count, cycle_count, pipeline_cycle_count;
int pipefd[2];
int resultfd[2];
bool if_print = false;

struct INST INSTLIST[MAXINST];
struct INST nop = {0, 0, 0, 0, 0, false};
struct STAGE StageF, StageD, StageE, StageM, StageW;

// 返回指令类型的函数，用于打印
char *itype(struct INST ins) {
  if (ins.Hlt)
    return hlt;
  if (ins.Flush)
    return nope;
  switch (ins.opcode) {
  case ALU:
    return alu;
  case BRANCH:
    return branch;
  case JMP:
    return jmp;
  case LOAD:
    return load;
  case STORE:
    return store;
  case FPU:
    return fpu;
  case 0:
    return nope;
  default:
    return error;
  }
}

// 从管道中读取指令
struct INST read_from_pipe() {
  struct INST inst;
  if (read(pipefd[0], &inst, sizeof(struct INST)) > 0) {
    return inst;
  } else {
    inst.Hlt = true; // 没有更多指令时，设置Hlt为true
    return inst;
  }
}

// 将指令写入管道
void write_to_pipe(struct INST inst) {
  if (write(pipefd[1], &inst, sizeof(struct INST)) == -1) {
    perror("Write to pipe failed");
    exit(EXIT_FAILURE);
  }
}

// 解码指令的函数
struct instr decode(dword code) {
  struct instr ins;
  ins.code = code;
  ins.opcode = (code >> 26) & 0x3f;
  ins.fmt = (code >> 21) & 0x1f;
  ins.base = (code >> 21) & 0x1f;
  ins.rs = (code >> 21) & 0x1f;
  ins.rt = (code >> 16) & 0x1f;
  ins.rd = (code >> 11) & 0x1f;
  ins.ft = (code >> 16) & 0x1f;
  ins.fs = (code >> 11) & 0x1f;
  ins.fd = (code >> 6) & 0x1f;
  ins.shamt = (code >> 6) & 0x1f;
  ins.func = code & 0x3f;
  ins.imm = code & 0xffff;
  if (ins.code == 0x0) {
    ins.name = "hlt";
    ins.nextpc = 0;
    return ins;
  }
  switch (ins.opcode) {
  case 0:
    switch (ins.func) {
    case 0b000000:
      ins.name = "sll";
      dword temp_sll_rt = load_dword_reg(ins.rt);
      ins.rt_v = *(qword *)&temp_sll_rt;
      break;
    case 0b100000:
      ins.name = "add";
      dword temp_add_rs = load_dword_reg(ins.rs);
      dword temp_add_rt = load_dword_reg(ins.rt);
      ins.rs_v = *(qword *)&temp_add_rs;
      ins.rt_v = *(qword *)&temp_add_rt;
      break;
    case 0b100001:
      ins.name = "addu";
      uqword temp_addu_rs = load_udword_reg(ins.rs);
      uqword temp_addu_rt = load_udword_reg(ins.rt);
      ins.rs_v = *(qword *)&temp_addu_rs;
      ins.rt_v = *(qword *)&temp_addu_rt;
      break;
    default:
      handle_decode_error(ins.code);
    }
    break;
  case 0b001000:
    ins.name = "addi";
    dword temp_addi_rs = load_dword_reg(ins.rs);
    ins.rs_v = *(qword *)&temp_addi_rs;
    break;
  case 0b001001:
    ins.name = "addiu";
    udword temp_addiu_rs = load_udword_reg(ins.rs);
    ins.rs_v = *(qword *)&temp_addiu_rs;
    break;
  case 0b010110:
    ins.name = "bge";
    qword temp_bge_rs = load_qword_reg(ins.rs);
    qword temp_bge_rt = load_qword_reg(ins.rt);
    ins.rs_v = *(qword *)&temp_bge_rs;
    ins.rt_v = *(qword *)&temp_bge_rt;
    break;
  case 0b010001:
    switch (ins.fmt) {
    case 0b10000:
      switch (ins.func) {
      case 0b000000:
        ins.name = "addf";
        float temp_addf_fs = load_float_reg(ins.fs);
        float temp_addf_ft = load_float_reg(ins.ft);
        ins.fs_v = *(qword *)&temp_addf_fs;
        ins.ft_v = *(qword *)&temp_addf_ft;
        break;
      case 0b000010:
        ins.name = "mulf";
        float temp_mulf_fs = load_float_reg(ins.fs);
        float temp_mulf_ft = load_float_reg(ins.ft);
        ins.fs_v = *(qword *)&temp_mulf_fs;
        ins.ft_v = *(qword *)&temp_mulf_ft;
        break;
      default:
        handle_decode_error(ins.code);
      }
      break;
    case 0b10001:
      switch (ins.func) {
      case 0x06:
        ins.name = "mov.d";
        double temp_mov_d_fs = load_double_reg(ins.fs);
        ins.fs_v = *(qword *)&temp_mov_d_fs;
        break;
      default:
        handle_decode_error(ins.code);
        system_break();
      }
      break;
    case 0b00100: {
      ins.name = "mtc1";
      dword temp_mtc1_rt = load_dword_reg(ins.rt);
      ins.rt_v = *(qword *)&temp_mtc1_rt;
      break;
    }
    case 0b00000: {
      ins.name = "mfc1";
      float temp_mfc1_fs = load_float_reg(ins.fs);
      ins.fs_v = *(qword *)&temp_mfc1_fs;
      break;
    }
    case 0b00101: {
      ins.name = "dmtc1";
      qword temp_dmtc1_rt = load_qword_reg(ins.rt);
      ins.rt_v = *(qword *)&temp_dmtc1_rt;
      break;
    }
    case 0b00001: {
      ins.name = "dmfc1";
      double temp_dmfc1_fs = load_double_reg(ins.fs);
      ins.fs_v = *(qword *)&temp_dmfc1_fs;
      break;
    }
    default:
      handle_decode_error(ins.code);
      break;
    }
    break;
  case 0b110001:
    ins.name = "loadf";
    qword temp_loadf_base = load_qword_reg(ins.base);
    ins.base_v = *(qword *)&temp_loadf_base;
    break;
  case 0b111001:
    ins.name = "storef";
    float temp_storef_fs = load_float_reg(ins.ft);
    ins.ft_v = *(qword *)&temp_storef_fs;
    qword temp_storef_base = load_qword_reg(ins.base);
    ins.base_v = *(qword *)&temp_storef_base;
    break;
  default:
    handle_decode_error(ins.code);
  }
  ins.latency = 0;
  return ins;
}

// 执行指令的函数
struct instr execute(struct instr ins, unsigned int pc) {
  if (strcmp(ins.name, "sll") == 0) {
    dword temp_sll_rd = *(dword *)&ins.rt_v << ins.shamt;
    ins.rd_v = *(qword *)&temp_sll_rd;
  } else if (strcmp(ins.name, "add") == 0) {
    dword temp_add_rd = *(dword *)&ins.rs_v + *(dword *)&ins.rt_v;
    ins.rd_v = *(qword *)&temp_add_rd;
  } else if (strcmp(ins.name, "addu") == 0) {
    uqword temp_addu_rd = *(uqword *)&ins.rs_v + *(uqword *)&ins.rt_v;
    ins.rd_v = *(qword *)&temp_addu_rd;
  } else if (strcmp(ins.name, "addi") == 0) {
    dword temp_addi_rt = *(dword *)&ins.rs_v + ins.imm;
    ins.rt_v = *(qword *)&temp_addi_rt;
  } else if (strcmp(ins.name, "addiu") == 0) {
    uqword temp_addiu_rt = *(uqword *)&ins.rs_v + ((uqword)ins.imm & 0xffff);
    ins.rt_v = *(qword *)&temp_addiu_rt;
  } else if (strcmp(ins.name, "addf") == 0) {
    float temp_addf_fd = *(float *)&ins.fs_v + *(float *)&ins.ft_v;
    ins.fd_v = *(qword *)&temp_addf_fd;
  } else if (strcmp(ins.name, "mulf") == 0) {
    float temp_mulf_fd = *(float *)&ins.fs_v * *(float *)&ins.ft_v;
    ins.fd_v = *(qword *)&temp_mulf_fd;
  } else if (strcmp(ins.name, "mov.d") == 0) {
    double temp_movd_fd = *(double *)&ins.fs_v;
    ins.fd_v = *(qword *)&temp_movd_fd;
  } else if (strcmp(ins.name, "mtc1") == 0) {
    float temp_mtc1_fs = *(float *)&ins.rt_v;
    ins.fs_v = *(qword *)&temp_mtc1_fs;
  } else if (strcmp(ins.name, "mfc1") == 0) {
    dword temp_mfc1_rt = *(dword *)&ins.fs_v;
    ins.rt_v = *(qword *)&temp_mfc1_rt;
  } else if (strcmp(ins.name, "dmtc1") == 0) {
    float temp_dmtc1_fs = *(double *)&ins.rt_v;
    ins.fs_v = *(qword *)&temp_dmtc1_fs;
  } else if (strcmp(ins.name, "dmfc1") == 0) {
    qword temp_dmfc1_rt = *(qword *)&ins.fs_v;
    ins.rt_v = *(qword *)&temp_dmfc1_rt;
  } else if (strcmp(ins.name, "loadf") == 0) {
    qword temp_loadf_rt = ins.base_v + ins.imm;
    ins.rt_v = temp_loadf_rt;
  } else if (strcmp(ins.name, "storef") == 0) {
    qword temp_storef_rt = ins.base_v + ins.imm;
    ins.rt_v = temp_storef_rt;
  } else if (strcmp(ins.name, "bge") == 0) {
    if (*(qword *)&ins.rs_v >= *(qword *)&ins.rt_v) {
      ins.nextpc = pc + (ins.imm << 2);
    } else {
      ins.nextpc = pc + 4;
    }
    return ins;
  } else {
    handle_excute_error(ins.code);
  }
  ins.nextpc = pc + 4;
  return ins;
}

// 处理内存访问的函数
struct instr memory(struct instr ins) {
  if (strcmp(ins.name, "loadf") == 0) {
    float temp_loadf_ft = load_float(ins.rt_v);
    ins.ft_v = *(qword *)&temp_loadf_ft;
    ins.latency = accessDCache(49, ins.rt_v, cycle_count);
  } else if (strcmp(ins.name, "storef") == 0) {
    float temp_storef_ft = *(float *)&ins.ft_v;
    store_float(ins.rt_v, temp_storef_ft);
    ins.latency = accessDCache(57, ins.rt_v, cycle_count);
  }
  return ins;
}

// 写回结果到寄存器的函数
void writeback(struct instr ins) {
  if (strcmp(ins.name, "sll") == 0) {
    dword temp_sll_rd = *(dword *)&ins.rd_v;
    store_dword_reg(ins.rd, temp_sll_rd);
  } else if (strcmp(ins.name, "add") == 0) {
    dword temp_add_rd = *(dword *)&ins.rd_v;
    store_dword_reg(ins.rd, temp_add_rd);
  } else if (strcmp(ins.name, "addu") == 0) {
    uqword temp_addu_rd = *(uqword *)&ins.rd_v;
    store_qword_reg(ins.rd, *(qword *)&temp_addu_rd);
  } else if (strcmp(ins.name, "addi") == 0) {
    dword temp_addi_rt = *(dword *)&ins.rt_v;
    store_dword_reg(ins.rt, *(qword *)&temp_addi_rt);
  } else if (strcmp(ins.name, "addiu") == 0) {
    uqword temp_addiu_rt = *(uqword *)&ins.rt_v;
    store_qword_reg(ins.rt, *(qword *)&temp_addiu_rt);
  } else if (strcmp(ins.name, "addf") == 0) {
    float temp_addf_fd = *(float *)&ins.fd_v;
    store_float_reg(ins.fd, temp_addf_fd);
  } else if (strcmp(ins.name, "mulf") == 0) {
    float temp_mulf_fd = *(float *)&ins.fd_v;
    store_float_reg(ins.fd, temp_mulf_fd);
  } else if (strcmp(ins.name, "mov.d") == 0) {
    double temp_movd_fd = *(double *)&ins.rs_v;
    store_double_reg(ins.fd, temp_movd_fd);
  } else if (strcmp(ins.name, "mtc1") == 0) {
    float temp_mtc1_fs = *(float *)&ins.rt_v;
    store_float_reg(ins.fs, temp_mtc1_fs);
  } else if (strcmp(ins.name, "mfc1") == 0) {
    dword temp_mfc1_rt = *(dword *)&ins.fs_v;
    store_dword_reg(ins.rt, *(qword *)&temp_mfc1_rt);
  } else if (strcmp(ins.name, "dmtc1") == 0) {
    double temp_dmtc1_fs = *(double *)&ins.rt_v;
    store_double_reg(ins.fs, temp_dmtc1_fs);
  } else if (strcmp(ins.name, "dmfc1") == 0) {
    qword temp_dmfc1_rt = *(qword *)&ins.fs_v;
    store_double_reg(ins.rt, temp_dmfc1_rt);
  } else if (strcmp(ins.name, "loadf") == 0) {
    store_float_reg(ins.ft, *(float *)&ins.ft_v);
  }
}

// 错误处理函数实现
void handle_decode_error(dword code) {
  printf("decode error with code: %#x\n", code);
  exit(EXIT_FAILURE);
}

void handle_excute_error(dword code) {
  printf("excute error with code: %#x\n", code);
  exit(EXIT_FAILURE);
}

// 从文件中加载指令到缓存
void load_instructions(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }

  char line[10];
  int inst_count = 0;

  // 将机器码存入指令缓存
  while (fgets(line, sizeof(line), file)) {
    dword code = (dword)strtol(line, NULL, 16);
    // 将dword类型的指令拆分为字节并存入缓存
    inst_cache[inst_count++] = (code >> 24) & 0xFF;
    inst_cache[inst_count++] = (code >> 16) & 0xFF;
    inst_cache[inst_count++] = (code >> 8) & 0xFF;
    inst_cache[inst_count++] = code & 0xFF;
  }

  fclose(file);
}

// 从指令缓存中取指
dword fetch(unsigned int pc) {
  ins_count++;
  // 从缓存中读取四个字节并组合成一个dword
  dword code = 0;
  code |= inst_cache[pc] << 24;
  code |= inst_cache[pc + 1] << 16;
  code |= inst_cache[pc + 2] << 8;
  code |= inst_cache[pc + 3];
  return code;
}

// 将具体指令转换为抽象指令
struct INST instr_to_INST(struct instr input) {
  struct INST output;

  if (strcmp(input.name, "hlt") == 0) {
    output.opcode = 0x00;
  } else if (strcmp(input.name, "sll") == 0) {
    output.opcode = ALU;
  } else if (strcmp(input.name, "add") == 0) {
    output.opcode = ALU;
  } else if (strcmp(input.name, "addu") == 0) {
    output.opcode = ALU;
  } else if (strcmp(input.name, "addi") == 0) {
    output.opcode = ALU;
  } else if (strcmp(input.name, "addiu") == 0) {
    output.opcode = ALU;
  } else if (strcmp(input.name, "bge") == 0) {
    output.opcode = BRANCH;
  } else if (strcmp(input.name, "addf") == 0) {
    output.opcode = FPU;
  } else if (strcmp(input.name, "mulf") == 0) {
    output.opcode = FPU;
  } else if (strcmp(input.name, "mov.d") == 0) {
    output.opcode = FPU;
  } else if (strcmp(input.name, "mtc1") == 0) {
    output.opcode = FPU;
  } else if (strcmp(input.name, "mfc1") == 0) {
    output.opcode = FPU;
  } else if (strcmp(input.name, "dmtc1") == 0) {
    output.opcode = FPU;
  } else if (strcmp(input.name, "dmfc1") == 0) {
    output.opcode = FPU;
  } else if (strcmp(input.name, "loadf") == 0) {
    output.opcode = LOAD;
  } else if (strcmp(input.name, "storef") == 0) {
    output.opcode = STORE;
  } else {
    // Handle unknown instruction
    output.opcode = 0xFF; // Example error code
  }

  output.RS1 = (char)input.rs;
  output.RS2 = (char)input.rt;
  output.Rd = (char)input.rd;
  output.Imm = (int)input.imm;
  output.Hlt = (input.code == 0x0);
  output.Latency = input.latency;
  return output;
}

// 执行指令的主循环
void execute_instructions() {
  unsigned int pc = 0;
  cycle_count = 0;
  while (pc < MAXINST) {
    cycle_count++;
    switch (current_state) {
    case IF:
      cur_instr_code = fetch(pc);
      current_state = ID;
      break;
    case ID:
      cur_instr = decode(cur_instr_code);
      current_state = EX;
      break;
    case EX:
      if (cur_instr.code == 0x0) {
        goto halt;
      }
      cur_instr = execute(cur_instr, pc);
      current_state = MEM;
      break;
    case MEM:
      cur_instr = memory(cur_instr);
      cycle_count += cur_instr.latency;
      // printf("at time %d, latency=%d\n", cycle_count, cur_instr.latency);
      current_state = WB;
      break;
    case WB:
      writeback(cur_instr);
      current_state = IF;
      struct INST inst = instr_to_INST(cur_instr);
      write_to_pipe(inst); // 将指令写入管道
      pc = cur_instr.nextpc;
      break;
    }
  }
halt:
  struct INST inst = instr_to_INST(cur_instr);
  write_to_pipe(inst); // 发送结束信号
}

void print_usage(const char *program_name) {
  printf("MIPS64 CPU Simulator\nUsage: %s <instruction_file> "
         "[-p]\n[Args]\t-p\tprint "
         "process\n",
         program_name);
}

int pipeline_execute() {
  struct INST inst;
  StageF.valid = true;
  StageF.inst.Hlt = false;
  StageD.valid = true;
  StageD.inst.Hlt = false;
  StageE.valid = true;
  StageE.inst.Hlt = false;
  StageM.valid = true;
  StageM.inst.Hlt = false;
  StageW.valid = true;
  StageW.inst.Hlt = false;
  int DM_busy = 0;
  bool IM_busy = false; // unused
  bool end_flag = false;
  int ALU_busy = 0;
  int FPU_busy = 0;
  while (1) {
    if (StageE.inst.Hlt)
      break;
    StageW.inst = nop;

    StageW.valid = DM_busy == 0;
    // StageW.valid = true;
    // whether to start WB
    if (!(!StageW.valid)) {
      StageW = StageM;
      StageM.valid = true;
    } else {
      StageM.valid = false;
    }
    // whether to start MEM
    if (!(!StageM.valid || DM_busy)) {
      StageM = StageE;
      DM_busy = StageM.inst.Latency;
      // StageM.inst.Flush = false;
      StageE.valid = true;
    } else {
      StageE.valid = false;
    }

    // whether to start EX
    if (!(!StageE.valid || ALU_busy >= ALU_cnt || FPU_busy >= FPU_cnt ||
          ((StageE.inst.opcode == LOAD &&
            !StageE.inst.Flush) && // Flush，指这条指令实际上被认为是/不是空指令
           (StageD.inst.RS1 == StageE.inst.Rd ||
            StageD.inst.RS2 == StageE.inst.Rd)))) {
      StageE = StageD;
      StageD.valid = true;
    } else {
      if (StageE.valid) {
        StageE.inst.Flush = true;
        StageE.inst.Latency = 0;
      }
      StageD.valid = false;
    }
    // whether to start ID
    if (!(!StageD.valid ||
          ((StageF.inst.opcode == BRANCH && !StageF.inst.Flush) &&
           (StageF.inst.RS1 == StageD.inst.Rd ||
            StageF.inst.RS2 == StageD.inst.Rd)) ||
          ((StageF.inst.opcode == JMP && !StageF.inst.Flush) &&
           (StageF.inst.RS1 == StageD.inst.Rd ||
            StageF.inst.RS2 == StageD.inst.Rd)))) {
      StageD = StageF;
      StageF.valid = true;
    } else {
      if (StageD.valid)
        StageD.inst.Flush = true;
      StageE.inst.Latency = 0;
      StageF.valid = false;
    }
    // whether to fetch an instruction
    if (!(!StageF.valid || (!StageD.inst.Flush && StageD.inst.opcode == JMP) ||
          (!StageD.inst.Flush && StageD.inst.opcode == BRANCH) ||
          (!StageE.inst.Flush && StageD.inst.opcode == JMP) ||
          (!StageE.inst.Flush && StageD.inst.opcode == BRANCH))) {
      if (!end_flag)
        StageF.inst = read_from_pipe();
      else
        StageF.inst = nop;
      if (StageF.inst.Hlt)
        end_flag = true;
      StageF.inst.Flush = false;
      StageF.valid = true;
    } else {
      if (StageF.valid)
        StageF.inst.Flush = true;
      StageE.inst.Latency = 0;
      StageF.valid = false;
    }
    if (if_print) {
      // printf(
      //     "cycle %05d \t|F:%s %d\t|D:%s %d\t|E:%s %d\t|M:%s %d\t|W:%s
      //     %d\t|\n", pipeline_cycle_count, itype(StageF.inst),
      //     StageF.inst.Latency, itype(StageD.inst), StageD.inst.Latency,
      //     itype(StageE.inst), StageE.inst.Latency, itype(StageM.inst),
      //     StageM.inst.Latency, itype(StageW.inst), StageW.inst.Latency);
      printf("cycle %05d \t|F:%s\t|D:%s\t|E:%s\t|M:%s\t|W:%s\t|\n",
             pipeline_cycle_count, itype(StageF.inst), itype(StageD.inst),
             itype(StageE.inst), itype(StageM.inst), itype(StageW.inst));
    }
    if (DM_busy > 0) {
      DM_busy--;
    }
    // printf("DM_busy=%d\n", DM_busy);
    pipeline_cycle_count++;
  }
  return pipeline_cycle_count;
}

int main(int argc, char *argv[]) {
  const char *instruction_file = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0) {
      if_print = 1;
    } else {
      instruction_file = argv[i];
    }
  }

  if (instruction_file == NULL) {
    print_usage(argv[0]);
    return 1;
  }

  if (pipe(pipefd) == -1) {
    perror("Pipe failed");
    exit(EXIT_FAILURE);
  }

  if (pipe(resultfd) == -1) {
    perror("Pipe failed");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork(); // 创建子进程
  if (pid == 0) {     // 子进程
    close(pipefd[1]); // 子进程只读
    close(resultfd[0]);
    pipeline_cycle_count = pipeline_execute();
    write(resultfd[1], &pipeline_cycle_count, sizeof(pipeline_cycle_count));
    close(resultfd[1]);
    exit(EXIT_SUCCESS);
  } else if (pid > 0) { // 父进程
    close(pipefd[0]);   // 父进程只写
    close(resultfd[1]);
    if (if_print) {
      printf("存入的待计算数据\n");
      for (int i = 0; i < 64; i++) {
        store_float(i * 4, 0.1 * (i + 1) + 4);
        printf("f[%d]:%.2f\t", i, load_float(i * 4));
        if (!((i + 1) % 4))
          printf("\n");
      }
    }

    load_instructions(instruction_file);
    execute_instructions();

    int status;
    waitpid(pid, &status, 0); // 等待子进程结束
    if (if_print) {
      printf("\n使用c直接计算出的标准答案\n");
      for (int i = 0; i < 64; i++) {
        printf("f[%d]:%.2f\t", i, (0.1 * (i + 1) + 4) * 0.9 + 0.5);
        if (!((i + 1) % 4))
          printf("\n");
      }
      printf("\n读取的结果数据\n");
      for (int i = 0; i < 64; i++) {
        printf("f[%d]:%.2f\t", i, load_float(i * 4));
        if (!((i + 1) % 4))
          printf("\n");
      }
    }
    printf("Instruction count:%d\n", ins_count);
    read(resultfd[0], &pipeline_cycle_count, sizeof(pipeline_cycle_count));
    close(resultfd[0]);
    printf("Pipeline Cycle count:%d\n", pipeline_cycle_count);
    printf("Pipeline CPI:%.4f\n",
           ((double)pipeline_cycle_count / (double)ins_count));
    if (!if_print)
      printf("提示：可以添加-p参数运行以打印过程信息，推荐打印到文本文件中查看:"
             " ./pipeline_multi_process "
             "<file> "
             "-p > log.txt\n");
  } else {
    perror("Failed to fork");
    exit(EXIT_FAILURE);
  }
  return 0;
}