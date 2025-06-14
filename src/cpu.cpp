#include "cpu.hpp"
#include "arith.hpp"
#include "bin.hpp"
#include "decoder.hpp"
#include "stuff.hpp"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>

u32 Cpu::exec(u32 word) {
  cur = word;
  Instr instr = Decoder::decodeA(word);
  switch (instr) {
  case Instr::adcImm:
    return adcImm();
  case Instr::adcReg:
    return adcReg();
  case Instr::adcShiftedReg:
    return adcShiftedReg();
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
  case Instr::adr:
    return adr();
  case Instr::andImm:
    return andImm();
  case Instr::andReg:
    return andReg();
  case Instr::andShiftedReg:
    return andShiftedReg();
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
  case Instr::movImm:
    return movImm();
  case Instr::movImm16:
    return movImm16();
  case Instr::movReg:
    return movReg();
  case Instr::movt:
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
  case Instr::mvnImm:
    return mvnImm();
  case Instr::mvnReg:
    return mvnReg();
  case Instr::mvnShiftedReg:
    return mvnShiftedReg();
  case Instr::mul:
    return mul();
  case Instr::nop:
    return nxt();
  case Instr::pkh:
    return pkh();
    /* case Instr::qadd:
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
       return qsub8(); */
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
    return udiv();
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
    return sxtb16();
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
  default:
    printf("unhandled instruction: ");
    Decoder::printInstr(instr);
    exit(1);
  }
}

u32 Cpu::x(const char *prog) { return exec(assemble(prog)); }

static void testadcImm();
static void testadcReg();
static void testadcShiftedReg();
static void testaddImm();
static void testadr();
static void testand();
static void testshift();
static void testmov();
static void testbits();
static void testmul();
static void testmrs();
static void testq();
static void testrbit();
static void testdiv();
static void testubfx();
static void testxtab();
static void testsmul();

void Cpu::test() {
  testsmul();
  // testxtab();
  //  testubfx();
  // testdiv();
  // testrbit();
  // testq();
  //  testmrs();
  //   testmul();
  //  testbits();
  //    testmov();
  //    testshift();
  //    testand();
  //    testadr();
  //    testaddImm();
  //     testadcShiftedReg();
  //     testadcReg();
  //     testadcImm();
}

static void testsmul() {
  Cpu c;
  c.r(0, 0x12345678);
  c.x("smlabt r1, r0, r0, r3");
  assert(c.r(1) == 0x6260060);
  c.r(0, 0xffffffff);
  c.x("smlabt r1, r0, r0, r3");
  assert(c.r(1) == 1);

  c.r(3, 0x12345678);
  c.x("smlad r1, r0, r0, r3");
  assert(c.r(1) == 0x1234567a);
  c.r(0, 0x12345678);
  c.x("smladx r1, r0, r0, r3");
  assert(c.r(1) == 0x1e805738);

  c.r(0, 0x12345678);
  c.r(1, 0x87654321);
  c.x("smlal r1, r0, r0, r0");
  assert(c.r(0) == 0x137fbd54);
  assert(c.r(1) == 0xa55a1b61);

  c.r(0, 0x12345678);
  c.r(1, 0x87654321);
  c.x("smlalbt r1, r0, r0, r0");
  assert(c.r(0) == 0x12345678);
  printf("r1: %x\n", c.r(1));
  assert(c.r(1) == 0x8d8b4381);

  c.r(0, 0x12345678);
  c.r(1, 0x87654321);
  c.x("smlalbb r1, r0, r0, r0");
  assert(c.r(0) == 0x12345678);
  printf("r1: %x\n", c.r(1));
  assert(c.r(1) == 0xa49a1b61);

  c.r(0, 0xffffffff);
  c.r(1, 0xffffffff);
  c.x("smlalbt r1, r0, r0, r0");
  assert(c.r(0) == 0);
  printf("r1: %x\n", c.r(1));
  assert(c.r(1) == 0);

  c.r(3, 0x12345678);
  c.r(0, 0xff00ff00);
  c.r(1, 0xff00ff);
  c.x("smlald r1, r0, r0, r3");
  assert(c.r(1) == 0x9654ff);

  c.r(0, 0x12345678);
  c.r(1, 0x87654321);
  c.r(3, 0x87654321);
  c.x("smlaldx r1, r0, r0, r3");
  assert(c.r(1) == 0x636e9d2d);
}

