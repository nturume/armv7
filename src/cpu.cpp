#include "cpu.hpp"
#include "bin.hpp"
#include "decoder.hpp"
#include "stuff.hpp"
#include <atomic>
#include <cassert>
#include <cstdint>
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
  case Instr::qadd:
    return qadd();
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
void Cpu::test() {
  testq();
  //testmrs();
  // testmul();
  //testbits();
  //  testmov();
  //  testshift();
  //  testand();
  //  testadr();
  //  testaddImm();
  //   testadcShiftedReg();
  //   testadcReg();
  //   testadcImm();
}

static void testq() {
  Cpu c;
  c.r(0, INT32_MAX);
  c.x("qadd r0,  r0, r0");
  assert(c.q());
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
  assert(c.r(2)==0xccccaaaa);

  c.x("pkhtb r2, r0, r1");
  assert(c.r(2)==0xddddbbbb);
  
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
