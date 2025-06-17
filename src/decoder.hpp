
#pragma once
#include "./stuff.hpp"
#include <sys/types.h>

enum class Instr {
  srs,
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
  ldrt2, // B==0
  strbImm,
  strbReg,
  strbt1,
  strbt2, // B==0
  ldrbImm,
  ldrbLit,
  ldrbReg,
  ldrbt1,
  ldrbt2, // B==0
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
  
  uadd16,
  uasx,
  usax,
  usub16,
  uadd8,
  usub8,

  uqadd16,
  uqasx,
  uqsax,
  uqsub16,
  uqadd8,
  uqsub8,

  uhadd16,
  uhasx,
  uhsax,
  uhsub16,
  uhadd8,
  uhsub8,

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

  smlad,
  smuad,
  smlsd,
  smusd,
  sdiv,
  udiv,
  smlald,
  smlsld,
  smmla,
  smmul,
  smmls,

  andReg,
  eorReg,
  subReg,
  rsbReg,
  addReg,
  adcReg,
  sbcReg,
  rscReg,
  tstReg,
  teqReg,
  cmpReg,
  cmnReg,
  orrReg,
  movReg,
  lslImm,
  lsrImm,
  asrImm,
  rrx,
  rorImm,
  bicReg,
  mvnReg,

  andShiftedReg,
  eorShiftedReg,
  subShiftedReg,
  rsbShiftedReg,
  addShiftedReg,
  adcShiftedReg,
  sbcShiftedReg,
  rscShiftedReg,
  tstShiftedReg,
  teqShiftedReg,
  cmpShiftedReg,
  cmnShiftedReg,
  orrShiftedReg,
  lslReg,
  lsrReg,
  asrReg,
  rorReg,
  bicShiftedReg,
  mvnShiftedReg,

  mrsBanked,
  msrBanked,
  mrs,
  // msr,
  msrApp,
  msrSys,

  bx,
  clz,
  bxj,
  blxReg,
  eret,
  bkpt,
  hvc,
  smc,

  qadd,
  qsub,
  qdadd,
  qdsub,

  smlabb,
  smlawb,
  smulwb,
  smlalbb,
  smulbb,

  mul,
  mla,
  umaal,
  mls,
  umull,
  umlal,
  smull,
  smlal,

  swp,
  strex,
  ldrex,
  strexd,
  ldrexd,
  strexb,
  ldrexb,
  strexh,
  ldrexh,

  strhReg,
  ldrhReg,
  strhImm,
  ldrhImm,
  ldrhLit,
  ldrdReg,
  ldrsbReg,
  ldrdImm,
  ldrdLit,
  ldrsbImm,
  ldrsbLit,

  strht,
  strht2,
  ldrht,
  ldrsbt,
  ldrsht,

  andImm,
  eorImm,
  subImm,
  adr,
  rsbImm,
  addImm,
  adcImm,
  sbcImm,
  rscImm,
  tstImm,
  teqImm,
  cmpImm,
  cmnImm,
  orrImm,
  movImm,
  bicImm,
  mvnImm,

  movImm16,
  movt,

  strd,
  ldrsh,
  strdImm,
  ldrshImm,
  ldrshLit,

  yield,
  wfe,
  wfi,
  sev,
  csdb,
  dbg,
  msrImmApp,
  msrImmSys,
};

