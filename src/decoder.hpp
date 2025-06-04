
#pragma once
#include "./stuff.hpp"

enum class Instr {
  src,
  rfe,
};

namespace  Decoder {  
  Instr decodeA(u32 instr);
  inline int printInstr(Instr instr) {
    switch(instr) {
      case Instr::src:
        return printf("Instr::src\n");
      case Instr::rfe:
        return printf("Instr::rfe\n");
    }

    printf("Unknown instruction\n");
  }
}

