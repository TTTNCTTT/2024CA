R_Type_dict =  {
                'sll'    : {'opcode':'000000', 'rs':'00000', 'rt':'rt',    'rd':'rd',    'shamt':'shamt', 'func':'000000'},
                'add'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100000'},
                'addu'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100001'},
}

I_Type_dict = {
                'addi'   : {'opcode':'001000', 'rt':'rt', 'rs': 'rs',    'imm':'imm'},
                'addiu'  : {'opcode':'001001', 'rt':'rt', 'rs': 'rs',    'imm':'imm'}, 
                'bge'    : {'opcode':'010110', 'rs': 'rs', 'rt': 'rt', 'imm': 'label'},#
}

F_Type_dict = {
                'add.s' : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000000'},
                'addf'  : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000000'},
                'mul.s' : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000010'},
                'mulf'  : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000010'},
                'dmtc1' : {'opcode':'010001', 'fmt':'00101', 'ft':'rt', 'fs':'fs', 'fd':'00000', 'func':'000000'},
                'mov.d' : {'opcode':'010001', 'fmt':'10001', 'ft':'00000', 'fs':'fs', 'fd':'fd', 'func':'000110'},
            }

Load_Store_Type_dict = {
                'l.s'  : {'opcode':'110001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'loadf': {'opcode':'110001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                's.s'  : {'opcode':'111001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'storef': {'opcode':'111001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                }

Pseudo_Type_dict = {
                'mv'     : {'opcode':'111111', 'func':'000011', 'rt':'rt', 'rs':'rs', 'imm':'00000'},
                }

main:
// d=0.9, d into $f1    0xcccccccd
    addiu $rax, $r0, 0xcccc         // 1100110011001100
    sll $rax, $rax, 16
    addiu $r15, $r0, 0xcccd
    addu $rax, $r15, $rax
    mv $f1, $rax  // d=0.9 in $f1

// 0.5 into $f2   0x3f000000
    addi $r11, $r0, 0x3f00
    sll $r11, $r11, 16
    mv $f2, $r11     // 0.5 in $f2

    addi $r12, $r0, 1     // 1 into $r12
    addi $r13, $r0, 4     // 4 into $r13
    addi $rbx, $r0, 64    // n=64, n into $rbx
    addi $rdx, $r0, 0     // i=0, i into $rdx
    addi $r10, $r0, 0x3bf    // (&fa[0]) = 0x3bf, addr into $r10

loop:
    loadf $f0, 0($r10) // fa[i], fa[i] into $f0
    mulf  $f0, $f1, $f0   // fa[i] = fa[i]*d
    addf $f0, $f2, $f0    // fa[i] = fa[i]+0.5
    storef $f0, 0($r10)   // store fa[i]
    add  $rdx, $r12, $rdx    // i++
    add  $r10, $r13, $r10    // addr += 4
    bge $rdx, $rbx, loop
    hlt

根据上述汇编代码，以及下面的字典，写出译码、执行的c程序：
R_Type_dict =  {
                'sll'    : {'opcode':'000000', 'rs':'00000', 'rt':'rt',    'rd':'rd',    'shamt':'shamt', 'func':'000000'},
                'add'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100000'},
                'addu'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100001'},
}

I_Type_dict = {
                'addi'   : {'opcode':'001000', 'rt':'rt', 'rs': 'rs',    'imm':'imm'},
                'addiu'  : {'opcode':'001001', 'rt':'rt', 'rs': 'rs',    'imm':'imm'}, 
                'bge'    : {'opcode':'010110', 'rs': 'rs', 'rt': 'rt', 'imm': 'label'},#
}

F_Type_dict = {
                'add.s' : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000000'},
                'addf'  : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000000'},
                'mul.s' : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000010'},
                'mulf'  : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000010'},
                'dmtc1' : {'opcode':'010001', 'fmt':'00101', 'ft':'rt', 'fs':'fs', 'fd':'00000', 'func':'000000'},
                'mov.d' : {'opcode':'010001', 'fmt':'10001', 'ft':'00000', 'fs':'fs', 'fd':'fd', 'func':'000110'},
            }

Load_Store_Type_dict = {
                'l.s'  : {'opcode':'110001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'loadf': {'opcode':'110001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                's.s'  : {'opcode':'111001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'storef': {'opcode':'111001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                }

Pseudo_Type_dict = {
                'mv'     : {'opcode':'111111', 'func':'000011', 'rt':'rt', 'rs':'rs', 'imm':'00000'},
                }
已知存储器访问接口：
#define MAXSIZE 1024
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
char mem[MAXSIZE];

// 寄存器文件
qword regfile[REGCNT] = {
    0,
};

double regfile_F[REGCNTF] = {
    0.0,
};
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
int store_byte_reg(int, byte);
int store_word_reg(int, word);
int store_dword_reg(int, dword);
int store_qword_reg(int, qword);
float load_float(unsigned int);
double load_double(unsigned int);
float load_float_reg(int);
double load_double_reg(int);
int store_float(unsigned, float);
int store_double(unsigned, double);
int store_float_reg(int, float);
int store_double_reg(int, double);