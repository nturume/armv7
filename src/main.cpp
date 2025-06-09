#include "./decoder.hpp"
#include "./elf.hpp"
#include "./mem.hpp"
#include "./bin.hpp"
#include "./stuff.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

Memory<1024> m;

#ifndef TESTING
fn main() -> i32 {
  u32 instr =  assemble(
           "wfi"
           ""
         );
  // Decoder::test();
    
  //       Elf elf("./build/elf");
  // Elf::ProgramHeaderIterator iter(elf.file, elf.header);

  // m.loadElf(iter);

  // u32 instr = m.readBE<u32>(elf.header.e_entry);

  // printf("Instruction: %x\n", instr);
  // Decoder::decodeA(instr);
  // Elf::Ph ph = {};

  // if(iter.next(&ph) == nullptr) {
  //   return 0;
  // }
}
#else

int main() {
  Memory<64>::test();
  Decoder::test();
  return 0;
}

#endif
