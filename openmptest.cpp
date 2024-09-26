// #include <iostream>
// #include <omp.h>

#include<omp.h>
 
#include<iostream>
#include <time.h>
void test() {
  int a = 0;
  for (int i = 0; i < 100000000; i++)
    a++;
}
// int main() {
//   clock_t t1 = clock();
//   #pragma omp parallel for
//   for (int i = 0; i < 80; i++)
//     test();
//   clock_t t2 = clock();
//   std::cout << "time: " << (double)(t2 - t1) / CLOCKS_PER_SEC << std::endl;
// }

 
int main()
 
{
 
    std::cout << "parallel begin:\n";
 
    #pragma omp parallel
 
    {
 
        std::cout << omp_get_thread_num();
 
    }
 
    std::cout << "\n parallel end.\n";
 
    std::cin.get();
 
    return 0;
 
}