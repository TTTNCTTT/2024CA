#include <stdio.h>
#include <stdint.h>

int main() {
    int32_t int_value = 0x40866666; // 示例int32_t值
    float float_value;

    // 使用指针将int32_t的二进制表示赋值给float
    float_value = *((float*)&int_value);

    // 打印结果
    printf("int_value: %d\n", int_value);
    printf("float_value: %f\n", float_value);
    printf("float_value (as int): %d\n", *((int32_t*)&float_value));

    return 0;
}
