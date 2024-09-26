#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMSIZE 1024
#define MAXINST 4096
#define REGCNT 32
#define REGCNTF 32
// Intel风格
typedef int8_t byte;
typedef uint8_t ubyte;
typedef int16_t word;
typedef uint16_t uword;
typedef int32_t dword;
typedef uint32_t udword;
typedef int64_t qword;
typedef uint64_t uqword;

// 主存
char mem[MEMSIZE];

// 寄存器文件
qword regfile[REGCNT] = {
    0,
};
double regfile_F[REGCNTF] = {
    0.0,
};

// 指令存储器
unsigned char inst_cache[MAXINST];

void system_break(void);
qword load_byte(unsigned int);
qword load_word(unsigned int);
qword load_dword(unsigned int);
qword load_qword(unsigned int);
uqword load_ubyte(unsigned int);
uqword load_uword(unsigned int);
uqword load_udword(unsigned int);
int store_byte(unsigned int, byte);
int store_word(unsigned int, word);
int store_dword(unsigned int, dword);
int store_qword(unsigned int, qword);
qword load_byte_reg(int index);
qword load_word_reg(int index);
qword load_dword_reg(int index);
qword load_qword_reg(int index);
uqword load_ubyte_reg(int index);
uqword load_uword_reg(int index);
uqword load_udword_reg(int index);
int store_byte_reg(int, ubyte);
int store_word_reg(int, uword);
int store_dword_reg(int, udword);
int store_qword_reg(int, uqword);
float load_float(unsigned int);
double load_double(unsigned int);
float load_float_reg(int);
double load_double_reg(int);
int store_float(unsigned, float);
int store_double(unsigned, double);
int store_float_reg(int, float);
int store_double_reg(int, double);

// 从主存中加载数据
qword load_byte(unsigned int addr) {
  if (addr >= MEMSIZE) {
    printf("addr error when load byte: %#x\n", addr);
    system_break();
  }
  return (qword)(*(byte *)(mem + addr));
}

qword load_word(unsigned int addr) {
  if (addr + sizeof(word) - 1 >= MEMSIZE) {
    printf("addr error when load word: %#x\n", addr);
    system_break();
  }
  return (qword)(*(word *)(mem + addr));
}

qword load_dword(unsigned int addr) {
  if (addr + sizeof(dword) - 1 >= MEMSIZE) {
    printf("addr error when load dword: %#x\n", addr);
    system_break();
  }
  return (qword)(*(dword *)(mem + addr));
}

qword load_qword(unsigned int addr) {
  if (addr + sizeof(qword) - 1 >= MEMSIZE) {
    printf("addr error when load qword: %#x\n", addr);
    system_break();
  }
  return *(qword *)(mem + addr);
}

// 从主存中加载无符号数据
uqword load_ubyte(unsigned int addr) {
  if (addr >= MEMSIZE) {
    printf("addr error when load ubyte: %#x\n", addr);
    system_break();
  }
  return (uqword)(*(ubyte *)(mem + addr));
}

uqword load_uword(unsigned int addr) {
  if (addr + sizeof(uword) - 1 >= MEMSIZE) {
    printf("addr error when load uword: %#x\n", addr);
    system_break();
  }
  return (uqword)(*(uword *)(mem + addr));
}

uqword load_udword(unsigned int addr) {
  if (addr + sizeof(udword) - 1 >= MEMSIZE) {
    printf("addr error when load udword: %#x\n", addr);
    system_break();
  }
  return (uqword)(*(udword *)(mem + addr));
}

// 储存数据到主存中
int store_byte(unsigned int addr, byte v) {
  if (addr >= MEMSIZE) {
    printf("addr error when store byte: %#x\n", addr);
    system_break();
  }
  *(byte *)(mem + addr) = v;
  return 0;
}

int store_word(unsigned int addr, word v) {
  if (addr + sizeof(word) - 1 >= MEMSIZE) {
    printf("addr error when store word: %#x\n", addr);
    system_break();
  }
  *(word *)(mem + addr) = v;
  return 0;
}

int store_dword(unsigned int addr, dword v) {
  if (addr + sizeof(dword) - 1 >= MEMSIZE) {
    printf("addr error when store dword: %#x\n", addr);
    system_break();
  }
  *(dword *)(mem + addr) = v;
  return 0;
}

int store_qword(unsigned int addr, qword v) {
  if (addr + sizeof(qword) - 1 >= MEMSIZE) {
    printf("addr error when store qword: %#x\n", addr);
    system_break();
  }
  *(qword *)(mem + addr) = v;
  return 0;
}

// 从寄存器中读取数据
qword load_byte_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load byte: %#x\n", index);
    system_break();
  }
  return (qword)regfile[index] & 0xff;
}

qword load_word_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load word: %#x\n", index);
    system_break();
  }
  return (qword)regfile[index] & 0xffff;
}

qword load_dword_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load dword: %#x\n", index);
    system_break();
  }
  return (qword)regfile[index] & 0xffffffff;
}

qword load_qword_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load qword: %#x\n", index);
    system_break();
  }
  return regfile[index];
}

