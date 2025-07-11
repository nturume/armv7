#pragma once
#include "stuff.hpp"
#include <cstdio>
#include <cstring>

inline u32 assemble(const char *program) {
  u8 buf[1024] = {0};
  if (strlen(program) > sizeof(buf)) {
    printf("program too large\n");
    return 0;
  }

  const char *fmt = "echo \"%s\" | arm-none-eabi-as -o elf.tmp\n";
  sprintf((i8 *)buf, fmt, program);
  if (system((i8 *)buf) == -1) {
    printf("failed to run assembler\n");
    return 0;
      }
  // printf("=========disasm==============\n");
  // if(system("arm-none-eabi-objdump -d elf.tmp")==-1){
  //   printf("objdump failed\n");
  //   exit(1);
  // }
  // printf("========= disasm ============\n");
  if (system("arm-none-eabi-objcopy -O binary elf.tmp bin.tmp\n") == -1) {
    printf("objcopy failed\n");
    return 0;
  }
  // printf("\n==============hex=============\n");
  // if(system("hexdump -C bin.tmp")==-1) {
  //   printf("failed to hexdump\n");
  //   exit(1);
  // }
  // printf("=============hex===============\n");
  FILE *bin = fopen("bin.tmp", "r");
  if (!bin) {
    printf("failed to open bin file\n");
    return 0;
  }
  u32 instr = 0;
  if (fread(&instr, 1, 4, bin) != 4) {
    printf("failed to read Instr\n");
    return 0;
  }
  if (system("rm *.tmp\n") == -1) {
    printf("failed to rm tmp files\n");
    return 0;
  }
  return instr;
}
