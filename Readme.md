**使用方法**

汇编器：

```shell
python mips_assembler.py [-h] [-p] <input_file> <output_file>
```

单周期 CPU：

```shell
gcc -o mips_cpu mips_cpu.c
./mips_cpu <instruction_file>
```

多周期 CPU：

```shell
gcc -o mips_cpu_multicycle mips_cpu_multicycle.c
./mips_cpu_multicycle <instruction_file>
```
