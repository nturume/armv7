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
#include "uart.hpp"

#ifndef TESTING
int  main() {
  Cpu c = {};
    
  Elf elf("./build/c.elf");
  Elf::ProgramHeaderIterator iter(elf.file, elf.header);

  c.mem.loadElf(iter);

  c.mem.regions.push_back(UART::getRegion());

  //c.pcReal(elf.header.e_entry);
  printf("Header entry %x\n", elf.header.e_entry);
  c.reset();

  while(true) {
   // printf("pc: %u sp: %u, r0: %x\n", c.pcReal(), c.r(13), c.r(0));
    u32 word = c.mem.a32u(c.pcReal());
    Decoder::printInstr(Decoder::decodeA(word));
    try {
    u32 pc = c.exec(word);
    c.pcReal(pc);
    } catch(InvalidRegion& in) {
      printf("===invalid region===\n");
      c.pcReal(c.takeDataAbortException());
    }
    fgetc(stdin);
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