// 从寄存器中读取无符号数据
uqword load_ubyte_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load ubyte: %#x\n", index);
    system_break();
  }
  return (regfile[index] & 0xff);
}

uqword load_uword_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load uword: %#x\n", index);
    system_break();
  }
  return (regfile[index] & 0xffff);
}

uqword load_udword_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load udword: %#x\n", index);
    system_break();
  }
  return (regfile[index] & 0xffffffff);
}

// 储存数据到寄存器中，注意0号寄存器不允许存入数据
int store_byte_reg(int index, ubyte v) {
  if (index >= REGCNT || index < 1) {
    printf("reg index error when store byte: %#x\n", index);
    system_break();
  }
  regfile[index] = v;
  return 0;
}

int store_word_reg(int index, uword v) {
  if (index >= REGCNT || index < 1) {
    printf("reg index error when store word: %#x\n", index);
    system_break();
  }
  regfile[index] = v;
  return 0;
}

int store_dword_reg(int index, udword v) {
  if (index >= REGCNT || index < 1) {
    printf("reg index error when store dword: %#x\n", index);
    system_break();
  }
  // *(udword *)(regfile + index) = v;
  regfile[index] = v;
  return 0;
}

int store_qword_reg(int index, uqword v) {
  if (index >= REGCNT || index < 1) {
    printf("reg index error when store qword: %#x\n", index);
    system_break();
  }
  regfile[index] = v;
  return 0;
}

// 浮点存、取操作
float load_float(unsigned int addr) {
  if (addr + sizeof(float) - 1 >= MEMSIZE) {
    printf("addr error when load float: %#x\n", addr);
    system_break();
  }
  return *(float *)(mem + addr);
}

double load_double(unsigned int addr) {
  if (addr + sizeof(double) - 1 >= MEMSIZE) {
    printf("addr error when load double: %#x\n", addr);
    system_break();
  }
  return *(double *)(mem + addr);
}

float load_float_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load float: %#x\n", index);
    system_break();
  }
  return *(float *)(regfile_F + index);
}

double load_double_reg(int index) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when load double: %#x\n", index);
    system_break();
  }
  return regfile_F[index];
}

int store_float(unsigned int addr, float v) {
  if (addr + sizeof(float) - 1 >= MEMSIZE) {
    printf("addr error when store float: %#x\n", addr);
    system_break();
  }
  *(float *)(mem + addr) = v;
  return 0;
}

int store_double(unsigned int addr, double v) {
  if (addr + sizeof(double) - 1 >= MEMSIZE) {
    printf("addr error when store float: %#x\n", addr);
    system_break();
  }
  *(double *)(mem + addr) = v;
  return 0;
}

int store_float_reg(int index, float v) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when store float: %#x\n", index);
    system_break();
  }
  *(float *)(regfile_F + index) = v;
  return 0;
}

int store_double_reg(int index, double v) {
  if (index >= REGCNT || index < 0) {
    printf("reg index error when store float: %#x\n", index);
    system_break();
  }
  regfile_F[index] = v;
  return 0;
}

// 出现异常
void system_break() {
  printf("fault!\n");
  exit(-1);
}

// 两个简单的测试函数
void test_mem() {
  store_qword(0, 0xdeadbeefbeefdead);
  printf("========== memory ===========\n");
  printf("byte: %#016jx\n", load_byte(0));
  printf("half: %#016jx\n", load_word(0));
  printf("word: %#016jx\n", load_dword(0));
  printf("doub: %#016jx\n", load_qword(0));
  printf("ubyte: %#016jx\n", load_ubyte(0));
  printf("uhalf: %#016jx\n", load_uword(0));
  printf("uword: %#016jx\n", load_udword(0));
  printf("=============================\n");
}

void test_regfile() {
  store_qword_reg(1, 0xbeefdeaddeadbeef);
  printf("========== regfile ===========\n");
  printf("reg byte: %#016jx\n", load_byte_reg(1));
  printf("reg word: %#016jx\n", load_word_reg(1));
  printf("reg dword: %#016jx\n", load_dword_reg(1));
  printf("reg qword: %#016jx\n", load_qword_reg(1));
  printf("reg ubyte: %#016jx\n", load_ubyte_reg(1));
  printf("reg uword: %#016jx\n", load_uword_reg(1));
  printf("reg udword: %#016jx\n", load_udword_reg(1));
  printf("==============================\n");
}

void test_regfile_F() {
  printf("========== float regfile ===========\n");
  store_float(15, 0.1);
  store_double(20, 0.2);
  printf("float: %f\n", load_float(15));
  printf("double: %f\n", load_double(20));
  store_float_reg(1, 0.1);
  store_double_reg(2, 0.2);
  printf("reg float: %f\n", load_float_reg(1));
  printf("reg double: %f\n", load_double_reg(2));
  printf("=====================================\n");
}

// 对边界检测函数的测试
// void test_error() {
//   printf("doub: %#016jx\n", load_qword(1024));
//   printf("reg word: %jx\n", load_qword_reg(1024));
// }

// int main() {
//   test_mem();
//   test_regfile();
//   test_regfile_F();
//   test_error();
//   return 0;
// }
