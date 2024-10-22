# MIPS64 CPU

使用 C 实现的 MIPS64 处理器模拟器，分为单周期与多周期版本。实现的指令集为 MIPS64 的子集，编码方式与 MIPS64 保持一致，寄存器命名使用 Intel 风格。

## 使用方法

### 快速运行——使用`makefile`：

```shell
make all    #运行MIPS64汇编器、编译单周期与多周期CPU
make cpu    #运行单周期CPU
make cpu_m  #运行多周期CPU
make cpu_p  #运行流水线CPU
make cpu_p_m #运行多进程版流水线CPU
```

### 分步使用

-   `mips_assembler.py`文件——汇编器：

```shell
python mips_assembler.py [-h] [-p] <input_file> <output_file>
```

参数`-h`表示打印帮助信息，`-p`表示打印过程。

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

-   `pipeline.c`文件——流水线 CPU：

```shell
gcc -o pipeline pipeline.c
./pipeline <instruction_file>
```

-   `pipeline_multi_process.c`文件——流水线 CPU：

```shell
gcc -o pipeline_multi_process pipeline_multi_process.c
./pipeline_multi_process [-p] <instruction_file>
```

参数`-p`表示打印过程。
