#include "cpu.hpp"
#include "bin.hpp"
#include "decoder.hpp"
#include "stuff.hpp"
#include "test.hpp"
#include "uart.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void Cpu::test() { tester(); }

u32 Cpu::exec(u32 word) {
  cur = word;
  // printf("%x ", word);
  Instr instr = Decoder::decode(word, currInstrSet());
  // Decoder::printInstr(instr);
  switch (instr) {
  case Instr::adcImm:
    return adcImm();
  case Instr::adcReg:
    return adcReg();
  case Instr::adcShiftedReg:
    return adcShiftedReg();
  case Instr::sbcImm:
    return sbcImm();
  case Instr::sbcReg:
    return sbcReg();
  case Instr::sbcShiftedReg:
    return sbcShiftedReg();
  case Instr::addImm:
    return addImm();
  case Instr::addReg:
    return addReg();
  case Instr::addShiftedReg:
    return addShiftedReg();
  case Instr::subImm:
    return subImm();
  case Instr::subReg:
    return subReg();
  case Instr::subShiftedReg:
    return subShiftedReg();
  case Instr::cmnImm:
    return cmnImm();
  case Instr::cmnReg:
    return cmnReg();
  case Instr::cmnShiftedReg:
    return cmnShiftedReg();
  case Instr::cmpImm:
    return cmpImm();
  case Instr::cmpReg:
    return cmpReg();
  case Instr::cmpShiftedReg:
    return cmpShiftedReg();
  case Instr::adr: // not
    return adr();
  case Instr::andImm:
    return andImm();
  case Instr::andReg:
    return andReg();
  case Instr::andShiftedReg:
    return andShiftedReg();
  case Instr::orrReg:
    return orrReg();
  case Instr::orrImm:
    return orrImm();
  case Instr::lsrReg:
    return lsrReg();
  case Instr::orrShiftedReg:
    return orrShiftedReg();
  case Instr::eorImm:
    return eorImm();
  case Instr::eorReg:
    return eorReg();
  case Instr::eorShiftedReg:
    return eorShiftedReg();
  case Instr::teqImm:
    return teqImm();
  case Instr::teqReg:
    return teqReg();
  case Instr::teqShiftedReg:
    return teqShiftedReg();
  case Instr::tstImm:
    return tstImm();
  case Instr::tstReg:
    return tstReg();
  case Instr::tstShiftedReg:
    return tstShiftedReg();
  case Instr::mvnImm: // not
    return mvnImm();
  case Instr::mvnReg:
    return mvnReg();
  case Instr::mvnShiftedReg:
    return mvnShiftedReg();
  case Instr::asrImm:
    return asrImm();
  case Instr::asrReg:
    return asrReg();
  case Instr::lslImm:
    return lslImm();
  case Instr::lslReg:
    return lslReg();
  case Instr::lsrImm:
    return lsrImm();
  case Instr::rrx:
    return rrx();
  case Instr::rorImm:
    return rorImm();
  case Instr::rorReg:
    return rorReg();
  case Instr::rsbImm:
    return rsbImm();
  case Instr::rsbReg:
    return rsbReg();
  case Instr::rsbShiftedReg:
    return rsbShiftedReg();
  case Instr::rscImm:
    return rscImm();
  case Instr::rscReg:
    return rscReg();
  case Instr::rscShiftedReg:
    return rscShiftedReg();
  case Instr::movImm: // not
    return movImm();
  case Instr::movImm16:
    return movImm16();
  case Instr::movReg:
    return movReg();
  case Instr::movt: // not
    return movt();
  case Instr::bicImm:
    return bicImm();
  case Instr::bicReg:
    return bicReg();
  case Instr::bicShiftedReg:
    return bicShiftedReg();
  case Instr::bfc:
    return bfc();
  case Instr::bfi:
    return bfi();
  case Instr::clz:
    return clz();
  case Instr::mla:
    return mla();
  case Instr::mls:
    return mls();
  case Instr::mrs:
    return mrs();
  case Instr::msrImmApp:
    return msrImmApp();
  case Instr::msrApp:
    return msrApp();
  case Instr::mul:
    return mul();
  case Instr::nop:
    return nxt();
  case Instr::pkh:
    return pkh();
  case Instr::qadd: //==
    return qadd();
  case Instr::qadd16:
    return qadd16();
  case Instr::qadd8:
    return qadd8();
  case Instr::qasx:
    return qasx();
  case Instr::qdadd:
    return qdadd();
  case Instr::qdsub:
    return qdsub();
  case Instr::qsax:
    return qsax();
  case Instr::qsub:
    return qsub();
  case Instr::qsub16:
    return qsub16();
  case Instr::qsub8:
    return qsub8(); //===
  case Instr::rbit:
    return rbit();
  case Instr::rev:
    return rev();
  case Instr::rev16:
    return rev16();
  case Instr::revsh:
    return revsh();
  case Instr::sbfx:
    return sbfx();
  case Instr::sdiv:
    return sdiv();
  case Instr::ubfx:
    return ubfx();
  case Instr::udiv:
    return udiv(); //=======not
  case Instr::umaal:
    return umaal();
  case Instr::umlal:
    return umlal();
  case Instr::umull:
    return umull();
  case Instr::uxtab:
    return uxtab();
  case Instr::uxtab16:
    return uxtab16();
  case Instr::uxtah:
    return uxtah();
  case Instr::uxtb:
    return uxtb();
  case Instr::uxtb16:
    return uxtb16();
  case Instr::uxth:
    return uxth();
  case Instr::sxtab:
    return sxtab();
  case Instr::sxtab16:
    return sxtab16();
  case Instr::sxtah:
    return sxtah();
  case Instr::sxth:
    return sxth();
  case Instr::sxtb:
    return sxtb();
  case Instr::sxtb16:
    return sxtb16(); // T
  case Instr::smlabb:
    return smlabb();
  case Instr::smlad:
    return smlad();
  case Instr::smlal:
    return smlal();
  case Instr::smlalbb:
    return smlalbb();
  case Instr::smlald:
    return smlald();
  case Instr::smlawb:
    return smlawb();
  case Instr::smlsd:
    return smlsd();
  case Instr::smlsld:
    return smlsld();
  case Instr::smmla:
    return smmla();
  case Instr::smmls:
    return smmls();
  case Instr::smmul:
    return smmul();
  case Instr::smuad:
    return smuad();
  case Instr::smulbb:
    return smulbb();
  case Instr::smull:
    return smull();
  case Instr::smulwb:
    return smulwb();
  case Instr::smusd:
    return smusd();
  case Instr::ssat:
    return ssat();
  case Instr::ssat16:
    return ssat16();
  case Instr::usat:
    return usat();
  case Instr::usat16:
    return usat16();
  case Instr::sadd16:
    return sadd16();
  case Instr::sadd8:
    return sadd8();
  case Instr::sasx:
    return sasx();
  case Instr::sel:
    return sel();
  case Instr::shadd16:
    return shadd16();
  case Instr::shadd8:
    return shadd8();
  case Instr::shasx:
    return shasx();
  case Instr::ssax:
    return ssax();
  case Instr::shsax:
    return shsax();
  case Instr::ssub16:
    return ssub16();
  case Instr::ssub8:
    return ssub8();
  case Instr::shsub16:
    return shsub16();
  case Instr::shsub8:
    return shsub8();
  case Instr::usax:
    return usax();
  case Instr::usub16:
    return usub16();
  case Instr::usub8:
    return usub8();
  case Instr::uadd16:
    return uadd16();
  case Instr::uadd8:
    return uadd8();
  case Instr::uasx:
    return uasx();
  case Instr::uhadd16:
    return uhadd16();
  case Instr::uhadd8:
    return uhadd8();
  case Instr::uhasx:
    return uhasx();
  case Instr::uhsax:
    return uhsax();
  case Instr::uhsub16:
    return uhsub16();
  case Instr::uhsub8:
    return uhsub8();
  case Instr::usad8:
    return usad8();
  case Instr::usada8:
    return usada8();
  case Instr::uqadd16:
    return uqadd16();
  case Instr::uqadd8:
    return uqadd8();
  case Instr::uqasx:
    return uqasx();
  case Instr::uqsax:
    return uqsax();
  case Instr::uqsub16:
    return uqsub16();
  case Instr::uqsub8:
    return uqsub8();
  //====== mem ========
  case Instr::strReg:
    return strReg();
  case Instr::ldrReg:
    return ldrReg();
  case Instr::strImm:
    return strImm();
  case Instr::ldrImm:
    return ldrImm();
  case Instr::ldrLit:
    return ldrLit();
  case Instr::ldrbImm:
    return ldrbImm();
  case Instr::strbImm:
    return strbImm();
  case Instr::strbReg:
    return strbReg();
  case Instr::ldrbReg:
    return ldrbReg();
  case Instr::ldrbLit:
    return ldrbLit();
  case Instr::ldrhImm:
    return ldrhImm();
  case Instr::ldrhLit:
    return ldrhLit();
  case Instr::ldrhReg:
    return ldrhReg();
  case Instr::ldrshImm:
    return ldrshImm();
  case Instr::ldrshLit:
    return ldrshLit();
  case Instr::ldrsh:
    return ldrshReg();
  case Instr::ldrsbImm:
    return ldrsbImm();
  case Instr::ldrsbLit:
    return ldrsbLit();
  case Instr::ldrsbReg:
    return ldrsbReg();
  case Instr::ldrdImm:
    return ldrdImm();
  case Instr::ldrdLit:
    return ldrdLit();
  case Instr::ldrdReg:
    return ldrdReg();
  case Instr::ldrex:
    return ldrex();
  case Instr::ldrexb:
    return ldrexb();
  case Instr::ldrexh:
    return ldrexh();
  case Instr::ldrexd:
    return ldrexd();
  case Instr::ldm:
    return ldm();
  case Instr::ldmda:
    return ldmda();
  case Instr::ldmdb:
    return ldmdb();
  case Instr::ldmib:
    return ldmib();
  case Instr::strhImm:
    return strhImm();
  case Instr::strhReg:
    return strhReg();
  case Instr::strdImm:
    return strdImm();
  case Instr::strd:
    return strdReg();
  case Instr::strex:
    return strex();
  case Instr::strexb:
    return strexb();
  case Instr::strexh:
    return strexh();
  case Instr::strexd:
    return strexd();
  case Instr::stm:
    return stm();
  case Instr::stmda:
    return stmda();
  case Instr::stmdb:
    return stmdb();
  case Instr::stmib:
    return stmib();
  case Instr::pop:
    return pop();
  case Instr::push:
    return push();
  //=== branch ====
  case Instr::b:
    return b();
  case Instr::bl:
    return bl();
  case Instr::bx:
    return bx();
  case Instr::blxReg:
    return blxReg();
  case Instr::undefined:
    return takeUndefInstrException();
  case Instr::svc:
    return svc();
  case Instr::msrSys:
    return msrSys();
  case Instr::srs:
    return srs();
  case Instr::rfe:
    return rfe();
  case Instr::msrImmSys:
    return msrImmSys();
  case Instr::cps:
    return cps();
  case Instr::mcr1:
  case Instr::mcr2:
    return mcr();
  case Instr::mrc1:
  case Instr::mrc2:
    return mrc();
  case Instr::dmb:
  case Instr::isb:
  case Instr::dsb:
  case Instr::pldImm:
  case Instr::pldLit:
  case Instr::pldReg:
    return nxt();
  case Instr::wfi:
    leavedisas();
    return nxt();
  case Instr::wfe:
    enterdisas();
    return nxt();
  case Instr::yield:
    return nxt();
  default:
    printf("unhandled instruction at 0x%x ", pcReal());
    Decoder::printInstr(instr);
    abort();
  }
}

