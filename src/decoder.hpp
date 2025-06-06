
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
  svc,
  stc1,
  ldc1Imm,
  ldc1Lit,
  mcrr1,
  mrrc1,
  cdp1,
  mcr1,
  mrc1,
  usada8,
  usad8,
  sbfx,
  bfc,
  bfi,
  ubfx,

  sadd16,
  sasx,
  ssax,
  ssub16,
  sadd8,
  ssub8,

  qadd16,
  qasx,
  qsax,
  qsub16,
  qadd8,
  qsub8,

  shadd16,
  shasx,
  shsax,
  shsub16,
  shadd8,
  shsub8,

  pkh,
  sxtab16,
  sxtb16,
  sel,
  ssat,
  ssat16,
  sxtab,
  sxtb,
  rev,
  sxtah,
  sxth,
  rev16,
  uxtab16,
  uxtb16,
  usat,
  usat16,
  uxtab,
  uxtb,
  rbit,
  uxtah,
  uxth,
  revsh, 
};

namespace Decoder {
Instr decodeA(u32 instr);
inline int printInstr(Instr instr) {
  switch (instr) {
  case Instr::revsh:
    return printf("Instr::revsh\n");
  case Instr::uxth:
    return printf("Instr::uxth\n");
  case Instr::uxtah:
    return printf("Instr::uxtah\n");
  case Instr::rbit:
    return printf("Instr::rbit\n");
  case Instr::uxtb:
    return printf("Instr::uxtb\n");
  case Instr::uxtab:
    return printf("Instr::uxtab\n");
  case Instr::usat16:
    return printf("Instr::usat16\n");
  case Instr::usat:
    return printf("Instr::usat\n");
  case Instr::uxtb16:
    return printf("Instr::uxtb16\n");
  case Instr::uxtab16:
    return printf("Instr::uxtab16\n");
  case Instr::rev16:
    return printf("Instr::rev16\n");
  case Instr::sxth:
    return printf("Instr::sxth\n");
  case Instr::sxtah:
    return printf("Instr::sxtah\n");
  case Instr::rev:
    return printf("Instr::rev\n");
  case Instr::sxtb:
    return printf("Instr::sxtb\n");
  case Instr::sxtab:
    return printf("Instr::sxtab\n");
  case Instr::ssat16:
    return printf("Instr::ssat16\n");
  case Instr::ssat:
    return printf("Instr::ssat\n");
  case Instr::sel:
    return printf("Instr::sel\n");
  case Instr::sxtb16:
    return printf("Instr::sxtb16\n");
  case Instr::sxtab16:
    return printf("Instr::sxtab16\n");
  case Instr::pkh:
    return printf("Instr::pkh\n");
  case Instr::qsub8:
    return printf("Instr::qsub8\n");
  case Instr::qadd8:
    return printf("Instr::qadd8\n");
  case Instr::qsub16:
    return printf("Instr::qsub16\n");
  case Instr::qsax:
    return printf("Instr::qsax\n");
  case Instr::qasx:
    return printf("Instr::qasx\n");
  case Instr::qadd16:
    return printf("Instr::qadd16\n");

  case Instr::shsub8:
    return printf("Instr::shsub8\n");
  case Instr::shadd8:
    return printf("Instr::shadd8\n");
  case Instr::shsub16:
    return printf("Instr::shsub16\n");
  case Instr::shsax:
    return printf("Instr::shsax\n");
  case Instr::shasx:
    return printf("Instr::shasx\n");
  case Instr::shadd16:
    return printf("Instr::shadd16\n");

  case Instr::ssub8:
    return printf("Instr::ssub8\n");
  case Instr::sadd8:
    return printf("Instr::sadd8\n");
  case Instr::ssub16:
    return printf("Instr::ssub16\n");
  case Instr::ssax:
    return printf("Instr::ssax\n");
  case Instr::sasx:
    return printf("Instr::sasx\n");
  case Instr::sadd16:
    return printf("Instr::sadd16\n");
  case Instr::ubfx:
    return printf("Instr::ubfx\n");
  case Instr::bfi:
    return printf("Instr::bfi\n");
  case Instr::bfc:
    return printf("instr::bfc\n");
  case Instr::sbfx:
    return printf("Instr::sbfx\n");
  case Instr::usad8:
    return printf("Instr::usad8\n");
  case Instr::usada8:
    return printf("Instr::usada8\n");
  case Instr::mcrr1:
    return printf("Instr::mcrr1\n");
  case Instr::mrrc1:
    return printf("Instr::mrrc1\n");
  case Instr::cdp1:
    return printf("Instr::cdp1\n");
  case Instr::mcr1:
    return printf("Instr::mcr1\n");
  case Instr::mrc1:
    return printf("Instr::mrc1\n");
    
  case Instr::ldc1Lit:
    return printf("Instr::ldc1Lit\n");
  case Instr::ldc1Imm:
    return printf("Instr::ldc1Imm\n");
  case Instr::stc1:
    return printf("Instr::stc1\n");
  case Instr::svc:
    return printf("Instr::svc\n");
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
