import sys
import re
import copy
import argparse
#错误检测系列函数
def R_error_check(instruction):
    instruction_name = instruction[0]    
    line_num = instruction[-1]
    num_of_operand = -2 
    for operand in R_Type_dict[instruction[0]].values(): #统计操作数的数量
        if (operand != "00000"): 
            num_of_operand += 1
    if (len(instruction[1]) != num_of_operand):
        print("Error No.1 found in line %d:\nIncorrect number of operands. %d required but %d were given."%(line_num, num_of_operand ,len(instruction[1]))) #操作数有误
        return False
    elif instruction_name in ['jr','jalr']:
        if not(instruction[1][0] in register_map):
            print("Error  No.2 found in line %d:\nInvalid label name."%(line_num))
            return False
    else:
        for i in range(len(instruction[1])): #若为break和syscall指令，则len(instruction[1])=0,for循环中的程序将不会执行
            if (i==2 and instruction_name in ['sll', 'srl', 'sra']):
                if (not(instruction[1][2]).isdigit()):
                    print("Error No.3 found in line %d, operand 3:\nOperand 3 of an %s instruction must be a number."%(line_num,instruction_name)) #操作数类型有误
                    return False
                elif (int(instruction[1][2]) > 2**5-1 or int(instruction[1][2]) < 0):
                    print("Error No.4 found in line %d, operand 2:\nNumber operand out of range."%line_num)
                    return False
            else:
                if (list(instruction[1][i])[0] != '$'):
                    print("Error No.5 found in line %d, operand %d:\nR-type instructions do notaccept immediate operands."%(line_num, i+1)) #R指令不接受立即数
                    return False
                elif not(instruction[1][i] in register_map or instruction[1][i] in register_map_float):
                    print("Error No.6 found in line %d, operand %d:\nInvalid register operand."%(line_num, i+1)) #找不到寄存器$?
                    return False
    return True

def I_error_check(instruction,label_instructions):
    instruction_name = instruction[0]
    line_num = instruction[-1]
    num_of_operand = -1

    for operand in I_Type_dict[instruction[0]].values(): #统计操作数的数量
        if (operand != "00000"): 
            num_of_operand += 1
    if (len(instruction[1]) != num_of_operand):
        print("Error No.7 found in line %d:\nIncorrect number of operands. %d required but %d were given."%(line_num, num_of_operand ,len(instruction[1]))) #操作数有误
        return False
    else:
        if(num_of_operand == 2):
            if (list(instruction[1][0])[0] != '$'):
                print("Error No.8 found in line %d, operand 1:\nOperand 1 of an %s instruction must be an register operand."%(line_num,instruction_name)) #操作数类型有误
                return False
            elif (not(instruction[1][1]).isdigit()):
                print("Error No.9 found in line %d, operand 2:\nOperand 2 of an %s instruction must be an immediate operand."%(line_num,instruction_name)) #操作数类型有误
                return False
            elif not(instruction[1][0] in register_map):
                print("Error No.10 found in line %d, operand 1:\nInvalid register operand."%(line_num)) #找不到寄存器$?
                return False
            elif (int(instruction[1][1]) > 2**15-1 or int(instruction[1][1]) < -2**15):
                print("Error No.11 found in line %d, operand 2:\nImmediate operand out of range."%line_num)
                return False
        else:
            for i in range(2):
                if(list(instruction[1][i])[0] != '$'):
                    print("Error No.12 found in line %d, operand %d:\nOperand %d of an %s instruction must be an register operand."%(line_num, i+1, i+1,instruction_name)) #操作数类型有误
                    return False
                elif not(instruction[1][i] in register_map):
                    print(instruction[1][i] in register_map, instruction[1][i])
                    print("Error No.13 found in line %d, operand %d:\nInvalid register operand."%(line_num, i+1)) #找不到寄存器$?
                    return False
            if (instruction_name in I_Type_Branch):
                if not(instruction[1][2] in label_instructions):
                    print("Error No.14 found in line %d, operand 3:\nInvalid lable."%(line_num)) #找不到标签
                    return False
            else:
                if (not(instruction[1][2]).isdigit()):
                    print("Error No.15 found in line %d, operand 3:\nOperand 3 of an %s instruction must be an immediate operand."%(line_num,instruction_name)) #操作数类型有误
                    return False
                elif instruction_name in I_Type_Unsigned:
                    if (int(instruction[1][2]) > 2**16-1): #立即数超范围
                        print("Error No.16 found in line %d, operand 3:\nImmediate operand out of range."%line_num)
                        return False
                else:
                    if ((int(instruction[1][2]) > 2**15-1 or int(instruction[1][2]) < -2**15)): #立即数超范围
                        print("Error No.16.5 found in line %d, operand 3:\nImmediate operand out of range."%line_num)
                        return False
    return True

