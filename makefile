# Makefile for MIPS CPU project


PYTHON_SCRIPT = mips_assembler.py
ASM_INPUT = mips_asm_input.txt
BIN_OUTPUT = bin
CC = gcc
CFLAGS = -o


all: mips_asm mips_cpu mips_cpu_multicycle pipeline pipeline_multi_process
	
mips_asm: mips_asm_input.txt
	python $(PYTHON_SCRIPT) $(ASM_INPUT) $(BIN_OUTPUT)

mips_cpu: mips_cpu.c
	$(CC) $(CFLAGS) mips_cpu mips_cpu.c

mips_cpu_multicycle: mips_cpu_multicycle.c
	$(CC) $(CFLAGS) mips_cpu_multicycle mips_cpu_multicycle.c

pipeline: pipeline.c
	$(CC) $(CFLAGS) pipeline pipeline.c

pipeline_multi_process: pipeline_multi_process.c
	$(CC) $(CFLAGS) pipeline_multi_process pipeline_multi_process.c

cpu:
	./mips_cpu $(BIN_OUTPUT)

cpu_m:
	./mips_cpu_multicycle $(BIN_OUTPUT)

cpu_p:
	./pipeline $(BIN_OUTPUT)

cpu_p_m:
	./pipeline_multi_process $(BIN_OUTPUT)

clean:
	rm -f mips_cpu mips_cpu_multicycle pipeline pipeline_multi_process $(BIN_OUTPUT)