u32 Cpu::x(const char *prog) { return exec(assemble(prog)); }

#include<unistd.h>
#include<sys/uio.h>
#include<sys/resource.h>
#include<sys/stat.h>

static void brk(Cpu *c) {
  u32 brk_addr = c->r(0);
  // printf("BRK: %x\n", brk_addr);
  if (brk_addr == 0) {
    c->r(0, c->mem.program_end);
  } else {
    u32 increment = brk_addr - c->mem.program_end;
    // printf("Increment: %x\n", increment);
    if(increment>1024*1024) {
      return c->r(0, ~0);
    }
    c->r(0, 0);
    //assert(false);
  }
}

static void setTls(Cpu *c) {
  // c->printRegisters();
  c->user_tls = c->r(0);
  c->r(0, 0);
}

static void write(Cpu *c) {
  // printf("WRITE => pc: %x\n", c->pcReal());
  // fgetc(stdin);
  c->r(0, write(c->r(0), c->mem.sysPtr(c->r(1)), c->r(2)));
}

static void writeV(Cpu *c) {
  c->r(0, writev(c->r(0),(const struct iovec *) c->mem.sysPtr(c->r(1)), c->r(2)));
}

static void exitGroup(Cpu *c) {
  // printf("_exit(%d)\n", c->r(0));
  _exit(c->r(0));
}