def LS_error_check(instruction):
    instruction_name = instruction[0]
    line_num = instruction[-1]
    num_of_operand=3 
    if (len(instruction[1]) != num_of_operand):
        print("Error No.17 found in line %d:\nIncorrect number of operands. %d required but %d were given."%(line_num, num_of_operand ,len(instruction[1]))) #操作数有误
        return False
    else:
        for i in range(2): #instruction[1]的顺序是：rs,rt,imm
            if(list(instruction[1][i])[0] != '$'):
                print("Error No.18 found in line %d, operand %d:\nOperand %d of an %s instruction must be an register operand."%(line_num, i+1, i+1,instruction_name)) #操作数类型有误
                return False
            elif not(instruction[1][i] in register_map):
                print("Error No.19 found in line %d, operand %d:\nInvalid register operand."%(line_num, i+1)) #找不到寄存器$?
                return False
        if (not(instruction[1][2]).isdigit()):
                print("Error No.20 found in line %d, operand 2:\nOperand 2 of an %s instruction must be an immediate operand."%(line_num,instruction_name)) #操作数类型有误
                return False
        elif (int(instruction[1][2]) > 2**15-1 or int(instruction[1][2]) < -2**15):
            print("Error No.21 found in line %d, operand 2:\nImmediate operand out of range."%line_num)
            return False
    return True


def J_error_check(instruction):
    num_of_operand = 1
    line_num = instruction[-1]
    if (len(instruction[1]) != num_of_operand):
        print("Error No.22 found in line %d:\nIncorrect number of operands. %d required but %d were given."%(line_num, num_of_operand ,len(instruction[1]))) #操作数有误
        return False
    else:
        if (instruction[0] in register_map):
            print("Error No.23 found in line %d:\nInvalid label name."%(line_num))
            return False
        elif (instruction[1][0].isdigit()):
            print("Error No.24 found in line %d:\nInvalid label name."%(line_num))
            return False
    return True

def error_check(assembly_code):
    error_backup=copy.deepcopy(assembly_code)
    for i, line in enumerate(error_backup):
        line = re.sub(r'0x[0-9a-fA-F]+', lambda x: str(int(x.group(), 16)), line)
        error_backup[i] = line

    label_counter = 0
    error_detect_label_dict = {}
    # print(error_backup)
    for i,line in enumerate(error_backup):
        line = re.sub(r'#.*', '', line)  # 移除注释
        error_backup[i] = line.strip()
        if error_backup[i]:
            if (error_backup[i]).endswith(':'):  # 处理标签
                label, _ = (error_backup[i]).split(':')
                error_detect_label_dict[label] = i - label_counter-1
                label_counter += 1
    # print(error_backup)
    line_num=0
    for i, line in enumerate(error_backup):
        line_num+=1
        line = re.sub(r',', '', line)
        if line:
            instruction = [line.split()[0],line.split()[1:],line_num]
            # print(instruction)
            type=0
            if (instruction[0] == 'nop'):
                if (len(instruction[1]) != 0):
                    print("Error No.25 found in line %d:\nIncorrect number of operands. 0 required but %d were given."%(line_num ,len(instruction[1]))) #操作数的数量有误
                    return False
            elif (instruction[0] == 'li'):
                if (len(instruction[1]) != 2):
                    print("Error No.26 found in line %d:\nIncorrect number of operands. 2 required but %d were given."%(line_num ,len(instruction[1]))) #操作数的数量有误
                    return False
                elif(list(instruction[1][0])[0] != '$'):
                    print("Error No.27 found in line %d, operand 1:\nOperand 1 of a %s instruction must be an register operand."%(line_num, instruction[0])) #操作数类型有误
                    return False
                elif not(instruction[1][0] in register_map):
                    print("Error No.28 found in line %d, operand %d:\nInvalid register operand."%(line_num, 1)) #找不到寄存器$?
                    return False
                elif (not(instruction[1][1]).isdigit()):
                    print("Error No.29 found in line %d, operand 2:\nOperand 2 of an %s instruction must be an immediate operand."%(line_num,instruction[0])) #操作数类型有误
                    return False
                elif (int(instruction[1][1]) > 2**31-1 or int(instruction[1][2]) < -2**31): #立即数超范围
                    print("Error No.30 found in line %d, operand 2:\nImmediate operand out of range."%line_num)
                    return False
            elif (instruction[0] in ["mv", "not", "neg"]):
                if (len(instruction[1]) != 2):
                    print("Error No.31 found in line %d:\nIncorrect number of operands. 2 required but %d were given."%(line_num ,len(instruction[1]))) #操作数的数量有误
                    return False
                for i in range(2):
                    if(list(instruction[1][i])[0] != '$'):
                        print("Error No.32 found in line %d, operand %d:\nOperand %d of a %s instruction must be an register operand."%(line_num, i+1, i+1, instruction[0])) #操作数类型有误
                        return False
                    elif not(instruction[1][i] in register_map):
                        print("Error No.33 found in line %d, operand %d:\nInvalid register operand."%(line_num, i+1)) #找不到寄存器$?
                        return False
            elif (is_R_Type(instruction)):
                if (not(R_error_check(instruction))):
                    return False
                else:
                    type='R'
            elif (is_I_Type(instruction)):
                if (not(I_error_check(instruction,error_detect_label_dict))):
                    return False
                else:
                    type='I'
            elif (is_LS_Type(instruction)):
                instruction[1] = [instruction[1][0]]+[''.join((list((instruction[1][1].split('('))[1]))[:-1])]+[(instruction[1][1].split('('))[0]]
                if (not(LS_error_check(instruction))):
                    return False
                else:
                    type='LS'
            elif (is_J_Type(instruction)):
                if (not(J_error_check(instruction))):
                    return False
                else:
                    type='J'
    else:
        return True



