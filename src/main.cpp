#include "./bin.hpp"
#include "./decoder.hpp"
#include "./elf.hpp"
#include "./mem.hpp"
#include "./stuff.hpp"
#include "arith.hpp"
#include "cpu.hpp"
#include "uart.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef TESTING
int main() {
  Cpu c;

  Elf elf("./build/c.elf");
  Elf::ProgramHeaderIterator iter(elf.file, elf.header);

  c.mem.loadElf(iter);

  c.mem.regions.push_back(UART::getRegion());
  c.reset();
  // c.pcReal(0x60010000);
  printf("Header entry %x\n", elf.header.e_entry);
  // c.reset();

  while (true) {
    // printf("pc: %u sp: %u, r0: %x\n", c.pcReal(), c.r(13), c.r(0));
    //c.printMode(c.M());
    try {
      u32 word = c.mem.a32u(c.pcReal());
      //Decoder::printInstr(Decoder::decodeA(word));
      u32 pc = c.exec(word);
      c.pcReal(pc);
    } catch (InvalidRegion &in) {
      printf("===invalid region=== %x\n", in.address);
      c.pcReal(c.takeDataAbortException());
    }
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
