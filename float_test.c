// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #define MAXSIZE 1024
// #define REGCNT 64
// // Intel风格
// typedef int8_t  byte;
// typedef int16_t word;
// typedef int32_t dword;
// typedef int64_t qword;

// int main() {
//     printf("%#16llx",(long long int)0.5);
//     return 0;
// }

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
int main() {
    clock_t start = clock();
    float num = 0.5;
    uint32_t hex;
    memcpy(&hex, &num, sizeof(float));
    printf("%#16llx\n", hex);
    return 0;
}