R_Type_dict =  {
                'sll'    : {'opcode':'000000', 'rs':'00000', 'rt':'rt',    'rd':'rd',    'shamt':'shamt', 'func':'000000'},
                'srl'    : {'opcode':'000000', 'rs':'00000', 'rt':'rt',    'rd':'rd',    'shamt':'shamt', 'func':'000010'},
                'sra'    : {'opcode':'000000', 'rs':'00000', 'rt':'rt',    'rd':'rd',    'shamt':'shamt', 'func':'000011'},
                'sllv'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'000100'},
                'srlv'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'000110'},
                'srav'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'000111'},
                'jr'     : {'opcode':'000000', 'rs':'rs',    'rt':'00000', 'rd':'00000', 'shamt':'00000', 'func':'001000'},
                'jalr'   : {'opcode':'000000', 'rs':'rs',    'rt':'00000', 'rd':'00000', 'shamt':'00000', 'func':'001001'}, 
                'syscall': {'opcode':'000000', 'rs':'00000', 'rt':'00000', 'rd':'00000', 'shamt':'00000', 'func':'001100'},
                'break  ': {'opcode':'000000', 'rs':'00000', 'rt':'00000', 'rd':'00000', 'shamt':'00000', 'func':'001101'},
                'mfhi'   : {'opcode':'000000', 'rs':'00000', 'rt':'00000', 'rd':'rd',    'shamt':'00000', 'func':'010000'},
                'mthi'   : {'opcode':'000000', 'rs':'rs',    'rt':'00000', 'rd':'00000', 'shamt':'00000', 'func':'010001'}, 
                'mflo'   : {'opcode':'000000', 'rs':'00000', 'rt':'00000', 'rd':'rd',    'shamt':'00000', 'func':'010010'},
                'mtlo'   : {'opcode':'000000', 'rs':'rs',    'rt':'00000', 'rd':'00000', 'shamt':'00000', 'func':'010011'}, 
                'mult'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'00000', 'shamt':'00000', 'func':'011000'}, 
                'multu'  : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'00000', 'shamt':'00000', 'func':'011001'},
                'div'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'00000', 'shamt':'00000', 'func':'011010'},
                'divu'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'00000', 'shamt':'00000', 'func':'011011'},
                'add'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100000'},
                'addu'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100001'},
                'sub'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100010'},
                'subu'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100011'},
                'and'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100100'},
                'or'     : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100101'},
                'xor'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100110'},
                'nor'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'100111'},
                'slt'    : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'101010'},
                'sltu'   : {'opcode':'000000', 'rs':'rs',    'rt':'rt',    'rd':'rd',    'shamt':'00000', 'func':'101011'},
                }



J_Type_dict =  {'j'      : {'opcode':'000010', 'rs': '00000', 'rt':'00000', 'addr':'label'},
                'jal'    : {'opcode':'000011', 'rs': '00000', 'rt':'00000', 'addr':'label'}, 
                            
                }

