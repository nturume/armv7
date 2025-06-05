
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
  strImm,
  strReg,
  strt1,
  strt2, // B==0
  ldrImm,
  ldrLit,
  ldrReg,
  ldrt1,
  ldrt2,//B==0
  strbImm,
  strbReg,
  strbt1,
  strbt2,//B==0
  ldrbImm,
  ldrbLit,
  ldrbReg,
  ldrbt1,
  ldrbt2,//B==0
  stmda,
  ldmda,
  stm,
  ldm,
  pop,
  stmdb,
  push,
  ldmdb,
  stmib,
  ldmib,
  ldmUser,
  stmUser,
  ldmExRet,
  b,
};

namespace Decoder {
Instr decodeA(u32 instr);
inline int printInstr(Instr instr) {
  switch (instr) {
  case Instr::b:
    return printf("Instr::b\n");
  case Instr::ldmExRet:
    return printf("Instr::ldmExRet\n");
  case Instr::stmUser:
    return printf("Instr::stmUser\n");
  case Instr::ldmUser:
    return printf("Instr::ldmUser\n");
  case Instr::ldmib:
    return printf("Instr::ldmib\n");
  case Instr::stmib:
    return printf("Instr::stmib\n");
  case Instr::ldmdb:
    return printf("Instr::ldmdb\n");
  case Instr::push:
    return printf("Instr::push\n");
  case Instr::stmdb:
    return printf("Instr::stmdb\n");
  case Instr::pop:
    return printf("Instr::pop\n");
  case Instr::ldm:
    return printf("Instr::ldm\n");
  case Instr::stm:
    return printf("Instr::stm\n");
  case Instr::ldmda:
    return printf("Instr::ldmda\n");
  case Instr::stmda:
    return printf("Instr::stmda\n");
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
  case Instr::strImm:
    return printf("Instr::strImm\n");
  case Instr::strReg:
    return printf("Instr::strReg\n");
  case Instr::strt1:
    return printf("Instr::strt1\n");
  case Instr::strt2:
    return printf("Instr::strt2\n");
  case Instr::ldrImm:
    return printf("Instr::ldrImm2\n");
  case Instr::ldrLit:
    return printf("Instr::ldrLit\n");
  case Instr::ldrReg:
    return printf("Instr::ldrReg\n");
  case Instr::ldrt1:
    return printf("Instr::ldrt1\n");
  case Instr::ldrt2:
    return printf("Instr::ldrt2\n");
  case Instr::strbImm:
    return printf("Instr::strbImm\n");
  case Instr::strbReg:
    return printf("Instr::strbReg\n");
  case Instr::strbt1:
    return printf("Instr::strbt1\n");
  case Instr::strbt2:
    return printf("Instr::strbt2\n");
  case Instr::ldrbImm:
    return printf("Instr::ldrbImm\n");
  case Instr::ldrbLit:
    return printf("Instr::ldrbLit\n");
  case Instr::ldrbReg:
    return printf("Instr::ldrbReg\n");
  case Instr::ldrbt1:
    return printf("Instr::ldrbt1\n");
  case Instr::ldrbt2:
    return printf("Instr::ldrbt2\n");
  }

  return printf("Unknown instruction\n");
}
} // namespace Decoder
