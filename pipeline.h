#include "./regfile.c" // 定义了寄存器、主存接口
#include <stdbool.h>
#include <string.h>

#define ALU 0x01
#define BRANCH 0x02
#define JMP 0x04
#define LOAD 0x08
#define STORE 0x10
#define FPU 0x20
#define ALU_cnt 4
#define FPU_cnt 4
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
struct INST {
  char opcode;
  char RS1, RS2, Rd;
  int Imm;
  bool Flush;
  bool Hlt;
};

struct STAGE {
  bool valid;
  struct INST inst;
};

// 错误处理函数声明
void handle_decode_error(dword);
void handle_excute_error(dword);