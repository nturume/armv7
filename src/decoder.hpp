
#pragma once
#include "./stuff.hpp"

enum class Instr {
  src,
  rfe,
  blx,
  bl,
  stc2,
  ldc2Lit,
  ldc2Imm,
  mcrr2,
  mrrc2,
};

namespace Decoder {
Instr decodeA(u32 instr);
inline int printInstr(Instr instr) {
  switch (instr) {
  case Instr::src:
    return printf("Instr::src\n");
  case Instr::rfe:
    return printf("Instr::rfe\n");
  case Instr::bl:
    return printf("Instr::bl\n");
  case Instr::blx:
    return printf("Instr::blx\n");
  case Instr::stc2:
    return printf("Instr::stc2\n");
  case Instr::ldc2Imm:
    return printf("Instr::ldc2Imm\n");
  case Instr::ldc2Lit:
    return printf("Instr::ldcLit\n");
  case Instr::mcrr2:
    return printf("Instr::mcrr2\n");
  case Instr::mrrc2:
    return printf("Instr::mrrc2\n");
  }

  printf("Unknown instruction\n");
}
} // namespace Decoder