static void testxtab() {
  Cpu c;
  c.r(0, 0xff);

  c.x("sxtab r1, r2, r0");
  assert(c.r(1) == 0xffffffff);
  c.r(0, 0x12345678);
  c.x("sxtab16 r1, r2, r0");
  assert(c.r(1) == 0x340078);
  c.r(0, 0x12348678);
  c.x("sxtah r1, r2, r0");
  assert(c.r(1) == 0xffff8678);
  c.r(0, 0x800080);
  c.x("sxtb16 r1, r0");
  assert(c.r(1) == 0xff80ff80);
  c.r(0, 0x12345678);
  c.x("uxtab r1, r0, r0");
  assert(c.r(1) == 0x123456f0);
  c.x("uxtab16 r1, r0, r0");
  assert(c.r(1) == 0x126856f0);
  c.x("uxtah r1, r0, r0");
  assert(c.r(1) == 0x1234acf0);
  c.x("uxtb r1, r0");
  assert(c.r(1) == 0x78);
  c.x("uxtb16 r1, r0");
  assert(c.r(1) == 0x340078);
  c.x("uxth r1, r0");
  assert(c.r(1) == 0x5678);
}

static void testubfx() {
  Cpu c;
  c.r(0, 0x12345678);
  c.x("ubfx r0, r0, #9, #3");
  assert(c.r(0) == 3);
}

static void testdiv() {
  Cpu c;
  c.r(0, 8);
  c.r(1, 2);
  c.x("sdiv r0, r0, r1");
  printf("r0: ===> %d\n", c.r(0));
  assert(c.r(0) == 4);

  c.r(0, 0x80000000);
  c.r(1, 0xffffffff);
  c.x("sdiv r0, r0, r1");
  assert(c.r(0) == 0x80000000);
}

static void testrbit() {
  Cpu c;
  c.r(0, 0x8000000f);
  c.x("rbit r0, r0");
  assert(c.r(0) == 0xf0000001);

  c.r(0, 0xffff0000);
  c.x("rev r0, r0");
  assert(c.r(0) == 0xffff);

  c.r(0, 0xffeeddbb);
  c.x("rev16 r0, r0");
  assert(c.r(0) == 0xeeffbbdd);

  c.r(0, 0xddff);
  c.x("revsh r0, r0");
  assert(c.r(0) == 0xffffffdd);

  c.r(0, 0b1);
  c.x("sbfx r1, r0, #0, #1");

  std::printf("----------====>>: %b\n", c.r(1));
  assert(c.r(1) == 0xffffffff);
  c.r(0, 0x80000000);
  c.x("sbfx r1 ,r0, #31, #1");
  std::printf("----------====>>: %b\n", c.r(1));
  assert(c.r(1) == 0xffffffff);
}

static void testq() {
  Cpu c;

  c.r(1, 0x12345678);
  c.r(2, 0x87654321);

  return;

  c.x("qsub16 r0, r1, r2");
  assert(c.r(0) == 0x7fff1357);

  c.x("qsub8 r0, r1, r2");
  assert(c.r(0) == 0x7fcf1357);

  c.r(0, 0x12345678);
  c.x("qsax r0, r0, r0");
  assert(c.r(0) == 0xbbbc68ac);

  c.r(0, 0xffffffff);
  c.x("qdsub r0, r0, r0");
  assert(c.r(0) == 1);

  c.r(0, 0xffffffff);
  c.x("qdadd r0, r0, r0");
  assert(c.r(0) == 0xfffffffd);

  c.r(0, 0xffffffff);
  c.x("qadd r0,  r0, r0");

  c.r(0, 0xffffffff);
  c.x("qadd16 r0, r0, r0");
  assert(c.r(0) == 0xfffefffe);

  c.r(0, 0x88888888);
  c.x("qadd16 r0, r0, r0");
  assert(c.r(0) == 0x80008000);

  c.r(0, 0xdddddddd);
  c.x("qadd16 r0, r0, r0");
  assert(c.r(0) == 0xbbbabbba);

  c.r(0, 0xffffdddd);
  c.x("qadd16 r0, r0, r0");
  assert(c.r(0) == 0xfffebbba);

  c.r(0, 0xbbbbbbbb);
  c.x("qadd16 r0, r0, r0");
  assert(c.r(0) == 0x80008000);

  c.r(0, 0xddddddd);
  c.x("qadd16 r0, r0, r0");
  assert(c.r(0) == 0x1bbabbba);

  c.r(0, 0xffffffff);
  c.x("qadd8 r0, r0, r0");
  assert(c.r(0) == 0xfefefefe);

  c.r(0, 0xffffffff);
  c.x("qasx r0, r0, r0");
  assert(c.r(0) == 0xfffe0000);

  c.r(0, 0x12345678);
  c.x("qasx r0, r0, r0");
  assert(c.r(0) == 0x68ac4444);
}