namespace Decoder {
void test();
Instr decodeA(u32 instr);
inline int printInstr(Instr instr) {
  switch (instr) {
  case Instr::strd:
    return printf("Instr::strd\n");
  case Instr::ldrsh:
    return printf("Instr::ldrsh\n");
  case Instr::strdImm:
    return printf("Instr::strdImm\n");
  case Instr::ldrshImm:
    return printf("Instr::ldrshImm\n");
  case Instr::ldrshLit:
    return printf("Instr::ldrshLit\n");
  case Instr::yield:
    return printf("Instr::yield\n");
  case Instr::wfe:
    return printf("Instr::wfe\n");
  case Instr::wfi:
    return printf("Instr::wfi\n");
  case Instr::sev:
    return printf("Instr::sev\n");
  case Instr::csdb:
    return printf("Instr::csdb\n");
  case Instr::dbg:
    return printf("Instr::dbg\n");
  case Instr::msrImmApp:
    return printf("Instr::msrImmApp\n");
  case Instr::movt:
    return printf("Instr::movt\n");
  case Instr::msrImmSys:
    return printf("Instr::msrImmSys\n");
  case Instr::movImm16:
    return printf("Instr::movImm16\n");
  case Instr::eorImm:
    return printf("Instr::eorImm\n");
  case Instr::subImm:
    return printf("Instr::subImm\n");
  case Instr::adr:
    return printf("Instr::adr\n");
  case Instr::rsbImm:
    return printf("Instr::rsbImm\n");
  case Instr::addImm:
    return printf("Instr::addImm\n");
  case Instr::adcImm:
    return printf("Instr::adcImm\n");
  case Instr::sbcImm:
    return printf("Instr::sbcImm\n");
  case Instr::rscImm:
    return printf("Instr::rscImm\n");
  case Instr::tstImm:
    return printf("Instr::tstImm\n");
  case Instr::teqImm:
    return printf("Instr::teqImm\n");
  case Instr::cmpImm:
    return printf("Instr::cmpImm\n");
  case Instr::cmnImm:
    return printf("Instr::cmnImm\n");
  case Instr::orrImm:
    return printf("Instr::orrImm\n");
  case Instr::movImm:
    return printf("Instr::movImm\n");
  case Instr::bicImm:
    return printf("Instr::bicImm\n");
  case Instr::mvnImm:
    return printf("Instr::mvnImm\n");
  case Instr::andImm:
    return printf("Instr::andImm\n");
  case Instr::strht:
    return printf("Instr::strht\n");
  case Instr::strht2:
    return printf("Instr::strht2\n");
  case Instr::ldrht:
    return printf("Instr::ldrht\n");
  case Instr::ldrsbt:
    return printf("Instr::ldrsbt\n");
  case Instr::ldrsht:
    return printf("Instr::ldrsht\n");
  case Instr::strhReg:
    return printf("Instr::strhReg\n");
  case Instr::ldrhReg:
    return printf("Instr::ldrhReg\n");
  case Instr::strhImm:
    return printf("Instr::strhImm\n");
  case Instr::ldrhImm:
    return printf("Instr::ldrhImm\n");
  case Instr::ldrhLit:
    return printf("Instr::ldrhLit\n");
  case Instr::ldrdReg:
    return printf("Instr::ldrdReg\n");
  case Instr::ldrsbReg:
    return printf("Instr::ldrsbReg\n");
  case Instr::ldrdImm:
    return printf("Instr::ldrdImm\n");
  case Instr::ldrdLit:
    return printf("Instr::ldrdLit\n");
  case Instr::ldrsbImm:
    return printf("Instr::ldrsbImm\n");
  case Instr::ldrsbLit:
    return printf("Instr::ldrsbLit\n");
  case Instr::swp:
    return printf("Instr::swp\n");
  case Instr::strex:
    return printf("Instr::strex\n");
  case Instr::ldrex:
    return printf("Instr::ldrex\n");
  case Instr::strexd:
    return printf("Instr::strexd\n");
  case Instr::ldrexd:
    return printf("Instr::ldrexd\n");
  case Instr::strexb:
    return printf("Instr::strexb\n");
  case Instr::ldrexb:
    return printf("Instr::ldrexb\n");
  case Instr::strexh:
    return printf("Instr::strexh\n");
  case Instr::ldrexh:
    return printf("Instr::ldrexh\n");
  case Instr::mul:
    return printf("Instr::mul\n");
  case Instr::mla:
    return printf("Instr::mla\n");
  case Instr::umaal:
    return printf("Instr::umaal\n");
  case Instr::mls:
    return printf("Instr::mls\n");
  case Instr::umull:
    return printf("Instr::umull\n");
  case Instr::umlal:
    return printf("Instr::umlal\n");
  case Instr::smull:
    return printf("Instr::smull\n");
  case Instr::smlal:
    return printf("Instr::smlal\n");
  case Instr::smlabb:
    return printf("Instr::smlabb\n");
  case Instr::smlawb:
    return printf("Instr::smlawb\n");
  case Instr::smulwb:
    return printf("Instr::smulwb\n");
  case Instr::smlalbb:
    return printf("Instr::smlalbb\n");
  case Instr::smulbb:
    return printf("Instr::smulbb\n");
  case Instr::qadd:
    return printf("Instr::qadd\n");
  case Instr::qsub:
    return printf("Instr::qsub\n");
  case Instr::qdadd:
    return printf("Instr::qdadd\n");
  case Instr::qdsub:
    return printf("Instr::qdsub\n");
  case Instr::bx:
    return printf("Instr::bx\n");
  case Instr::clz:
    return printf("Instr::clz\n");
  case Instr::bxj:
    return printf("Instr::bxj\n");
  case Instr::blxReg:
    return printf("Instr::blxReg\n");
  case Instr::eret:
    return printf("Instr::eret\n");
  case Instr::bkpt:
    return printf("Instr::bkpt\n");
  case Instr::hvc:
    return printf("Instr::hvc\n");
  case Instr::smc:
    return printf("Instr::smc\n");
  case Instr::mrsBanked:
    return printf("Instr::mrsBanked\n");
  case Instr::msrBanked:
    return printf("Instr::msrBanked\n");
  case Instr::mrs:
    return printf("Instr::mrs\n");
  // case Instr::msr:
  //   return printf("Instr::msr\n");
  case Instr::msrApp:
    return printf("Instr::msrApp\n");
  case Instr::msrSys:
    return printf("Instr::msrSys\n");
  case Instr::orrShiftedReg:
    return printf("Instr::orrShiftedReg\n");
  case Instr::lslReg:
    return printf("Instr::lslReg\n");
  case Instr::lsrReg:
    return printf("Instr::lsrReg\n");
  case Instr::asrReg:
    return printf("Instr::asrReg\n");
  case Instr::rorReg:
    return printf("Instr::rorReg\n");
  case Instr::bicShiftedReg:
    return printf("Instr::bicShiftedReg\n");
  case Instr::mvnShiftedReg:
    return printf("Instr::mvnShiftedReg\n");
  case Instr::eorShiftedReg:
    return printf("Instr::eorShiftedReg\n");
  case Instr::subShiftedReg:
    return printf("Instr::subShiftedReg\n");
  case Instr::rsbShiftedReg:
    return printf("Instr::rsbShiftedReg\n");
  case Instr::addShiftedReg:
    return printf("Instr::addShiftedReg\n");
  case Instr::adcShiftedReg:
    return printf("Instr::adcShiftedReg\n");
  case Instr::sbcShiftedReg:
    return printf("Instr::sbcShiftedReg\n");
  case Instr::rscShiftedReg:
    return printf("Instr::rscShiftedReg\n");
  case Instr::tstShiftedReg:
    return printf("Instr::tstShiftedReg\n");
  case Instr::teqShiftedReg:
    return printf("Instr::teqShiftedReg\n");
  case Instr::cmpShiftedReg:
    return printf("Instr::cmpShiftedReg\n");
  case Instr::cmnShiftedReg:
    return printf("Instr::cmnShiftedReg\n");
  case Instr::andShiftedReg:
    return printf("Instr::andShiftedReg\n");
  case Instr::asrImm:
    return printf("Instr::asrImm\n");
  case Instr::andReg:
    return printf("Instr::andReg\n");
  case Instr::eorReg:
    return printf("Instr::eorReg\n");
  case Instr::subReg:
    return printf("Instr::subReg\n");
  case Instr::rsbReg:
    return printf("Instr::rsbReg\n");
  case Instr::addReg:
    return printf("Instr::addReg\n");
  case Instr::adcReg:
    return printf("Instr::adcReg\n");
  case Instr::sbcReg:
    return printf("Instr::sbcReg\n");
  case Instr::rscReg:
    return printf("Instr::rscReg\n");
  case Instr::tstReg:
    return printf("Instr::tstReg\n");
  case Instr::teqReg:
    return printf("Instr::teqReg\n");
  case Instr::cmpReg:
    return printf("Instr::cmpReg\n");
  case Instr::cmnReg:
    return printf("Instr::cmnReg\n");
  case Instr::orrReg:
    return printf("Instr::orrReg\n");
  case Instr::movReg:
    return printf("Instr::movReg\n");
  case Instr::lslImm:
    return printf("Instr::lslImm\n");
  case Instr::lsrImm:
    return printf("Instr::lsrImm\n");
  case Instr::rrx:
    return printf("Instr::rrx\n");
  case Instr::rorImm:
    return printf("Instr::rorImm\n");
  case Instr::bicReg:
    return printf("Instr::bicReg\n");
  case Instr::mvnReg:
    return printf("Instr::mvnReg\n");
  case Instr::smlad:
    return printf("Instr::smlad\n");
  case Instr::smuad:
    return printf("Instr::smuad\n");
  case Instr::smlsd:
    return printf("Instr::smlsd\n");
  case Instr::smusd:
    return printf("Instr::smusd\n");
  case Instr::sdiv:
    return printf("Instr::sdiv\n");
  case Instr::smlald:
    return printf("Instr::smlald\n");
  case Instr::smlsld:
    return printf("Instr::smlsld\n");
  case Instr::smmla:
    return printf("Instr::smmla\n");
  case Instr::smmul:
    return printf("Instr::smmul\n");
  case Instr::smmls:
    return printf("Instr::smmls\n");
  case Instr::udiv:
    return printf("Instr::udiv\n");
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
  case Instr::srs:
    return printf("Instr::srs\n");
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
    return printf("Instr::ldrImm\n");
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
  case Instr::uadd16:
    return printf("Instr::uadd16\n");
  case Instr::uasx:
    return printf("Instr::uasx\n");
  case Instr::usax:
    return printf("Instr::usax\n");
  case Instr::usub16:
    return printf("Instr::usub16\n");
  case Instr::uadd8:
    return printf("Instr::uadd8\n");
  case Instr::usub8:
    return printf("Instr::usub8\n");
  case Instr::uqadd16:
    return printf("Instr::uqadd16\n");
  case Instr::uqasx:
    return printf("Instr::uqasx\n");
  case Instr::uqsax:
    return printf("Instr::uqsax\n");
  case Instr::uqsub16:
    return printf("Instr::uqsub16\n");
  case Instr::uqadd8:
    return printf("Instr::uqadd8\n");
  case Instr::uqsub8:
    return printf("Instr::uqsub8\n");
  case Instr::uhadd16:
    return printf("Instr::uhadd16\n");
  case Instr::uhasx:
    return printf("Instr::uhasx\n");
  case Instr::uhsax:
    return printf("Instr::uhsax\n");
  case Instr::uhsub16:
    return printf("Instr::uhsub16\n");
  case Instr::uhadd8:
    return printf("Instr::uhadd8\n");
  case Instr::uhsub8:
    return printf("Instr::uhsub8\n");
  }

  return printf("Unknown instruction\n");
}
} // namespace Decoder
