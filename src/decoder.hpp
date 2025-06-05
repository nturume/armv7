
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
  cdp2,
  mcr2,
  mrc2,
  cps,
  setend,
  unpredictable,
  nop,
  pliImm,
  pldImm,
  pldLit,
  clrex,
  dsb,
  dmb,
  isb,
  pliReg,
  pldReg,
  undefined,
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
  case Instr::cdp2:
    return printf("Instr::cdp2\n");
  case Instr::mcr2:
    return printf("Instr::mcr2\n");
  case Instr::mrc2:
    return printf("Instr::mrc2\n");
  case Instr::cps:
    return printf("Instr::cps\n");
  case Instr::setend:
    return printf("Instr::setend\n");
  case Instr::unpredictable:
    return printf("Instr::unpredictable\n");
  case Instr::nop:
    return printf("Instr::nop\n");
  case Instr::pliImm:
    return printf("Instr::pliImm\n");
  case Instr::pldImm:
    return printf("Instr::pldImm\n");
  case Instr::pldLit:
    return printf("Instr::pldLit\n");
  case Instr::clrex:
    return printf("Instr::clrex\n");
  case Instr::dsb:
    return printf("Instr::dsb\n");
  case Instr::dmb:
    return printf("Instr::dmb\n");
  case Instr::isb:
    return printf("Instr::isb\n");
  case Instr::pliReg:
    return printf("Instr::pliReg\n");
  case Instr::pldReg:
    return printf("Instr::pldReg\n");
  case Instr::undefined:
    return printf("Instr::undefined\n");
  }

  return printf("Unknown instruction\n");
}
} // namespace Decoder