static void setTid(Cpu *c) {
  // printf("TID: %x\n", c->r(0));
  return c->r(0, ~0);
}

static void setRobustList(Cpu *c) {
  // printf("Robust List: %x\n", c->r(0));
  return c->r(0, ~0);
}


static void sys398(Cpu *c) {
  // printf("Arg: %x\n", c->r(0));
  return c->r(0, ~0);
}



static void readLinkAt(Cpu *c) {
  // printf("Arg: %x\n", c->r(0));
  // printf("Arg2: %s\n", (char*)c->mem.sysPtr(c->r(1)));
  char *buf = (char*)c->mem.sysPtr(c->r(2));
  buf[0] = '/';
  buf[1] = 'x';
  // assert(false);
  return c->r(0, 2);
}

static void rtSigProcMask(Cpu *c) {
  // printf("Arg: %x\n", c->r(0));
  return c->r(0, ~0);
}

static void getID(Cpu *c) {
  // printf("get id Arg: %x\n", c->r(0));
  return c->r(0, 0);
}//0x408001c5

// 0x400fef50 0x400fef40


static void getPID(Cpu *c) {
  // printf("get pid Arg: %x\n", c->r(0));
  return c->r(0, 0);
}

static void getRandom(Cpu *c) {
  // printf("get random Arg: %x %x\n", c->r(0), c->r(1));
  u32 *buf = (u32*)c->mem.sysPtr(c->r(0));
  *buf = c->r(0);
  //assert(false);
  return c->r(0, c->r(1));
}

