#include <cstdio>
#include <cstdlib>
#include<cstring>
#include "./elf.hpp"


int main() {
  Elf loader("./build/elf");
  Elf::ProgramHeaderIterator iter(loader.file, loader.header);

  Elf::Ph ph = {}; 
    
  if(iter.next(&ph) == nullptr) {
    return 0;
  }

  printf("---: %u\n",ph.p_filesz);
  
  printf("hello world: size of elf32 header: %hu\n", loader.header.e_phnum);
}
