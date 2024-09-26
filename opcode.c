#include "CA/regfile.c"

// extern char* mem;
// extern qword regf;

struct instr {
  char *name;
  qword op1;
  qword op2;
};

// 截取指定高位
unsigned int get_high_bits(qword num, int high_bits) {
  // 计算掩码
  unsigned int mask = (1 << high_bits) - 1;
  // 将掩码左移到高位
  mask <<= (sizeof(unsigned int) * 8 - high_bits);
  // 使用掩码提取高位
  return (num & mask) >> (sizeof(unsigned int) * 8 - high_bits);
}

// 译码逻辑
struct instr decode(qword code) {
  int opcode1 = get_high_bits(code, 1);
  int opcode2 = get_high_bits(code, 2) & 0xf;
  int rA = get_high_bits(code, 3) & 0xf;
  int rB = get_high_bits(code, 3) & 0xf;
  struct instr ins;
  ins.op1 = rA;
  switch (opcode1) {
  case 0:
    ins.op2 = rB;
    switch (opcode2) {
    case 0:
      ins.name = "cmp";
      break;
    case 1:
      ins.name = "jg";
      break;
    default:
      printf("decode error with code: %#llx\n", code);
      break;
    }
    break;
  case 1:
    ins.op2 = rB;
    switch (opcode2) {
    case 0:
      ins.name = "movd";
      break;
    case 1:
      ins.op2 = code & (1 << 8 * sizeof(dword) - 1);
      ins.name = "immd";
      break;
    case 2:
      ins.name = "stored";
      break;
    case 3:
      ins.name = "stored";
      break;
    default:
      printf("decode error with code: %#llx\n", code);
      break;
    }
  case 2:
    ins.op2 = rB;
    switch (opcode2) {
    case 0:
      ins.name = "addd";
      break;
    case 1:
      ins.name = "addf";
      break;
    case 2:
      ins.name = "mulf";
      break;
    default:
      printf("decode error with code: %#llx\n", code);
      break;
    }
  case 3:
    ins.op2 = rB;
    switch (opcode2) {
    case 0:
      ins.name = "push";
      break;
    case 1:
      ins.name = "pop";
      break;
    case 2:
      ins.name = "mulf";
      break;
    default:
      printf("decode error with code: %#llx\n", code);
      break;
    }
  default:
    printf("decode error with code: %#llx\n", code);
    break;
  }
  return ins;
}

int main() {
  struct instr mycode = decode(0x13BC);
  printf("%s\n", mycode.name);
  return 0;
}