I_Type_dict = {
                'addi'   : {'opcode':'001000', 'rt':'rt', 'rs': 'rs',    'imm':'imm'},
                'addiu'  : {'opcode':'001001', 'rt':'rt', 'rs': 'rs',    'imm':'imm'}, #
                'daddi'  : {'opcode':'011000', 'rt':'rt', 'rs': 'rs',    'imm':'imm'}, #
                'daddiu' : {'opcode':'011001', 'rt':'rt', 'rs': 'rs',    'imm':'imm'}, #
                'slti'   : {'opcode':'001010', 'rt':'rt', 'rs': 'rs',    'imm':'imm'},
                'andi'   : {'opcode':'001100', 'rt':'rt', 'rs': 'rs',    'imm':'imm'},
                'ori'    : {'opcode':'001101', 'rt':'rt', 'rs': 'rs',    'imm':'imm'},
                'xori'   : {'opcode':'001110', 'rt':'rt', 'rs': 'rs',    'imm':'imm'},
                'lui'    : {'opcode':'001111', 'rt':'rt', 'rs': '00000', 'imm':'imm'},
                'beq'    : {'opcode':'000100', 'rs': 'rs', 'rt':'rt',    'imm':'label'},
                'bne'    : {'opcode':'000101', 'rs': 'rs', 'rt':'rt',    'imm':'label'},
                'blez'   : {'opcode':'000110', 'rs': 'rs', 'rt':'00000', 'imm':'label'},
                'bgtz'   : {'opcode':'000111', 'rs': 'rs', 'rt':'00000', 'imm':'label'}, 
                'bge'    : {'opcode':'010110', 'rs': 'rs', 'rt': 'rt', 'imm': 'label'},#
                'bgec'   : {'opcode':'010110', 'rs': 'rs', 'rt': 'rt', 'imm': 'label'},#
                'bltc'   : {'opcode':'010111', 'rs': 'rs', 'rt': 'rt', 'imm': 'label'},#
            }
I_Type_Unsigned = ['addiu','daddiu']
I_Type_Branch = ['beq', 'bne', 'blez', 'bgtz', 'bge', 'bgec', 'bltc']

