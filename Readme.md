**说明**

使用 C 实现的 MIPS64 处理器模拟器，分为单周期与多周期版本。实现的指令集为 MIPS64 的子集，编码方式与 MIPS64 保持一致，寄存器命名使用 Intel 风格。

**使用方法**

-   `mips_assembler.py`文件——汇编器：

```shell
python mips_assembler.py [-h] [-p] <input_file> <output_file>
```

-   `mips_cpu.c`文件——单周期 CPU：

```shell
gcc -o mips_cpu mips_cpu.c
./mips_cpu <instruction_file>
```

-   `mips_cpu_multicycle.c`文件——多周期 CPU：

```shell
gcc -o mips_cpu_multicycle mips_cpu_multicycle.c
./mips_cpu_multicycle <instruction_file>
```
