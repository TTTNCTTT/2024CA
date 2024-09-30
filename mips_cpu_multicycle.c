#include "./regfile.c" // 定义了寄存器、主存接口
#include <string.h>

// 定义CPU的状态枚举
typedef enum { IF, ID, EX, MEM, WB } State;

// 定义指令结构体
struct instr {
  char *name; // 指令名称
  dword code; // 指令代码
  int opcode; // 操作码
  int fmt;    // 指令格式
  int base, rs, rt, rd, fs, ft, fd, shamt, func;
  word imm; // 立即数
  // int uimm;
  int offset;          // 偏移量
  unsigned int nextpc; // 下一条指令的PC地址
  qword base_v, rs_v, rt_v, rd_v, fs_v, ft_v, fd_v; // 操作数的值
};

// 全局变量
dword cur_instr_code;
struct instr cur_instr;
State current_state = IF;
int cycle_count;
int ins_count;

// 错误处理函数声明
void handle_decode_error(dword);
void handle_excute_error(dword);

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
  } else if (strcmp(ins.name, "storef") == 0) {
    float temp_storef_ft = *(float *)&ins.ft_v;
    store_float(ins.rt_v, temp_storef_ft);
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
  system_break();
}

void handle_excute_error(dword code) {
  printf("excute error with code: %#x\n", code);
  system_break();
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

// 执行指令的主循环
void execute_instructions() {
  unsigned int pc = 0;
  // 多周期CPU的主循环，按照IF-ID-EX-MEM-WB顺序执行
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
      if (cur_instr.code == 0x0)
        goto halt;
      cur_instr = execute(cur_instr, pc);
      current_state = MEM;
      break;
    case MEM:
      cur_instr = memory(cur_instr);
      current_state = WB;
      break;
    case WB:
      writeback(cur_instr);
      current_state = IF;
      pc = cur_instr.nextpc;
      // printf("Executed: %s\n", cur_instr.name);
      break;
    }
  }
halt:;
}

void print_usage(const char *program_name) {
  printf("MIPS64 CPU Simulator\nUsage: %s <instruction_file>\n", program_name);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    print_usage(argv[0]);
    return 1;
  }

  const char *instruction_file = argv[1];

  printf("存入的待计算数据\n");
  for (int i = 0; i < 64; i++) {
    store_float(i * 4, 0.1 * (i + 1) + 4);
    printf("f[%d]:%.2f\t", i, load_float(i * 4));
    if (!((i + 1) % 4))
      printf("\n");
  }
  printf("\n使用c直接计算出的标准答案\n");
  load_instructions(instruction_file);
  execute_instructions();
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
  printf("\nInstruction count:%d\n", ins_count);
  printf("\nCycle count:%d\n", cycle_count);
  printf("\nCPI:%.4f\n", ((double)cycle_count / (double)ins_count));
  return 0;
}