
#include<stdio.h>
#include<assert.h>
#include<string.h>

unsigned long long a1[1053];
unsigned long long a2[1053];
unsigned long long a3[1053];

int main(int argc, char *argv[], char *envp[]) {
  // printf("SIZEOF A1: %lu\n", sizeof(a1));
  // fflush(stdout);
  // memset(a1, 0xff, sizeof(a1));
  // for(int i = 0; i < sizeof(a1)/8; i++) {
  //   assert(a1[i] == 0xffffffffffffffff);
  // }
  // memcpy(a2,a1, sizeof(a1));
  // assert(!memcmp(a2, a1, sizeof(a1)));
  // memmove(a3, a2, sizeof(a1));
  // assert(!memcmp(a3, a1, sizeof(a1)));

  // while(*envp) {
  //   int len = strnlen(*envp, 20);
  //   printf("len: %d  -> %s\n", len, *envp);
  //   envp+=1;
  // }

  // float a = 6.0;
  // float b = 3.0;
  // float c = a * b;
  //assert(c36.1);

  printf("FLOAT %.1f\n", 18.0);
  
  return 0;
}