F_Type_dict = {
                'add.s' : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000000'},
                'addf'  : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000000'},
                'add.d' : {'opcode':'010001', 'fmt':'10001', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000000'},
                'mul.s' : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000010'},
                'mulf'  : {'opcode':'010001', 'fmt':'10000', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000010'},
                'mul.d' : {'opcode':'010001', 'fmt':'10001', 'ft':'ft', 'fs':'fs', 'fd':'fd', 'func':'000010'},
                'mtc1'  : {'opcode':'010001', 'fmt':'00100', 'ft':'rt', 'fs':'fs', 'fd':'00000', 'func':'000000'},
                'mfc1'  : {'opcode':'010001', 'fmt':'00000', 'ft':'rt', 'fs':'fs', 'fd':'00000', 'func':'000000'},
                'dmtc1' : {'opcode':'010001', 'fmt':'00101', 'ft':'rt', 'fs':'fs', 'fd':'00000', 'func':'000000'},
                'dmfc1' : {'opcode':'010001', 'fmt':'00001', 'ft':'rt', 'fs':'fs', 'fd':'00000', 'func':'000000'},
                'mov.s' : {'opcode':'010001', 'fmt':'10000', 'ft':'00000', 'fs':'fs', 'fd':'fd', 'func':'000110'},
                'mov.d' : {'opcode':'010001', 'fmt':'10001', 'ft':'00000', 'fs':'fs', 'fd':'fd', 'func':'000110'}
            }
mov_r_f = ['mtc1', 'mfc1', 'dmtc1', 'dmfc1']
mov_f_f = ['mov.s', 'mov.d']


Load_Store_Type_dict = {
                'lb'   : {'opcode':'100000','rt':'rt', 'imm':'imm', 'rs':'rs'},
                'lbu'  : {'opcode':'100100','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'lh'   : {'opcode':'100001','rt':'rt', 'imm':'imm', 'rs':'rs'},
                'lhu'  : {'opcode':'100101','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'lw'   : {'opcode':'100011','rt':'rt', 'imm':'imm', 'rs':'rs'},
                'lwu'  : {'opcode':'100111','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'ld'   : {'opcode':'110111','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'l.s'  : {'opcode':'110001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'loadf': {'opcode':'110001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'l.d'  : {'opcode':'110101','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'sb'   : {'opcode':'101000','rt':'rt', 'imm':'imm', 'rs':'rs'},
                'sh'   : {'opcode':'101001','rt':'rt', 'imm':'imm', 'rs':'rs'},
                'sw'   : {'opcode':'101011','rt':'rt', 'imm':'imm', 'rs':'rs'},
                'sd'   : {'opcode':'111111','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                's.s'  : {'opcode':'111001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                'storef': {'opcode':'111001','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                's.d'  : {'opcode':'111101','rt':'rt', 'imm':'imm', 'rs':'rs'},#
                }


Pseudo_Type_dict = {
                'nop'    : {'opcode':'111111', 'func':'000001', 'rt':'00000', 'rs':'00000', 'imm':'00000'},
                'li'     : {'opcode':'111111', 'func':'000010', 'rt':'rt', 'rs':'00000', 'imm':'imm'},
                'mv'     : {'opcode':'111111', 'func':'000011', 'rt':'rt', 'rs':'rs', 'imm':'00000'},
                'not'    : {'opcode':'111111', 'func':'000100', 'rt':'rt', 'rs':'rs', 'imm':'00000'},
                'neg'    : {'opcode':'111111', 'func':'000101', 'rt':'rt', 'rs':'rs', 'imm':'00000'},
                }


register_map_mips = {
                    '$0': 0, '$at': 1, '$v0': 2, '$v1': 3, '$a0': 4, '$a1': 5, '$a2': 6, '$a3': 7,
                    '$t0': 8, '$t1': 9, '$t2': 10, '$t3': 11, '$t4': 12, '$t5': 13, '$t6': 14, '$t7': 15,
                    '$s0': 16, '$s1': 17, '$s2': 18, '$s3': 19, '$s4': 20, '$s5': 21, '$s6': 22, '$s7': 23,
                    '$t8': 24, '$t9': 25, '$k0': 26, '$k1': 27, '$gp': 28, '$sp': 29, '$fp': 30, '$ra': 31,
                    '$f0': 0, '$f1': 1, '$f2': 2, '$f3': 3, '$f4': 4, '$f5': 5, '$f6': 6, '$f7': 7,
                '$f8': 8, '$f9': 9, '$f10': 10, '$f11': 11, '$f12': 12, '$f13': 13, '$f14': 14, '$f15': 15,
                '$f16': 16, '$f17': 17, '$f18': 18, '$f19': 19, '$f20': 20, '$f21': 21, '$f22': 22, '$f23': 23,
                '$f24': 24, '$f25': 25, '$f26': 26, '$f27': 27, '$f28': 28, '$f29': 29, '$f30': 30, '$f31': 31
                }

register_map = {
                '$r0': 0, '$rax': 1, '$rcx': 2, '$rdx': 3, '$rbx': 4, '$rsp': 5, '$rbp': 6, '$rsi': 7,
                '$rdi': 8, '$r9': 9, '$r10': 10, '$r11': 11, '$r12': 12, '$r13': 13, '$r14': 14, '$r15': 15,
                '$r16': 16, '$r17': 17, '$r18': 18, '$r19': 19, '$r20': 20, '$r21': 21, '$r22': 22, '$r23': 23,
                '$r24': 24, '$r25': 25, '$r26': 26, '$r27': 27, '$r28': 28, '$r29': 29, '$r30': 30, '$r31': 31,
                '$f0': 0, '$f1': 1, '$f2': 2, '$f3': 3, '$f4': 4, '$f5': 5, '$f6': 6, '$f7': 7,
                '$f8': 8, '$f9': 9, '$f10': 10, '$f11': 11, '$f12': 12, '$f13': 13, '$f14': 14, '$f15': 15,
                '$f16': 16, '$f17': 17, '$f18': 18, '$f19': 19, '$f20': 20, '$f21': 21, '$f22': 22, '$f23': 23,
                '$f24': 24, '$f25': 25, '$f26': 26, '$f27': 27, '$f28': 28, '$f29': 29, '$f30': 30, '$f31': 31
            }

register_map_float = {
                
                }

parsed_instructions = []
p_instructions = []
label_instructions = {}
instructions = []

def binary_to_hex(binary_str):
    # 确保二进制字符串长度为32
    binary_str = binary_str.zfill(32)

    # 将32位二进制字符串每4位分割，并转换为16进制
    hex_str = ''
    for i in range(0, 32, 4):
        hex_digit = hex(int(binary_str[i:i+4], 2))[2:]
        hex_str += hex_digit

    return hex_str



def deal_with_Pseudo_Type(line):
    if line.split()[0] != 'li':
        if line.split()[0] == 'nop':
            return 'addi $0, $0, 0'
        elif line.split()[0] == 'mv':
            match = re.match(r'mv (\$\w+), (\$\w+)', line)
            if match:
                dest, src  = match.groups()
                if dest.startswith('$f') and src.startswith('$'):
                    if src.startswith('$f'):
                        return f'mov.d {src}, {dest}'
                    else:
                        return f'dmtc1 {src}, {dest}'
                elif src.startswith('$f') and dest.startswith('$'):
                    if dest.startswith('$f'):
                        return f'mov.d {src}, {dest}'
                    else:
                        return f'dmfc1 {src}, {dest}'
                else:
                    return f'addi {dest}, {src}, 0'
        elif line.split()[0] == 'not':
            return re.sub(r'not (\$\w+), (\$\w+)', r'xori \1, \2, -1', line)
        elif line.split()[0] == 'neg':
            return re.sub(r'neg (\$\w+), (\$\w+)', r'sub \1, $0, \2', line)
        else:
            return 'error_li'
    return line
            
def signed_extend_to_16bit(immediate):
    immediate = int(immediate)
    if immediate < 0:
        imm_bin = format(2**16 + immediate, '016b')
    else:
        imm_bin = format(immediate, '016b')
    return str(imm_bin)
def unsigned_extend_to_16bit(immediate):
    immediate = int(immediate)
    return format(immediate, '016b')
def preprocess_mips_assembly(assembly_code):
    instructions = []
    label_dict = {}
    label_counter = 0
    
    for i, line in enumerate(assembly_code):
        line = re.sub(r'#.*', '', line)  # 移除注释
        line = line.strip()
        if line:
            if line.endswith(':'):  # 处理标签
                label, _ = line.split(':')
                label_dict[label] = i - label_counter
                label_counter += 1
            else:
                if line.split()[0] in Pseudo_Type_dict.keys():
                    if line.split()[0] != 'li':
                        instructions.append(deal_with_Pseudo_Type(line))
                    elif line.split()[0] == 'li':
                        immediate = int(line.split(",")[-1].strip(), 0)  # 获取立即数，这里的0会根据字符串的前缀自动识别其进制
                        immediate_binary = format(immediate, '032b')  # 转换为32位二进制字符串

                        # 生成 LUI 和 ORI 指令
                        upper_imm = int(immediate_binary[:16],2)  # 取前16位作为 LUI 的立即数
                        lower_imm = int(immediate_binary[16:],2)  # 取后16位作为 ORI 的立即数
                        target_register = line.split(",")[0].split("$")[-1].strip()  # 获取目标寄存器

                        lui_instruction = f"lui ${target_register}, {upper_imm}"  # 生成 LUI 指令
                        ori_instruction = f"ori ${target_register}, ${target_register}, {lower_imm}"  # 生成 ORI 指令
                        instructions.append(lui_instruction)
                        #print(lui_instruction)
                        instructions.append(ori_instruction)
                        i = i+1
                        
                    else:
                        print('error')
                else:
                    instructions.append(line)
                    

    # 将16进制转换为10进制
    for i, ins in enumerate(instructions):
        ins = re.sub(r'0x[0-9a-fA-F]+', lambda x: str(int(x.group(), 16)), ins)
        instructions[i] = ins

    return instructions, label_dict

def is_hlt(instruction):
    return instruction[0] == 'hlt'

#R类型指令解析：
def is_R_Type(instruction):
    return instruction[0] in R_Type_dict

def parse_R_Type(instruction):
    operator, operands = instruction
    opcode = R_Type_dict[operator]['opcode']
    rs = R_Type_dict[operator]['rs']
    rt = R_Type_dict[operator]['rt']
    rd = R_Type_dict[operator]['rd']
    shamt = R_Type_dict[operator]['shamt']
    func = R_Type_dict[operator]['func']
    
    # 继续添加其他步骤，根据具体指令格式提取对应信息
    return (operator, opcode, rs, rt, rd, shamt, func, operands)

def get_R_Type_machine_code(ins_couple):
    global PC
    PC += 1
    res_code = ""
    res_code += ins_couple[1]
    if ins_couple[0] == 'jalr':
        parts = ins_couple[-1][0].split()
        if(len(parts) == 1):
            res_code += str(bin(register_map[parts[0]])[2:].rjust(5,'0')) #rs
            res_code += '00000'#rt
            res_code += str(bin(register_map['$ra'])[2:].rjust(5,'0')) #rd
            res_code += '00000'
            
        elif(len(parts) == 2):
            res_code += str(bin(register_map[parts[1]])[2:].rjust(5,'0')) #rs
            res_code += '00000'#rt
            res_code += str(bin(register_map[parts[0]])[2:].rjust(5,'0')) #rd
            res_code += '00000'
            
        else:
            print('invalid jalr')
            PC -=1
    elif (ins_couple[0]=='sll') or (ins_couple[0]=='srl') or(ins_couple[0]=='sra'):
        res_code += ins_couple[2] #rs
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5,'0')) #rt
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5,'0')) #rd
        res_code += str(bin(int(ins_couple[-1][2]))[2:].rjust(5,'0'))#shamt
       
    elif (ins_couple[0]=='sllv') or (ins_couple[0]=='srlv') or(ins_couple[0]=='srav'):
        res_code += str(bin(register_map[ins_couple[-1][2]])[2:].rjust(5,'0')) #rs
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5,'0')) #rt
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5,'0')) #rd
        res_code += '00000' #shamt
        
    elif ins_couple[0] == 'jr' or ins_couple[0] == 'mthi' or ins_couple[0] == 'mtlo':
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5,'0')) #ra
        res_code += ins_couple[3]
        res_code += ins_couple[4]
        res_code += ins_couple[5]
        
    elif ins_couple[0] == 'break' or ins_couple[0] == 'syscall':
        res_code += '00000'
        res_code += '00000'
        res_code += '00000'
        res_code += ins_couple[5]
        
    elif ins_couple[0] == 'mfhi' or ins_couple[0] == 'mflo':
        res_code += '00000'
        res_code += '00000'
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5,'0')) #rd
        res_code += '00000' #shamt
        
    elif ins_couple[0] == 'div' or ins_couple[0] == 'divu' or ins_couple[0] == 'mult' or ins_couple[0] == 'multu':
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5,'0')) #rs
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5,'0')) #rt
        res_code += '00000'
        res_code += '00000' #shamt
        
    else:
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5,'0')) #rs
        res_code += str(bin(register_map[ins_couple[-1][2]])[2:].rjust(5,'0')) #rt
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5,'0')) #rd
        res_code += ins_couple[5]  #shamt
        
    res_code += ins_couple[-2] #func
    
    return res_code

