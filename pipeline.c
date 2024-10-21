#include "./pipeline.h"
#include <pthread.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    print_usage(argv[0]);
    return 1;
  }

  const char *instruction_file = argv[1];

  printf("存入的待计算数据\n");
  for (int i = 0; i < 64; i++) {
    store_float(i * 4, 0.1 * (i + 1) + 4);
    printf("f[%d]:%.2f\t", i, load_float(i * 4));
    if (!((i + 1) % 4))
      printf("\n");
  }
  printf("\n使用c直接计算出的标准答案\n");
  load_instructions(instruction_file);
  execute_instructions();
  pipeline_excute();
  for (int i = 0; i < 64; i++) {
    printf("f[%d]:%.2f\t", i, (0.1 * (i + 1) + 4) * 0.9 + 0.5);
    if (!((i + 1) % 4))
      printf("\n");
  }
  printf("\n读取的结果数据\n");
  for (int i = 0; i < 64; i++) {
    printf("f[%d]:%.2f\t", i, load_float(i * 4));
    if (!((i + 1) % 4))
      printf("\n");
  }
  printf("\nInstruction count:%d\n", ins_count);
  printf("\nCycle count:%d\n", cycle_count);
  printf("\nCPI:%.4f\n", ((double)cycle_count / (double)ins_count));

  return 0;
}