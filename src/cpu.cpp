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
  case Instr::orrReg:
    return orrReg();
  case Instr::lsrReg:
    return lsrReg();
  case Instr::orrShiftedReg:
    return orrShiftedReg();
  case Instr::orrImm:
    return orrImm();
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
    // default:
    //  printf("unhandled instruction: ");
    //  Decoder::printInstr(instr);
    //  exit(1);
  }
}

u32 Cpu::x(const char *prog) { return exec(assemble(prog)); }

void Cpu::test() {
  Cpu c;c.cf();
c.r(0, 0x6b47b733);
c.r(1, 0x71228c49);
c.r(2, 0xa9e7e2c);
c.r(3, 0x1a20b8ea);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x51e32be6);
c.expectreg(1,0x71228c49);
c.expectreg(2,0xa9e7e2c);
c.expectreg(3,0x1a20b8ea);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x60fe343b);
c.r(1, 0x6ce93835);
c.r(2, 0x37535e4b);
c.r(3, 0x4bfc1b0b);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x7419a601);
c.expectreg(1,0x6ce93835);
c.expectreg(2,0x37535e4b);
c.expectreg(3,0x4bfc1b0b);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x42213435);
c.r(1, 0x19612ac8);
c.r(2, 0x4712340b);
c.r(3, 0x74024af4);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x792b1ed1);
c.expectreg(1,0x19612ac8);
c.expectreg(2,0x4712340b);
c.expectreg(3,0x74024af4);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x619d375b);
c.r(1, 0xd9c95da);
c.r(2, 0x17fd1a66);
c.r(3, 0x3f976ce5);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x40fec080);
c.expectreg(1,0xd9c95da);
c.expectreg(2,0x17fd1a66);
c.expectreg(3,0x3f976ce5);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x19159a6);
c.r(1, 0x61f009d3);
c.r(2, 0x41e5de7d);
c.r(3, 0x733198fc);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x665f89e2);
c.expectreg(1,0x61f009d3);
c.expectreg(2,0x41e5de7d);
c.expectreg(3,0x733198fc);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x779176ce);
c.r(1, 0x32ddb11a);
c.r(2, 0x49015172);
c.r(3, 0x2db0977e);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x3ddf6340);
c.expectreg(1,0x32ddb11a);
c.expectreg(2,0x49015172);
c.expectreg(3,0x2db0977e);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x1c9a6227);
c.r(1, 0x57ed847);
c.r(2, 0x53846b31);
c.r(3, 0x5a0fae0c);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x5c5cbfb9);
c.expectreg(1,0x57ed847);
c.expectreg(2,0x53846b31);
c.expectreg(3,0x5a0fae0c);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0xac84625);
c.r(1, 0x33a1dc27);
c.r(2, 0x70af2c7f);
c.r(3, 0x760ffd58);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x7f096c7a);
c.expectreg(1,0x33a1dc27);
c.expectreg(2,0x70af2c7f);
c.expectreg(3,0x760ffd58);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x24c46871);
c.r(1, 0x7b4daaac);
c.r(2, 0x1030b643);
c.r(3, 0x5c29cac);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0xe23e6ea9);
c.expectreg(1,0x7b4daaac);
c.expectreg(2,0x1030b643);
c.expectreg(3,0x5c29cac);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x6836e2e1);
c.r(1, 0x4784148e);
c.r(2, 0x51beb7b7);
c.r(3, 0x2a581717);
c.x("smlawb r0, r1, r2, r3");
c.expectreg(0,0x16268ca5);
c.expectreg(1,0x4784148e);
c.expectreg(2,0x51beb7b7);
c.expectreg(3,0x2a581717);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
return;
  c.cf();
c.r(0, 0x4dc18196);
c.r(1, 0x30b8c2d0);
c.r(2, 0x5981ca3e);
c.r(3, 0x4e8f7c7c);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x5f984550);
c.expectreg(1,0x30b8c2d0);
c.expectreg(2,0x5981ca3e);
c.expectreg(3,0x4e8f7c7c);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x6779f1bc);
c.r(1, 0x4e7448d2);
c.r(2, 0x425a3394);
c.r(3, 0x1e18a587);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x326e352e);
c.expectreg(1,0x4e7448d2);
c.expectreg(2,0x425a3394);
c.expectreg(3,0x1e18a587);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x77d7d810);
c.r(1, 0x2d06ea94);
c.r(2, 0x3320af92);
c.r(3, 0x5de0da90);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x66dedc28);
c.expectreg(1,0x2d06ea94);
c.expectreg(2,0x3320af92);
c.expectreg(3,0x5de0da90);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x48985d22);
c.r(1, 0x3dfd23d);
c.r(2, 0x21ccd5c0);
c.r(3, 0x634828c7);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x63cb193c);
c.expectreg(1,0x3dfd23d);
c.expectreg(2,0x21ccd5c0);
c.expectreg(3,0x634828c7);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x7ce87afa);
c.r(1, 0x43807486);
c.r(2, 0x160d5c4b);
c.r(3, 0x5cc7c1d0);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x62983959);
c.expectreg(1,0x43807486);
c.expectreg(2,0x160d5c4b);
c.expectreg(3,0x5cc7c1d0);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x77f19a4e);
c.r(1, 0x320edfd8);
c.r(2, 0x484dcbc3);
c.r(3, 0x5f4fcb1c);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x6d73088a);
c.expectreg(1,0x320edfd8);
c.expectreg(2,0x484dcbc3);
c.expectreg(3,0x5f4fcb1c);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x2e7a3754);
c.r(1, 0x7a5c0c7c);
c.r(2, 0x16194f4a);
c.r(3, 0x58a0d40b);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x6330b01a);
c.expectreg(1,0x7a5c0c7c);
c.expectreg(2,0x16194f4a);
c.expectreg(3,0x58a0d40b);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
c.cf();
c.r(0, 0x172f5f4b);
c.r(1, 0x5ee7d892);
c.r(2, 0x6cd1e5a4);
c.r(3, 0x64f0e0e1);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x8d482b86);
c.expectreg(1,0x5ee7d892);
c.expectreg(2,0x6cd1e5a4);
c.expectreg(3,0x64f0e0e1);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(1);
c.expectG(0);
c.cf();
//===========================
c.cf();
c.r(0, 0xfa09b62);
c.r(1, 0x4653afe3);
c.r(2, 0x33805d5e);
c.r(3, 0x771a8d1e);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x85406300);
c.expectreg(1,0x4653afe3);
c.expectreg(2,0x33805d5e);
c.expectreg(3,0x771a8d1e);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(1);
c.expectG(0);
c.cf();
//===========================
c.cf();
c.r(0, 0x14c7f8b5);
c.r(1, 0x75da90f2);
c.r(2, 0x153332a6);
c.r(3, 0xc9fd0c5);
c.x("smlawt r0, r1, r2, r3");
c.expectreg(0,0x16623933);
c.expectreg(1,0x75da90f2);
c.expectreg(2,0x153332a6);
c.expectreg(3,0xc9fd0c5);
c.expectN(0);
c.expectZ(0);
c.expectC(0);
c.expectV(0);
c.expectQ(0);
c.expectG(0);
c.cf();
return;

}