#I类型指令解析：
def is_I_Type(instruction):
    return instruction[0] in I_Type_dict

def parse_I_Type(instruction):
    operator, operands = instruction
    opcode = I_Type_dict[operator]['opcode']
    rs = I_Type_dict[operator]['rs']
    rt = I_Type_dict[operator]['rt']
    imm = I_Type_dict[operator]['imm']

    return (operator, opcode, rs, rt, imm, operands)

def get_I_Type_machine_code(ins_couple):
    global PC
    PC += 1
    res_code = ""
    res_code += ins_couple[1]
    
    if (ins_couple[0] == 'lw') or (ins_couple[0] == 'sw'):
        op, oper = ins_couple
        register_offset = re.findall(r'\$?\w+|\d+', oper[1])
        temp = [op, oper[0]] + register_offset
        res_code += str(bin(register_map[temp[2]])[2:].rjust(5, '0'))  # rs
        res_code += str(bin(register_map[temp[1]])[2:].rjust(5, '0'))  # rt
        res_code += signed_extend_to_16bit(temp[-1])  # imm
    
    elif (ins_couple[0] == 'bne') or (ins_couple[0] == 'beq'):
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # rs
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5, '0'))  # rt
        res_code += signed_extend_to_16bit(str(label_instructions[ins_couple[-1][2]] - PC + 1))  # imm
    
    elif (ins_couple[0] == 'addi') or (ins_couple[0] == 'slti'):
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5, '0'))  # rs
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # rt
        res_code += signed_extend_to_16bit(ins_couple[-1][2])  # imm
    
    elif ins_couple[0] == 'lui':
        res_code += '00000'
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # rt
        res_code += str(bin(int(ins_couple[-1][1]))[2:].zfill(16))
    
    elif ins_couple[0] == 'blez' or ins_couple[0] == 'bgtz':
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # rs
        res_code += '00000'
        res_code += signed_extend_to_16bit(str(label_instructions[ins_couple[-1][1]] - PC + 1))
    
    elif ins_couple[0] == 'bge' or ins_couple[0] == 'bgec' or ins_couple[0] == 'bltc':
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # rs
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5, '0'))  # rt
        res_code += signed_extend_to_16bit(str(label_instructions[ins_couple[-1][2]] - PC + 1))  # imm
    
    else:
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5, '0'))  # rs
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # rt
        res_code += str(bin(int(ins_couple[-1][2]))[2:].zfill(16))  # imm
    
    return res_code


