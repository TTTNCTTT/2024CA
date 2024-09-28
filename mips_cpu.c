#include "./regfile.c"

// extern char* mem;
// extern qword regf;

struct instr {
  char *name;
  int opcode;
  int fmt;
  int base;
  int rs;
  int rt;
  int rd;
  int fs;
  int ft;
  int fd;
  int shamt;
  int func;
  word imm;
  // int uimm;
  int offset;
  unsigned int nextpc;
};

void handle_error(dword);

struct instr decode_and_excute(dword code, unsigned int pc) {
  struct instr ins;
  if (code == 0x0) {
    ins.name = "hlt";
    ins.nextpc = 0;
    return ins;
  }
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

  switch (ins.opcode) {
  case 0:
    switch (ins.func) {
    case 0b000000:
      ins.name = "sll";
      store_dword_reg(ins.rd, load_dword_reg(ins.rt) << ins.shamt);
      break;
    case 0b100000:
      ins.name = "add";
      store_dword_reg(ins.rd, load_dword_reg(ins.rs) + load_dword_reg(ins.rt));
      break;
    case 0b100001:
      ins.name = "addu";
      uqword temp = load_udword_reg(ins.rs) + load_udword_reg(ins.rt);
      store_dword_reg(ins.rd, load_dword_reg(ins.rs) + load_dword_reg(ins.rt));
      break;
    default:
      handle_error(code);
    }
    ins.nextpc = pc + 4;
    break;
  case 0b001000:
    ins.name = "addi";
    store_dword_reg(ins.rt, load_dword_reg(ins.rs) + ins.imm);
    ins.nextpc = pc + 4;
    break;
  case 0b001001:
    ins.name = "addiu";
    store_dword_reg(ins.rt,
                    load_udword_reg(ins.rs) + ((uqword)ins.imm & 0xffff));
    ins.nextpc = pc + 4;
    break;
  case 0b010110:
    ins.name = "bge";
    if (load_qword_reg(ins.rs) >= load_qword_reg(ins.rt)) {
      ins.nextpc = pc + (ins.imm << 2);
    } else {
      ins.nextpc = pc + 4;
    }
    break;
  case 0b010001:
    switch (ins.fmt) {
    case 0b10000:
      switch (ins.func) {
      case 0b000000:
        ins.name = "addf";
        store_float_reg(ins.fd,
                        load_float_reg(ins.fs) + load_float_reg(ins.ft));
        break;
      case 0b000010:
        float temp_1 = load_float_reg(ins.fs);
        float temp_2 = load_float_reg(ins.ft);
        ins.name = "mulf";
        store_float_reg(ins.fd,
                        load_float_reg(ins.fs) * load_float_reg(ins.ft));
        break;
      default:
        handle_error(code);
      }
      break;
    case 0b10001:
      switch (ins.func) {
      case 0x06:
        ins.name = "mov.d";
        store_double_reg(ins.fd, load_double_reg(ins.fs));
        break;
      default:
        handle_error(code);
        system_break();
      }
      break;
    case 0b00100: {
      ins.name = "mtc1";
      dword temp = load_dword_reg(ins.rt);
      store_float_reg(ins.fs, *(float *)&temp);
      break;
    }
    case 0b00000: {
      ins.name = "mfc1";
      float temp = load_dword_reg(ins.fs);
      store_dword_reg(ins.rt, *(dword *)&temp);
      break;
    }
    case 0b00101: {
      ins.name = "dmtc1";
      qword temp = load_qword_reg(ins.rt);
      store_float_reg(ins.fs, *(float *)&temp);
      break;
    }
    case 0b00001: {
      ins.name = "dmfc1";
      double temp = load_double_reg(ins.fs);
      store_qword_reg(ins.rt, *(qword *)&temp);
      break;
    }
    default:
      handle_error(code);
      break;
    }
    ins.nextpc = pc + 4;
    break;
  case 0b110001:
    ins.name = "loadf";
    float temp_5 = load_float(load_qword_reg(ins.base) + ins.imm);
    store_float_reg(ins.ft, load_float(load_qword_reg(ins.base) + ins.imm));
    ins.nextpc = pc + 4;
    break;
  case 0b111001:
    ins.name = "storef";
    float temp_3 = load_float_reg(ins.ft);
    store_float(load_qword_reg(ins.base) + ins.imm, load_float_reg(ins.ft));
    ins.nextpc = pc + 4;
    break;
  default:
    handle_error(code);
  }
  return ins;
}

void handle_error(dword code) {
  printf("decode error with code: %#x unknown instruction\n", code);
  system_break();
}

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

// 取指
dword fetch_instruction(unsigned int pc) {
  // 从缓存中读取四个字节并组合成一个dword
  dword code = 0;
  code |= inst_cache[pc] << 24;
  code |= inst_cache[pc + 1] << 16;
  code |= inst_cache[pc + 2] << 8;
  code |= inst_cache[pc + 3];
  return code;
}

// 执行指令
void execute_instructions() {
  unsigned int pc = 0;

  // 从指令缓存中读取指令并执行
  while (pc < MAXINST) {
    dword code = fetch_instruction(pc);
    if (code == 0)
      break;
    struct instr ins = decode_and_excute(code, pc);
    // printf("Executed: %s\n", ins.name);
    pc = ins.nextpc;
  }
}

void print_usage(const char *program_name) {
  printf("Usage: %s <instruction_file>\n", program_name);
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
  return 0;
}
