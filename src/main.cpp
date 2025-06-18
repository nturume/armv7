#include "./decoder.hpp"
#include "./elf.hpp"
#include "./mem.hpp"
#include "./bin.hpp"
#include "./stuff.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "arith.hpp"
#include "cpu.hpp"


#ifndef TESTING
int  main() {
  //Cpu::test();
  // Cpu c;
  // u32 instr =  assemble("adc r1, r0, #0x80000000");
  // c.exec(instr);
  // c.printRegisters();
  // return 0; 

 
  
  // Decoder::test();

  Cpu c;
    
  Elf elf("./build/c.elf");
  Elf::ProgramHeaderIterator iter(elf.file, elf.header);

  c.mem.loadElf(iter);

  c.pcReal(elf.header.e_entry);

  while(true) {
    //printf("pc: %u sp: %u, r0: %x\n", c.pcReal(), c.r(13), c.r(0));
    u32 word = c.mem.a32u(c.pcReal());
    //fflush(stdout);
    u32 pc = c.exec(word);
    c.pcReal(pc);
    //fgetc(stdin);
  }
}
#else

int main() {
  Memory<64>::test();
  Decoder::test();
  Arith::test();
  return 0;
}

#endif
