# MIPS64 CPU

使用 C 实现的 MIPS64 处理器模拟器，分为单周期与多周期版本。实现的指令集为 MIPS64 的子集，编码方式与 MIPS64 保持一致，寄存器命名使用 Intel 风格。多进程流水线版本中还引入了模拟 Cache。

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

- `mips_assembler.py`文件——汇编器：

```shell
python mips_assembler.py [-h] [-p] <input_file> <output_file>
```

参数`-h`表示打印帮助信息，`-p`表示打印过程。

- `mips_cpu.c`文件——单周期 CPU：

```shell
gcc -o mips_cpu mips_cpu.c
./mips_cpu <instruction_file>
```

- `mips_cpu_multicycle.c`文件——多周期 CPU：

```shell
gcc -o mips_cpu_multicycle mips_cpu_multicycle.c
./mips_cpu_multicycle <instruction_file>
```

- `pipeline.c`文件——流水线 CPU：

```shell
gcc -o pipeline pipeline.c
./pipeline <instruction_file>
```

- `pipeline_multi_process.c`文件——流水线 CPU：

```shell
gcc -o pipeline_multi_process pipeline_multi_process.c
./pipeline_multi_process [-p] <instruction_file>
```

参数`-p`表示打印过程。

# 模拟器设计说明

### 1. Cache 设计方案

#### Cache 结构与参数

- **结构**：实现了一个四组两路组相联的缓存（即 2-way set associative cache），通过 LRU（最近最少使用）策略替换缓存块。
- **参数**：缓存的块大小为 16 字节（即包含 4 个浮点数`float`），共有 4 组，每组 2 个块。写缓冲区有 4 个槽，用于暂存写回主存的脏块。以下宏定义具体参数：
  ```c
  #define DC_NUM_SETS 4     // 缓存的组数
  #define DC_SET_SIZE 2     // 每组的块数
  #define DC_BLOCK_SIZE 16  // 块大小（4个float）-> 每4个load操作miss一次
  #define DC_WR_BUFF_SIZE 4 // 写缓冲槽位数
  ```

#### Cache 集成方式

缓存访问是在访存阶段（MEM）集成的，访问内存的指令`loadf`和`storef`均调用`accessDCache`函数。对于读写操作，`accessDCache`函数会处理命中、不命中、替换以及写缓冲的相应逻辑。

#### Cache 延迟计算

- **`accessDCache`函数**：主要根据操作类型和命中状态计算延迟。读写命中时，不会增加延迟；读写不命中时，根据是否需要替换脏块来决定延迟。
- **`wrBack`函数**：用于处理写回操作延迟。若写缓冲满了，则会先将缓冲的内容写回主存，并增加写回延迟（`WR_BUFF_FLUSH_LATENCY`），然后再将新的脏块写入写缓冲，增加`WR_BUFF_WR_LATENCY`延迟。

### 2. Cache 接入流水线的位置

缓存系统在流水线访存阶段（MEM 阶段）调用。`memory`函数中，`loadf`和`storef`指令会调用`accessDCache`函数，从而在流水线访存阶段执行缓存访问并计算延迟，将延迟累加到当前周期数上，以便计算实际流水线周期数和 CPI。

### 3. 五级流水的功能与时序模拟设计

模拟器实现了一个**五级流水线结构**，包括取指(IF)、译码(ID)、执行(EX)、访存(MEM)和写回(WB)五个阶段。主循环`execute_instructions`函数通过`switch-case`结构逐级推进各阶段，并借助全局状态`current_state`的变更来控制当前指令所处的流水阶段。

流水线调度逻辑在`pipeline_execute`函数中实现，其中的各级流水寄存器（`StageF`, `StageD`, `StageE`, `StageM`, `StageW`）用于在每个周期传递指令数据。函数会根据执行结果更新流水寄存器、控制冲突和数据依赖，并计算相应的延迟和周期开销。