# F类型指令解析：
def is_F_Type(instruction):
    return instruction[0] in F_Type_dict.keys()

def parse_F_Type(instruction):
    operator, operands = instruction
    opcode = F_Type_dict[operator]['opcode']
    fmt = F_Type_dict[operator]['fmt']
    ft = F_Type_dict[operator]['ft']
    fs = F_Type_dict[operator]['fs']
    fd = F_Type_dict[operator]['fd']
    func = F_Type_dict[operator]['func']
    
    # 继续添加其他步骤，根据具体指令格式提取对应信息
    return (operator, opcode, fmt, ft, fs, fd, func, operands)

def get_F_Type_machine_code(ins_couple):
    global PC
    PC += 1
    res_code = ""
    res_code += ins_couple[1]  # opcode
    res_code += ins_couple[2]  # fmt
    if ins_couple[0] in mov_r_f:
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # rt
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5, '0'))  # fs
        res_code += "00000"
    elif ins_couple[0] in mov_f_f:
        res_code += "00000"
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5, '0'))  # fs
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # fd
    else:
        res_code += str(bin(register_map[ins_couple[-1][2]])[2:].rjust(5, '0'))  # ft
        res_code += str(bin(register_map[ins_couple[-1][1]])[2:].rjust(5, '0'))  # fs
        res_code += str(bin(register_map[ins_couple[-1][0]])[2:].rjust(5, '0'))  # fd
    res_code += ins_couple[-2]  # func    
    return res_code



#Load_Store_类型指令解析：
def is_LS_Type(instruction):
    return instruction[0] in Load_Store_Type_dict

def parse_LS_Type(instruction):
    operator, operands = instruction
    opcode = Load_Store_Type_dict[operator]['opcode']
    rs = Load_Store_Type_dict[operator]['rs']
    rt = Load_Store_Type_dict[operator]['rt']
    imm = Load_Store_Type_dict[operator]['imm']

    return (operator, opcode, rs, rt, imm, operands)

