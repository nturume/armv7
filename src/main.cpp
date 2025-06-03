#include <cstdio>
#include <cstdlib>
#include<cstring>
#include "./elf.hpp"
#include "./stuff.hpp"

 fn main() -> i32 {
  Elf loader("./build/elf");
  Elf::ProgramHeaderIterator iter(loader.file, loader.header);

  Elf::Ph ph = {}; 
    
  if(iter.next(&ph) == nullptr) {
    return 0;
  }
}
