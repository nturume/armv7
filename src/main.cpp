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

  // UART::PL011 pl = {};
  // pl.enableTxfifo();
  // for(u8 i = 0; i < 40; i++) {
  //  pl.writeDR(i+97); 
  // }
  // pl.tx();
  // pl.printTxState();
  // return 0;

  Cpu c;
    
  Elf elf("./build/c.elf");
  Elf::ProgramHeaderIterator iter(elf.file, elf.header);

  c.mem.loadElf(iter);

  c.mem.regions.push_back(UART::getRegion());

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
