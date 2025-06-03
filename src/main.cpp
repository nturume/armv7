#include <cstdio>
#include <cstdlib>
#include<cstring>
#include "./elf.hpp"
#include "./stuff.hpp"
#include "./mem.hpp"

Memory<1024> m;

fn main() -> i32 {

  m.writeLE<u32>(0, 0xf00ff00f);
  u32 res = m.readLE<u32>(0);

  printf("res: %xu\n", res);

  // Elf loader("./build/elf");
  // Elf::ProgramHeaderIterator iter(loader.file, loader.header);

  // Elf::Ph ph = {}; 
    
  // if(iter.next(&ph) == nullptr) {
  //   return 0;
  // }
}
