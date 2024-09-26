#include <stdint.h>
#include <stdio.h>

int main() {
  uint16_t small_num = 0xFFFF;              // 16位无符号整数
  uint32_t large_num = (uint32_t)small_num; // 转换为32位无符号整数

  printf("small_num: 0x%X\n", small_num);
  printf("large_num: 0x%X\n", large_num);

  return 0;
}
