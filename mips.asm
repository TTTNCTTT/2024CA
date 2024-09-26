# 这是注释

#测试R类型指令
add $t1, $0, $0
add $ra, $0, $0
add $t2 , $0, $t1
sub $t3, $t2, $t1
and $t4, $t2, $0
or  $s3, $t4, $t1
xor $s2, $t1, $t2
nor $t4, $0, $t3
slt $s1, $s2, $t2
sll $s3, $t2, 5
srl $s3, $t2, 5
sra $s3, $t2, 5
sllv $s3, $t2, $t1
srlv $s3, $t2, $t1
srav $s3, $t2, $t1
Again:
jr $ra
jalr $s2
mult $t2, $t3
mfhi $s1
mflo $s2
mthi $t1
mtlo $t2



#测试I类型指令
addi $t1, $0, 128
addi $t2, $0, 0
slti $s1, $s2, 33
andi $s1, $s2, 54
lui $s1, 0x1234			#需要能识别16进制数
ori $s1, $s1, 0x5678	#需要能识别16进制数
xori $t1, $s1, 59
	sw $t0, 55($0)
	sw $t1, 0x47($0)
	lw $s3, 55($0)
	lw $s1, 0x47($0)
j OK
addi $t2, $s0, 45
OK:
	beq $t0, $0, Fail
	addi $t2, $s0, 54
Fail:
	addi $t2, $s0, 71
	beq $t2, $0, Finish
	slti $s3, $t1, 98
Finish:
	ori $t1, $s3, 91
	jal Again

#错误指令测试，需要给出错误的行号，并指向错误之处
#合法性判断，应该检查下列事项：
# 1、是否是合法的操作码
# 2、操作数个数、类型是否正确
# 3、（立即数）操作数是否超出范围
# 4、对J类指令，标签是否存在

#add $t1, $t0, 34	# R类型指令不接受立即数操作数
#sub $t2, 65, $s1	# R类型指令不接受立即数操作数
#xori $s1, $t2, $s1	# I类型指令最后一个必须是立即数
#andi $s1, $t1, 0x123456	# I类型指令立即数超出范围（16位）
#xori $s2, 111, $s1  # I类型指令第二个操作数必须是寄存器
#j 5456				# 标签必须是有效标识符
#jr OK				# JR指令，必须是寄存器操作数
#jalr xx				# JALR指令，必须是寄存器操作数
#mhfi $s1, $s2		# MHFI指令只能有一个操作数
#mhlo $t2, xxx		# MHLO指令只能有一个操作数
#sll $s3, $t2, 51	# 立即数操作数超出范围




#	main:	addi $2, $0, 5
#			addi $3, $0, 12
#			addi $7, $3, -9
#			or    $4, $7, $2
#			and  $5, $3, $4
#			add  $5, $5, $4
#			beq  $5, $7, end
#			slt    $4, $3, $4
#			beq  $4, $0, around
#			addi $5, $0, 0
#	around:    slt     $4, $7, $2
#		        add  $7, $4, $5
#		        sub  $7, $7, $2
#			sw   $7, 68($3)
#			lw    $2, 80($0)
#			j      end
#			addi $2, $0, 1
#	end:		sw   $2, 84($0)
#			lw    $1, 84($0)