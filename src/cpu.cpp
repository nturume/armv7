#include "cpu.hpp"
#include "stuff.hpp"
#include <string>

u32 Cpu::exec(u32 word) {
  cur = word;
  Instr instr = Decoder::decodeA(word); 
  switch(instr) {
    case Instr::adcImm:
      return adcImm();
  default:
    printf("unhandled instruction: ");
    Decoder::printInstr(instr);
    exit(1);
  }
}