static void testmrs() {
  Cpu c;
  c.z(1);
  c.n(1);
  c.v(1);
  c.c(1);
  c.q(1);
  c.x("mrs r0, apsr");
  assert(c.r(0) == 0xf8000000);
  c.cf();
  assert(c.apsr.back == 0);
  c.x("msr apsr_nzcvq, #0xf8000000");
  assert(c.apsr.back == 0xf8000000);
  c.cf();
  c.r(0, 0xf800f000);
  assert(c.apsr.back == 0);
  c.x("msr apsr_nzcvq, r0");
  assert(c.apsr.back == 0xf8000000);
}

static void testmul() {
  Cpu c;
  c.r(0, 40);
  c.r(1, 40);
  c.r(2, 40);
  c.x("mla r3, r1, r2, r0");
  assert(c.r(3) == 40 * 40 + 40);
  c.r(2, 2000);
  c.x("mls r3, r0, r1, r2");
  c.printRegisters();
  assert(c.r(3) == 2000 - 40 * 40);
}

static void testbits() {
  Cpu c;
  c.r(0, 0b1111);
  c.x("bfc r0, #0, #32");
  assert(c.r(0) == 0);
  c.r(0, 0xffff);
  c.x("bfc r0, #8, #4");
  assert(c.r(0) == 0xf0ff);
  c.x("bfc r0, #0, #4");
  assert(c.r(0) == 0xf0f0);
  c.r(0, NEG);
  c.x("bfc r0, #31, #1");
  assert(c.r(0) == 0);

  c.r(0, 0x30);
  c.r(1, 0xf);
  c.x("bfi r0, r1, #0, #4");
  assert(c.r(0) == 0x3f);
  c.x("bfi r0, r1, #28, #4");
  assert(c.r(0) == 0xf000003f);

  c.x("bfi r2, r1, #31, #1");
  assert(c.r(2) == NEG);

  c.x("bfi r3, r1, #0, #1");
  assert(c.r(3) == 1);

  c.x("bfc r1, #1, #1");
  assert(c.r(1) == 0b1101);

  c.x("clz r0, r5");
  assert(c.r(0) == 32);

  c.r(0, 0b11);
  c.x("clz r0, r0");
  assert(c.r(0) == 30);

  c.r(0, 0xddddaaaa);
  c.r(1, 0xccccbbbb);

  c.x("pkhbt r2, r0, r1");
  assert(c.r(2) == 0xccccaaaa);

  c.x("pkhtb r2, r0, r1");
  assert(c.r(2) == 0xddddbbbb);
}

static void testmov() {
  Cpu c;
  c.x("mov r0, #0xf0000000");
  assert(c.r(0) == 0xf0000000);
  c.x("movw r0, #0xffff");
  assert(c.r(0) == 0xffff);
  c.x("mov r1, r1");
  assert(!c.z());
  c.x("movs r0,r1");
  assert(c.z());
  c.x("mov r15, #4");
  assert(c.pc() == 12);

  c.r(0, 0x8888);
  c.x("movt r0, #0xffff");
  assert(c.r(0) >> 16 == 0xffff);
  assert((c.r(0) & 0xffff) == 0x8888);
}