def get_LS_Type_machine_code(ins_couple):
    global PC
    res_code = ""
    res_code += ins_couple[1]
    

    temp = []
    temp.append(ins_couple[0])  # 添加指令
    temp.append(ins_couple[5][0])  # 添加目标寄存器
    offset = ins_couple[5][1].split('(')
    
    temp.append(offset[1].replace(')', '')) 
    temp.append(offset[0])
    res_code += str(bin(register_map[temp[2]])[2:].rjust(5,'0')) #rs
    res_code += str(bin(register_map[temp[1]])[2:].rjust(5,'0')) #rt
    res_code += signed_extend_to_16bit(temp[3])
    PC += 1

    return res_code



#J类型指令解析：
def is_J_Type(instruction):
    return instruction[0] in J_Type_dict

def parse_J_Type(instruction):
    operator, operands = instruction
    opcode = J_Type_dict[operator]['opcode']
    return (operator, opcode, operands[0], operands)

def get_J_Type_machine_code(ins_couple):
    global PC
    PC += 1
    res_code = ""
    res_code += ins_couple[1]
    if(label_instructions[ins_couple[2]] - PC >= 0):
        res_code += str(bin(label_instructions[ins_couple[2]] - PC + 1))[2:].rjust(26,'0') #j & jal
    else:
        res_code += str(bin(abs(label_instructions[ins_couple[2]] - PC + 1)*4))[3:].rjust(26,'0')
        

    return res_code


parser = argparse.ArgumentParser(
    description="MIPS Assembler",
    usage="python mips_assembler.py [-h] [-p] <input_file> <output_file>"
)
parser.add_argument("input_file", help="Path to the assemble code")
parser.add_argument("output_file", help="Path to the binary code")
parser.add_argument("-p", "--print", action="store_true", help="Print the output")

args = parser.parse_args()

path = args.input_file
output_file = args.output_file
if_print = args.print

# path = "/home/ladev789/CPP/CA/mips_asm_input.txt"
# output_file = "/home/ladev789/CPP/CA/bincode.COE"
# if_print = True


with open(path, 'r',encoding = 'UTF-8') as file:
    # 逐行读取文件内容
    lines = file.readlines()
    
    # 去掉每行末尾的换行符并将每行存储到列表中
    line_list = [line.rstrip() for line in lines]
    for i, line in enumerate(line_list):
        line = re.sub(r'#.*', '', line)  # 移除注释
        line = re.sub(r'//.*', '', line)  # 移除注释
        line_list[i] = line.strip()

new_line_list = []
for line in line_list:
    if ':' in line:
        label, inst = line.split(':')
        new_line_list.append(label.strip()+':')
        # new_line_list.append(inst.strip())
    else:
        if line:
            new_line_list.append(line)
PC = 0
p_instructions, label_instructions = preprocess_mips_assembly(new_line_list)
instructions = p_instructions
for instruction in instructions:
    if instruction != "hlt":
        operator, operands = re.match(r'(\w+)\s(.+)', instruction).groups()
        operands = operands.split(',')
        operands = [operand.strip() for operand in operands]
        parsed_instructions.append((operator, operands))
    else:
        parsed_instructions.append(("hlt", []))

# print('—'*80)
f = open(output_file,'w')
try:
    if(error_check(new_line_list)):
        for ins in parsed_instructions:
            if is_R_Type(ins):
                temp = parse_R_Type(ins)
                bincode = binary_to_hex(get_R_Type_machine_code(temp))
                if if_print:
                    print(ins[0], bincode)
                f.write(bincode)
                f.write('\n')
            elif is_I_Type(ins):
                temp = parse_I_Type(ins)
                bincode = binary_to_hex(get_I_Type_machine_code(temp))
                if if_print:
                    print(ins[0], bincode)
                f.write(bincode)
                f.write('\n')
            elif is_J_Type(ins):
                temp = parse_J_Type(ins)
                bincode = binary_to_hex(get_J_Type_machine_code(temp))
                if if_print:
                    print(ins[0], bincode)
                f.write(bincode)
                f.write('\n')
            elif is_LS_Type(ins):
                temp = parse_LS_Type(ins)
                bincode = binary_to_hex(get_LS_Type_machine_code(temp))
                if if_print:
                    print(ins[0], bincode)
                f.write(bincode)
                f.write('\n')
            elif is_F_Type(ins):
                temp = parse_F_Type(ins)
                bincode = binary_to_hex(get_F_Type_machine_code(temp))
                if if_print:
                    print(ins[0], bincode)
                f.write(bincode)
                f.write('\n')
            elif is_hlt(ins):
                if if_print:
                    print("hlt","0"*8)
                f.write(binary_to_hex("0"*32))
                f.write('\n')
            else:
                print("Error No.34: Unknown type of instruction.")
except Exception as e:
    print(f'写入文件报错: {e}')
finally:
    f.close()
f.close()