static void tgKill(Cpu *c) {
  // printf("tgkill Arg: %x\n", c->r(0));
  return c->r(0, 0);
}


static void rtSigaction(Cpu *c) {
  // printf("get pid Arg: %x\n", c->r(0));
  return c->r(0, 0);
}


static void mmap2(Cpu *c) {
  // printf("mmpa2 Arg: %x\n", c->r(0));
  // printf("mmpa2 Arg: %x\n", c->r(1));
  // printf("mmpa2 Arg: %x\n", c->r(2));
  // printf("mmpa2 Arg: %x\n", c->r(3));
  // printf("mmpa2 Arg: %x\n", c->r(4));
  // printf("mmpa2 Arg: %x\n", c->r(5));
  Ram *r = c->mem.allocRandom(c->r(1));
  // printf("mmpa2 alloc at: %x\n", r->start);
  // assert(false);
  return c->r(0, r->start);
}

static void mProtect(Cpu *c) {
  // printf("get pid Arg: %x\n", c->r(0));
  // assert(false);
  return c->r(0, 0);
}


static void getRLimit(Cpu *c) {
  // printf("get rlimit Arg: %x\n", c->r(1));
  if(c->r(0)==RLIMIT_STACK) {
    u32 *r = (u32 *)c->mem.sysPtr(c->r(1));
    r[0] = 1024*1024;
    r[1] = 1024*1024;
  // assert(false);
    return c->r(0, 0);
  }
  assert(false);
  return c->r(0, ~0);
}


static void statX(Cpu *c) {
  // printf("get pid Arg: %x\n", c->r(0));
  // assert(false);
  // printf("statx Arg: %x\n", c->r(0));
  // printf("statx Arg: %x\n", c->r(1));
  // printf("statx Arg: %x\n", c->r(2));
  // printf("statx Arg: %x\n", c->r(3));
  // printf("statx Arg: %x\n", c->r(4));
  // printf("statx Arg: %x\n", c->r(5));
  i32 res = statx(
                  s32(c->r(0)),
                  (const char*)c->mem.sysPtr(c->r(1)),
                  s32(c->r(3)),
                  s32(c->r(4)),
                  (struct statx *)c->mem.sysPtr(c->r(5))
                   );
  return c->r(0, uns32(res));
}

void Cpu::doSysCall() {
  // printf("syscall happened: %d\n", r(7));
  switch (r(7)) {
  case 4:
    return write(this);
  case 45:
    return brk(this);
  case 983045:
    return setTls(this);
  case 248:
    return exitGroup(this);
  case 256:
    return setTid(this);
  case 338:
    return setRobustList(this);
  case 398:
    return sys398(this);
  case 191:
    return getRLimit(this);
  case 332:
    return readLinkAt(this);
  case 146:
    return writeV(this);
  case 192:
    return mmap2(this);
  case 175:
    return rtSigProcMask(this);
  case 224:
    return getID(this);
  case 20:
    return getPID(this);
  case 268:
    return tgKill(this);
  case 174:
    return rtSigaction(this);
  case 384:
    return getRandom(this);
  case 125:
    return mProtect(this);
  case 397:
    return statX(this);
  default: {
    printf("Unimplemented syscall: %d\n", r(7));
    abort();
  }
  }
}