static void testshift() {
  Cpu c;
  c.r(1, NEG);
  c.x("asr r0, r1, #3");
  assert(c.r(0) == 0xf0000000);

  c.r(0, NEG);
  c.r(1, 3);
  c.x("asr r0, r0, r1");
  assert(c.r(0) == 0xf0000000);

  c.r(0, 1);
  c.c(true);
  c.x("rrx r0, r0");
  assert(c.r(0) == NEG);
}

static void testand() {
  Cpu c;
  c.x("ands r0, #0");
  assert(c.z());
  c.x("add r0, r0,#0xff000000");
  c.x("ands r1, r0, #0xff000000");
  assert(!c.z());
  assert(c.r(1) == 0xff000000);

  c.r(0, 1);
  c.r(1, 0xffffffff);
  c.x("and r1, r1 , r0, lsl r0");
  assert(c.r(1) == 2);

  c.r(0, 0xf);
  c.r(1, 0xf);
  c.x("and r0, r0, r1, lsl #1");
  assert(c.r(0) == 0b1110);

  c.r(0, 0xffff);
  c.x("bic r0, r0,#0xff");
  assert(c.r(0) == 0xff00);
}

static void testadr() {
  Cpu c;
  c.r(15, 4);
  c.x("adr r0, .");
  assert(c.r(0) == 4);
  c.r(15, 0);
  c.r(0, 0);
  c.x("sub r0, pc, #2");
  assert(c.r(0) == 6);
  c.r(15, 0);
  c.r(0, 0);
  c.x("add r0, pc, #2");
  assert(c.r(0) == 10);
}

static void testadcShiftedReg() {
  Cpu c;
  c.r(0, 1);
  c.r(1, 4);
  c.c(true);
  c.x("adc r1, r1, r1, lsl r0");
  assert(c.r(1) == 8 + 4 + 1);
}

static void testaddImm() {
  Cpu c;
  c.x("adds r0, r0, #0");
  assert(c.z());
  c.cf();
  c.x("add r0, r0, #1");
  assert(c.r(0) == 1);
  c.x("adds r1, r1, #0x80000000");
  assert(c.n());
  assert(c.r(1) == 0x80000000);
  c.c(true);
  c.x("adds r2, r2, #0");
  assert(c.r(2) == 0);
  assert(!c.c());

  c.cf();
  c.r(0, 0);
  c.x("add r15, r0, #4");
  assert(c.r(15) == 4);

  c.cf();
  c.r(0, 0);
  c.r(1, 0);
  c.x("subs r0, r0, #1");
  assert(c.r(0) == 0xffffffff);
}

static void testadcReg() {
  Cpu c;
  c.r(0, 5);
  c.r(1, 5);
  c.x("adc r2, r0, r1");
  assert(c.r(2) == 10);
  c.r(2, 1);
  c.x("adc r0, r2, r2, lsl #2");
  assert(c.r(0) == (0b100 + 1));

  c.r(2, 1);
  c.x("adc r0, r2, r2, lsl #31");
  assert(c.r(0) == (0x80000000 + 1));

  c.r(2, 1);
  c.x("adc r0, r2, r2, lsr #1");
  assert(c.r(0) == (0b1));

  c.r(2, 1);
  c.x("adc r0, r2, r2, ror #1");
  assert(c.r(0) == (0x80000000 + 1));

  c.r(2, 0x80000000);
  c.x("adc r0, r4, r2, asr #3");
  assert(c.r(0) == (0xf0000000));

  c.r(2, 0b11);
  c.cf();
  c.x("adc r0, r5, r2, ror #0");
  // TODO maybe toolchain bug
  // assert(c.r(0)==(0b1));
}

static void testadcImm() {
  Cpu c;
  c.x("adcs r0, r0, #0");
  assert(c.z());
  c.cf();
  c.x("adc r0, r0, #1");
  assert(c.r(0) == 1);
  c.x("adcs r1, r1, #0x80000000");
  assert(c.n());
  assert(c.r(1) == 0x80000000);
  c.c(true);
  c.x("adcs r2, r2, #0");
  assert(c.r(2) == 1);
  assert(!c.c());

  assert(Cpu::bm(2) == 0b11);
  assert(Cpu::bm(1) == 1);
  assert(Cpu::bm(32) == 0xffffffff);

  c.cf();
  c.r(0, 0);
  c.x("adc r15, r0, #4");
  assert(c.r(15) == 4);
}
