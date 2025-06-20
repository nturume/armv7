#pragma once
#include "arith.hpp"
#include "assert.h"
#include "decoder.hpp"
#include "mem.hpp"
#include "stuff.hpp"
#include <cmath>
#include <cstdio>
#include <functional>
#include <iosfwd>
#include <string>
#include <system_error>

struct Cpu {

  union APSR {
    struct {
      u32 m : 5;
      u32 t : 1;
      u32 f : 1;
      u32 i : 1;
      u32 a : 1;
      u32 e : 1;
      u32 it : 6 = 0;
      u32 ge : 4;
      u32 _1: 4;
      u32 j: 1 = 0;
      u32 it2 : 2 = 0;
      u32 q : 1;
      u32 v : 1;
      u32 c : 1;
      u32 z : 1;
      u32 n : 1;
    } b;
    u32 back;
  };

  union SCTLR {
    struct {
      u32 m : 1;
      u32 a : 1;
      u32 c : 1;
      u32 _1 : 2 = 0b11;
      u32 cp15ben : 1;
      u32 _9 : 1 = 1;
      u32 b : 1;
      u32 _2 : 2 = 0;
      u32 sw : 1;
      u32 z : 1;
      u32 i : 1;
      u32 v : 1;
      u32 rr : 1;
      u32 _3 : 1 = 0;
      u32 _4 : 1 = 1;
      u32 ha : 1;
      u32 _5 : 1 = 1;
      u32 wxn : 1;
      u32 uwxn : 1;
      u32 fi : 1;
      u32 u : 1 = 1;
      u32 _6 : 1 = 1;
      u32 ve : 1;
      u32 ee : 1;
      u32 _7 : 1 = 0;
      u32 nmfi : 1;
      u32 tre : 1;
      u32 afe : 1;
      u32 te : 1;
      u32 _8 : 1 = 0;
    } b;
    u32 back;
  };

  union SCR {
    struct {
      u32 ns : 1;
      u32 irq : 1;
      u32 fiq : 1;
      u32 ea : 1;
      u32 fw : 1;
      u32 aw : 1;
      u32 net : 1;
      u32 scd : 1;
      u32 hce : 1;
      u32 sif : 1;
    } b;
    u32 back;
  };

  APSR apsr = {};
  SCTLR sctrl = {};
  SCR scr = {};
  // regs
  u32 regs[15][32] = {0};
  // spsr
  u32 spsr_svc;
  u32 spsr_abt;
  u32 spsr_und;
  u32 spsr_irq;
  u32 spsr_fiq;
  // pc
  u32 progcounter;

  u8 memfault = false;

  Memory<1024 * 1024> mem;

  u32 cur;

  inline bool cnd() {
    u8 cond = cur >> 28;
    bool result = false;
    switch (cond >> 1) {
    case 0:
      result = apsr.b.z;
      break;
    case 1:
      result = apsr.b.c;
      break;
    case 0b10:
      result = apsr.b.n;
      break;
    case 0b11:
      result = apsr.b.v;
      break;
    case 0b100:
      result = apsr.b.c and !apsr.b.z;
      break;
    case 0b101:
      result = apsr.b.n == apsr.b.v;
      break;
    case 0b110:
      result = (apsr.b.n == apsr.b.v) and !apsr.b.z;
      break;
    case 0b111:
      result = true;
      break;
    }
    if ((cond & 1) and cond != 0xf) {
      result = !result;
    }
    return result;
  }

  u8 M() { return apsr.b.m; }

  enum class Mode : u8 {
    user = 0b10000,
    fiq = 0b10001,
    irq = 0b10010,
    supervisor = 0b10011,
    monitor = 0b10110,
    abort = 0b10111,
    hyp = 0b11010,
    undefined = 0b11011,
    system = 0b11111,
    _
  };

  bool hasSecurityExt() { return false; }

  bool isSecure() { return false; }

  bool hasVirtExt() { return false; }

  bool badMode(u8 mode) {
    switch (Mode(mode)) {
    case Mode::user:
    case Mode::fiq:
    case Mode::irq:
    case Mode::supervisor:
    case Mode::abort:
    case Mode::undefined:
    case Mode::system:
      return false;
    // case Mode::hyp:return !hasVirtExt();
    // case Mode::monitor:return !hasSecurityExt();
    default:
      printf("bad mode\n");
      assert(false);
    }
  }
  
  u8 getBank() {
    switch(Mode(M())) {
      case Mode::user:
      case Mode::system:
        return u8(Mode::user);
      default:
        return M();
    }  
  }

  inline bool curModeIsUsrSys() {
    if (badMode(M())) {
    }
    return Mode(M()) == Mode::user or Mode(M()) == Mode::system ? true : false;
  }

  inline bool curModeIsHype() {
    if (badMode(M())) {
    }
    return Mode(M()) == Mode::hyp ? true : false;
  }

  inline bool curModeIsntUser() {
    if (badMode(M())) {
    }
    return Mode(M()) == Mode::user ? false : true;
  }

  void cpsr(u32 value) { apsr.back = value; }

  u32 cpsr() { return apsr.back; }

  void spsr(u32 value) {
    Mode mode = Mode(M());
    if (badMode(M())) {
    }
    switch (mode) {
    case Mode::abort:
      spsr_abt = value;
      break;
    case Mode::fiq:
      spsr_fiq = value;
      break;
    case Mode::irq:
      spsr_irq = value;
      break;
    case Mode::supervisor:
      spsr_svc = value;
      break;
    case Mode::undefined:
      spsr_und = value;
      break;
    default:
      printf("wrong mood! %u\n", u32(mode));
      exit(1);
    }
  }

  u32 spsr() {
    Mode mode = Mode(M());
    if (badMode(M())) {
    }
    switch (mode) {
    case Mode::abort:
      return spsr_abt;
    case Mode::fiq:
      return spsr_fiq;
    case Mode::irq:
      return spsr_irq;
    case Mode::supervisor:
      return spsr_svc;
    case Mode::undefined:
      return spsr_und;
    default:
      printf("wrong mood! %u\n", u32(mode));
      exit(1);
    }
  }

  u32 rMode(u8 pos) {
    if (badMode(M())) {
    };
    return regs[getBank()][pos];
  }

  void rMode(u8 pos, u32 value) {
    if (badMode(M())) {
    }
    regs[getBank()][pos] = value;
  }

  inline u32 r(u8 pos) {
    if ((pos & 0xf) == 15)
      return progcounter + 8;
    return rMode(pos & 0xf);
  }

  u32 excVectorBase() {
    if(sctrl.b.v) {
      return 0xffff0000;
    }
    if(hasSecurityExt()) assert(false);
    return 0;
  }

  inline void r(u8 pos, u32 value) { rMode(pos & 0xf, value); }

  inline bool haveLPAE() { return false; }

  void cpsrWriteByInstr(u32 value, u8 bytemask, bool is_excpt_return) {
    bool privileged = curModeIsntUser();
    bool nmfi = sctrl.b.nmfi;
    u32 _cpsr;

    if ((bytemask >> 3) & 1) {
      if (is_excpt_return) {
        _cpsr = (cpsr() & 0xffffff) | (value & 0xff000000);
        cpsr(_cpsr);
      } else {
        _cpsr = (cpsr() & 0x7ffffff) | (value & 0xf8000000);
        cpsr(_cpsr);
      }
    }

    if ((bytemask >> 2) & 1) {
      _cpsr = (cpsr() & 0xfff0ffff) | (value & 0xf0000);
      cpsr(_cpsr);
    }

    if ((bytemask >> 1) & 1) {
      if (is_excpt_return) {
        _cpsr = (cpsr() & 0xffff03ff) | (value & 0xfc00);
        cpsr(_cpsr);
      }
      _cpsr = (cpsr() & ~(1u << 9)) | (value & (1u << 9));
      cpsr(_cpsr);
      if (privileged and (isSecure() or scr.b.aw or hasVirtExt())) {
        _cpsr = (cpsr() & ~(1u << 8)) | (value & (1u << 8));
        cpsr(_cpsr);
      }
    }

    if (bytemask & 1) {
      if (privileged) {
        _cpsr = (cpsr() & ~(1u << 7)) | (value & (1u << 7));
        cpsr(_cpsr);
      }
      if (privileged and (!nmfi or !((value >> 6) & 1)) and
          (isSecure() or scr.b.fw or hasVirtExt())) {
        _cpsr = (cpsr() & ~(1u << 6)) | (value & (1u << 6));
        cpsr(_cpsr);
      }
      if (is_excpt_return) {
        _cpsr = (cpsr() & ~(1u << 5)) | (value & (1u << 5));
        cpsr(_cpsr);
      }
      if (privileged) {
        u8 valmode = value & 0x1f;
        if (badMode(valmode)) {
        }
        if (!isSecure() and valmode == 0b10110)
          assert(false);
        // if(!isSecure() and valmode == 0b10001 and nsacr.rfr)
        // assert(false);//TODO impl NSACR
        if (scr.b.ns == 0 and valmode == 0b11010)
          assert(false);
        if (!isSecure() and apsr.b.m != 0b11010 and valmode == 0b11010)
          assert(false);
        if (apsr.b.m == 0b11010 and valmode == 0b11010 and !is_excpt_return)
          assert(false);
        apsr.b.m = value & 0x1f;
      }
    }
  }

  void spsrWriteByInstr(u32 value, u8 bytemask) {
    if (curModeIsUsrSys()) {
      assert(false);
    }
    u32 _spsr;
    if ((bytemask >> 3) & 1) {
      _spsr = (spsr() & 0xffffff) | (value & 0xff000000);
      spsr(_spsr);
    }
    if ((bytemask >> 2) & 1) {
      _spsr = (spsr() & 0xfff0ffff) | (value & 0xf0000);
      spsr(_spsr);
    }
    if ((bytemask >> 1) & 1) {
      _spsr = (spsr() & 0xffff00ff) | (value & 0xff00);
      spsr(_spsr);
    }
    if (bytemask & 1) {
      if (badMode(value & 0x1f)) {
      }
      _spsr = (spsr() & 0xffffff00) | (value & 0xff);
      spsr(_spsr);
    }
  }

  inline u8 currentInstrSet() { return 0; }

  inline u32 pcStoreValue() { return r(15); }

  inline u32 branchTo(u32 ptr) {
    progcounter = ptr;
    return ptr;
  }

  inline void branchWritePc(u32 ptr) {
    //
    branchTo(ptr & (0xffffffff << 2));
  }

  inline u32 sp() { return r(13); }

  inline void sp(u32 v) { r(13, v); }

  inline u32 rl() { return r(14); }

  inline void rl(u32 v) { r(14, v); }

  inline u32 pc() { return r(15); }

  inline u32 pcReal() { return progcounter; }
  inline void pcReal(u32 v) { branchTo(v); }

  inline u32 bxWritePc(u32 ptr) {
    if ((ptr & 0b11) == 0) {
      return branchTo(ptr);
    }
    printf("%u %x\n", ptr, ptr);
    assert(true ? false : "bad address" == nullptr);
  }

  inline u32 aluWritePc(u32 ptr) { return bxWritePc(ptr); }

  inline u32 loadWritePc(u32 ptr) { return bxWritePc(ptr); }

  inline u32 expandImm(u16 imm12) { return Arith::expandImmC(imm12).v.u; }

  inline u8 n() { return apsr.b.n; }

  inline u8 z() { return apsr.b.z; }

  inline u8 c() { return apsr.b.c; }

  inline u8 v() { return apsr.b.v; }

  inline u8 q() { return apsr.b.q; }

  inline void cf() { apsr.back = 0; }

  inline void n(bool v) { apsr.b.n = v; }

  inline void z(bool v) { apsr.b.z = v; }

  inline void c(bool v) { apsr.b.c = v; }

  inline void v(bool v) { apsr.b.v = v; }

  inline void q(bool v) { apsr.b.q = v; }

  inline u8 ge() { return apsr.b.ge; }

  inline void ge(u8 v) { apsr.b.ge = v & 0xf; }

  void expectreg(u8 pos, u32 value) {
    u32 real = r(pos);
    if (real != value) {
      printf("expected r%d to be 0x%x but found 0x%x\n", pos & 0xf, value,
             real);
      printf("expected r%d to be 0b%b but found 0b%b\n", pos & 0xf, value,
             real);
      printf("r1 0b%b r2 0b%b\n", r(1), r(2));
      printf("r1 0x%x r2 0x%x\n", r(1), r(2));
      abort();
    }
  }

  void expectV(bool value) {
    if (v() != value) {
      printf("Expected V = %d but found %d\n", value, v());
      abort();
    }
  }
  void expectC(bool value) {
    if (c() != value) {
      printf("Expected C = %d but found %d\n", value, c());
      abort();
    }
  }
  void expectZ(bool value) {
    if (z() != value) {
      printf("Expected Z = %d but found %d\n", value, z());
      abort();
    }
  }
  void expectQ(bool value) {
    if (q() != value) {
      printf("Expected Q = %d but found %d\n", value, q());
      abort();
    }
  }
  void expectN(bool value) {
    if (n() != value) {
      printf("Expected N = %d but found %d\n", value, n());
      abort();
    }
  }

  void expectG(u8 value) {
    if (ge() != value) {
      printf("Expected GE = %b but found %b\n", value & 0b1111, ge());
      abort();
    }
  }

  void printRegisters() {
    printf("=========regs==========\n");
    for (u32 i = 0; i < 16; i++) {
      printf("r%d = %d %u %x %b\n", i, r(i), r(i), r(i), r(i));
    }
    printf("=========flags=========\n");
    printf("n: %d, z: %d, c: %d, v: %d, q: %d\n", (u32)apsr.b.n, (u32)apsr.b.z,
           (u32)apsr.b.c, (u32)apsr.b.v, (u32)apsr.b.q);
  }

  u32 exec(u32 word);
  u32 x(const char *prog);

  inline u32 nxt() { return pcReal() + 4; }

  static constexpr u32 bm(u8 n) { return u32(0xffffffff) >> (32 - n); }

  inline u8 decodeRegShift(u8 type) { return type & 0b11; }

  bool intZeroDivTrapping() {
    // TODO
    return false;
  }

  inline void setExclusiveMonitor(u32 addr, u8 n) {
    // TODO
  }

  inline bool exclusiveMonitorsPass(u32 addr, u8 n) {
    // TODO
    return true;
  }

  u32 alignmentFault() {
    // TODO
    assert(false);
  }

  bool unalignedSupport() { return true; }

  inline bool cbit(u8 n) { return (cur >> n) & 1; }

  inline bool aligned64(u32 a) { return (a & 0b111) == 0; }
  inline bool aligned32(u32 a) { return (a & 0b11) == 0; }
  inline bool aligned16(u32 a) { return (a & 0b1) == 0; }

  u32 reset() {
    cpsr(u8(Mode::supervisor));
    apsr.b.e = 0;
    apsr.b.i = 1;
    apsr.b.f = 1;
    apsr.b.a = 1;
    apsr.b.j = 0;
    apsr.b.t = 0;
    apsr.b.it = apsr.b.it2 = 0;

    u32 reset_vector = excVectorBase()&0xfffffffe;
    return branchTo(reset_vector);
  }

  u32 takeUndefInstrException() {
    u32 new_lr_value = pc()-4;
    u32 new_spsr_value = cpsr();
    apsr.b.m = u8(Mode::undefined);
    spsr(new_spsr_value);
    r(14, new_lr_value);
    apsr.b.i = 1;
    apsr.b.it = apsr.b.it2 = 0;
    apsr.b.j = 0;
    apsr.b.t = sctrl.b.te;
    apsr.b.e = sctrl.b.ee;
    return branchTo(excVectorBase()+4);
  }

  u32 takeSVCException() {
    u32 new_lr_value = pc()-4;
    u32 new_spsr_value = cpsr();
    apsr.b.m = u8(Mode::supervisor);
    spsr(new_spsr_value);
    r(14, new_lr_value);
    apsr.b.i = 1;
    apsr.b.it = apsr.b.it2 = 0;
    apsr.b.j = 0;
    apsr.b.t = sctrl.b.te;
    apsr.b.e = sctrl.b.ee;
    return branchTo(excVectorBase()+8);
  }
  
  u32 callSuperVisor(u16 value) {
    (void)value;
   return takeSVCException();
  }

  
  u32 takePrefetchAbortException() {
    u32 new_lr_value = pc()-4;
    u32 new_spsr_value = cpsr();
    apsr.b.m = u8(Mode::abort);
    spsr(new_spsr_value);
    r(14, new_lr_value);
    apsr.b.i = 1;
    apsr.b.it = apsr.b.it2 = 0;
    apsr.b.j = 0;
    apsr.b.t = sctrl.b.te;
    apsr.b.e = sctrl.b.ee;
    return branchTo(excVectorBase()+12);
  }


  u32 takeDataAbortException() {
    u32 new_lr_value = pc();
    u32 new_spsr_value = cpsr();
    apsr.b.m = u8(Mode::abort);
    spsr(new_spsr_value);
    r(14, new_lr_value);
    apsr.b.i = 1;
    apsr.b.it = apsr.b.it2 = 0;
    apsr.b.j = 0;
    apsr.b.t = sctrl.b.te;
    apsr.b.e = sctrl.b.ee;
    return branchTo(excVectorBase()+16);
  }

  inline u32 svc() {
    if(cnd()) {
      return callSuperVisor(cur);
    }
    return nxt();
  }

  inline u32 bx() {
    if (cnd()) {
      return bxWritePc(r(cur));
    }
    return nxt();
  }

  inline u32 b() {
    if (cnd()) {
      u32 imm32 = uns32(s32(cur << 8) >> 6);
      branchWritePc(pc() + imm32);
      return pcReal();
    }
    return nxt();
  }

  inline u32 bl() {
    if (cnd()) {
      u32 imm32 = uns32(s32(cur << 8) >> 6);
      r(14, pc() - 4);
      branchWritePc(pc() + imm32);
      return pcReal();
    }
    return nxt();
  }

  inline u32 ldmda() {
    if (cnd()) {
      u8 n = cur >> 16;
      u16 registers = cur;
      bool wback = cbit(21);
      u8 bitcount = bitcount16(registers);
      u32 address = r(n) - 4 * bitcount + 4;
      u32 newpc;
      bool setpc = false;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
         r(i, mem.a32a(address));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        newpc = loadWritePc(mem.a32a(address));
        setpc = true;
      }
      if (wback) {
        r(n, r(n) - 4 * bitcount);
      }
      if (setpc) {
        return newpc;
      }
    }
    return nxt();
  }

  inline u32 stmda() {
    if (cnd()) {
      u8 n = cur >> 16;
      u16 registers = cur;
      bool wback = cbit(21);
      u8 bitcount = bitcount16(registers);
      u32 address = r(n) - 4 * bitcount + 4;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          mem.a32a(address, r(i));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        mem.a32a(address, pcStoreValue());
      }
      if (wback) {
        r(n, r(n) - 4 * bitcount);
      }
    }
    return nxt();
  }

  inline u32 ldmdb() {
    if (cnd()) {
      u8 n = cur >> 16;
      u16 registers = cur;
      bool wback = cbit(21);
      u8 bitcount = bitcount16(registers);
      u32 address = r(n) - 4 * bitcount;
      u32 newpc;
      bool setpc = false;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          r(i, mem.a32a(address));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        newpc = loadWritePc(mem.a32a(address));
        setpc = true;
      }
      if (wback) {
        r(n, r(n) - 4 * bitcount);
      }
      if (setpc) {
        return newpc;
      }
    }
    return nxt();
  }

  inline u32 stmdb() {
    if (cnd()) {
      u8 n = cur >> 16;
      u16 registers = cur;
      bool wback = cbit(21);
      u8 bitcount = bitcount16(registers);
      u32 address = r(n) - 4 * bitcount;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          r(i, mem.a32a(address));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        mem.a32a(address, pcStoreValue());
      }
      if (wback) {
        r(n, r(n) - 4 * bitcount);
      }
    }
    return nxt();
  }

  inline u32 ldmib() {
    if (cnd()) {
      u8 n = cur >> 16;
      u16 registers = cur;
      bool wback = cbit(21);
      u8 bitcount = bitcount16(registers);
      u32 address = r(n) + 4;
      u32 newpc;
      bool setpc = false;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          r(i, mem.a32a(address));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        newpc = loadWritePc(mem.a32a(address));
        setpc = true;
      }
      if (wback) {
        r(n, r(n) + 4 * bitcount);
      }
      if (setpc) {
        return newpc;
      }
    }
    return nxt();
  }

  inline u32 stmib() {
    if (cnd()) {
      u8 n = cur >> 16;
      u16 registers = cur;
      bool wback = cbit(21);
      u8 bitcount = bitcount16(registers);
      u32 address = r(n) + 4;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          r(i, mem.a32a(address));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        mem.a32a(address, pcStoreValue());
      }
      if (wback) {
        r(n, r(n) + 4 * bitcount);
      }
    }
    return nxt();
  }

  inline u32 ldm() {
    if (cnd()) {
      u16 registers = cur;
      u8 n = cur >> 16;
      bool wback = cbit(21);
      u32 address = r(n);
      bool setpc = false;
      u32 newpc;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          r(i, mem.a32a(address));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        newpc = loadWritePc(mem.a32a(address));
        address += 4;
        setpc = true;
      }
      if (wback) {
        r(n, address);
      }
      if (setpc) {
        return newpc;
      }
    }
    return nxt();
  }

  inline u32 pop() {
    if (cnd()) {
      u16 registers = cur;
      u32 address = sp();
      bool setpc = false;
      u32 newpc;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          r(i, mem.a32a(address));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        newpc = loadWritePc(mem.a32a(address));
        address += 4;
        setpc = true;
      }
      sp(address);
      if (setpc) {
        return newpc;
      }
    }
    return nxt();
  }

  inline u32 stm() {
    if (cnd()) {
      u16 registers = cur;
      u8 n = cur >> 16;
      bool wback = cbit(21);
      u32 address = r(n);
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          mem.a32a(address, r(i));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        mem.a32a(address, pcStoreValue());
        address += 4;
      }
      if (wback) {
        r(n, address);
      }
    }
    return nxt();
  }

  inline u32 push() {
    if (cnd()) {
      u16 registers = cur;
      u32 address = sp() - 4 * bitcount16(registers);
      u32 sp_cpy = address;
      for (u8 i = 0; i < 15 and registers; i++) {
        if (registers & 1) {
          mem.a32a(address, r(i));
          address += 4;
        }
        registers >>= 1;
      }
      if (registers & 1) {
        mem.a32a(address, pcStoreValue());
      }
      sp(sp_cpy);
    }
    return nxt();
  }

  inline u32 ldrex() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      u32 address = r(n);
      setExclusiveMonitor(address, 4);
      r(t, mem.a32a(address));
    }
    return nxt();
  }

  inline u32 strexd() {
    if (cnd()) {
      u8 t = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      u32 address = r(n);
      if (exclusiveMonitorsPass(address, 8)) {
        mem.a32a(address, r(t));
        mem.a32a(address + 4, r(t + 1));
        r(d, 0);
      } else {
        r(d, 1);
      }
    }
    return nxt();
  }

  inline u32 strex() {
    if (cnd()) {
      u8 t = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      u32 address = r(n);
      if (exclusiveMonitorsPass(address, 4)) {
        mem.a32a(address, r(t));
        r(d, 0);
      } else {
        r(d, 1);
      }
    }
    return nxt();
  }

  inline u32 strexb() {
    if (cnd()) {
      u8 t = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      u32 address = r(n);
      if (exclusiveMonitorsPass(address, 1)) {
        mem.a8a(address, r(t));
        r(d, 0);
      } else {
        r(d, 1);
      }
    }
    return nxt();
  }

  inline u32 strexh() {
    if (cnd()) {
      u8 t = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      u32 address = r(n);
      if (exclusiveMonitorsPass(address, 2)) {
        mem.a16a(address, r(t));
        r(d, 0);
      } else {
        r(d, 1);
      }
    }
    return nxt();
  }

  inline u32 ldrexh() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      u32 address = r(n);
      setExclusiveMonitor(address, 2);
      r(t, mem.a16a(address));
    }
    return nxt();
  }

  inline u32 ldrexb() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      u32 address = r(n);
      setExclusiveMonitor(address, 1);
      r(t, mem.a8a(address));
    }
    return nxt();
  }

  inline u32 ldrexd() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      u32 address = r(n);
      if (!aligned64(address)) {
        return alignmentFault();
      }
      setExclusiveMonitor(address, 8);
      r(t, mem.a32a(address));
      r(t + 1, mem.a32a(address + 4));
    }
    return nxt();
  }

  inline u32 strReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto st = Arith::decodeImmShift(cur >> 5, cur >> 7);
      auto offset = Arith::shiftC(r(m), st.t, st.n, c());
      u32 offset_addr = add ? r(n) + offset.u() : r(n) - offset.u();
      u32 address = index ? offset_addr : r(n);
      u32 data = t == 15 ? pcStoreValue() : r(t);
      if (unalignedSupport() or aligned32(address)) {
        mem.a32u(address, data);
      }
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 strbReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto st = Arith::decodeImmShift(cur >> 5, cur >> 7);
      auto offset = Arith::shiftC(r(m), st.t, st.n, c());
      u32 offset_addr = add ? r(n) + offset.u() : r(n) - offset.u();
      u32 address = index ? offset_addr : r(n);
      mem.a8u(address, r(t));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 strImm() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = cur & 0xfff;
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      u32 data = t == 15 ? pcStoreValue() : r(t);
      mem.a32u(address, data);
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto st = Arith::decodeImmShift(cur >> 5, cur >> 7);
      auto offset = Arith::shiftC(r(m), st.t, st.n, c());
      u32 offset_addr = add ? r(n) + offset.u() : r(n) - offset.u();
      u32 address = index ? offset_addr : r(n);
      u32 data = mem.a32u(address);
      if (wback) {
        r(n, offset_addr);
      }
      if (t == 15) {
        if (aligned32(address)) {
          return loadWritePc(data);
        }
      } else if (unalignedSupport() or aligned32(address)) {
        r(t, data);
      }
    }
    return nxt();
  }

  inline u32 ldrhReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto offset = r(m);
      u32 offset_addr = add ? r(n) + offset : r(n) - offset;
      u32 address = index ? offset_addr : r(n);
      u16 data = mem.a16u(address);
      if (wback) {
        r(n, offset_addr);
      }
      if (unalignedSupport() or aligned16(address)) {
        r(t, data);
      }
    }
    return nxt();
  }

  inline u32 strhReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto offset = r(m);
      u32 offset_addr = add ? r(n) + offset : r(n) - offset;
      u32 address = index ? offset_addr : r(n);
      if (unalignedSupport() or aligned16(address)) {
        mem.a16u(address, r(t));
      }
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrshReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto offset = r(m);
      u32 offset_addr = add ? r(n) + offset : r(n) - offset;
      u32 address = index ? offset_addr : r(n);
      i32 data = s16(mem.a16u(address));
      if (wback) {
        r(n, offset_addr);
      }
      if (unalignedSupport() or aligned16(address)) {
        r(t, uns32(data));
      }
    }
    return nxt();
  }

  inline u32 ldrbReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto st = Arith::decodeImmShift(cur >> 5, cur >> 7);
      auto offset = Arith::shiftC(r(m), st.t, st.n, c());
      u32 offset_addr = add ? r(n) + offset.u() : r(n) - offset.u();
      u32 address = index ? offset_addr : r(n);
      r(t, mem.a8u(address));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrsbReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      auto offset = r(m);
      u32 offset_addr = add ? r(n) + offset : r(n) - offset;
      u32 address = index ? offset_addr : r(n);
      r(t, uns32(i32(s8(mem.a8u(address)))));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrdReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 offset_addr = add ? r(n) + r(m) : r(n) - r(m);
      u32 address = index ? offset_addr : r(n);
      r(t, mem.a32a(address));
      r(t + 1, mem.a32a(address + 4));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 strdReg() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      u8 m = cur;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 offset_addr = add ? r(n) + r(m) : r(n) - r(m);
      u32 address = index ? offset_addr : r(n);
      mem.a32a(address, r(t));
      mem.a32a(address + 4, r(t + 1));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrImm() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = cur & 0xfff;
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      u32 data = mem.a32u(address);
      if (wback) {
        r(n, offset_addr);
      }
      if (t == 15) {
        if (aligned32(address)) {
          return loadWritePc(data);
        }
      } else if (unalignedSupport() or aligned32(address)) {
        r(t, data);
      }
    }
    return nxt();
  }

  inline u32 ldrbImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = cur & 0xfff;
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      r(t, mem.a8u(address));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrhImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      u16 data = mem.a16u(address);
      if (wback) {
        r(n, offset_addr);
      }
      if (unalignedSupport() or aligned16(address)) {
        r(t, data);
      }
    }
    return nxt();
  }

  inline u32 strhImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      if (unalignedSupport() or aligned16(address)) {
        mem.a16u(address, r(t));
      }
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrdImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      r(t, mem.a32a(address));
      r(t + 1, mem.a32a(address + 4));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 strdImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      mem.a32a(address, r(t));
      mem.a32a(address + 4, r(t + 1));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrshImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      i32 data = s16(mem.a16u(address));
      if (wback) {
        r(n, offset_addr);
      }
      if (unalignedSupport() or aligned16(address)) {
        r(t, uns32(data));
      }
    }
    return nxt();
  }

  inline u32 ldrsbImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      i32 data = s8(mem.a8u(address));
      r(t, uns32(data));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 strbImm() {
    if (cnd()) {
      u8 t = cur >> 12;
      u8 n = cur >> 16;
      bool index = cbit(24);
      bool add = cbit(23);
      bool wback = !index or cbit(21);
      u32 imm32 = cur & 0xfff;
      u32 offset_addr = add ? r(n) + imm32 : r(n) - imm32;
      u32 address = index ? offset_addr : r(n);
      mem.a8u(address, r(t));
      if (wback) {
        r(n, offset_addr);
      }
    }
    return nxt();
  }

  inline u32 ldrLit() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      bool add = cbit(23);
      u32 base = align4(pc());
      u32 imm32 = cur & 0xfff;
      u32 address = add ? base + imm32 : base - imm32;
      u32 data = mem.a32u(address);
      if (t == 15) {
        if (aligned32(address)) {
          return loadWritePc(data);
        }
      } else if (unalignedSupport() or aligned32(address)) {
        r(t, data);
      }
    }
    return nxt();
  }

  inline u32 ldrbLit() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      bool add = cbit(23);
      u32 base = align4(pc());
      u32 imm32 = cur & 0xfff;
      u32 address = add ? base + imm32 : base - imm32;
      r(t, mem.a8u(address));
    }
    return nxt();
  }

  inline u32 ldrhLit() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      bool add = cbit(23);
      u32 base = align4(pc());
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 address = add ? base + imm32 : base - imm32;
      u16 data = mem.a16u(address);
      if (unalignedSupport() or aligned16(address)) {
        r(t, data);
      }
    }
    return nxt();
  }

  inline u32 ldrdLit() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      bool add = cbit(23);
      u32 base = align4(pc());
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 address = add ? base + imm32 : base - imm32;
      r(t, mem.a32a(address));
      r(t + 1, mem.a32a(address + 4));
    }
    return nxt();
  }

  inline u32 ldrshLit() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      bool add = cbit(23);
      u32 base = align4(pc());
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 address = add ? base + imm32 : base - imm32;
      i32 data = s16(mem.a16u(address));
      if (unalignedSupport() or aligned16(address)) {
        r(t, uns32(data));
      }
    }
    return nxt();
  }

  inline u32 ldrsbLit() {
    if (cnd()) {
      u8 t = (cur >> 12) & 0xf;
      bool add = cbit(23);
      u32 base = align4(pc());
      u32 imm32 = ((cur >> 4) & 0xf0) | (cur & 0xf);
      u32 address = add ? base + imm32 : base - imm32;
      i32 data = s8(mem.a8u(address));
      r(t, uns32(data));
    }
    return nxt();
  }

#define NEG 0x80000000

  inline u32 msrApp() {
    if (cnd()) {
      u8 n = cur;
      u8 mask = (cur >> 18) & 0b11;
      bool write_nzcq = mask >> 1;
      bool write_g = mask & 1;
      if (write_nzcq) {
        apsr.back |= (r(n) & (0xf8000000));
      }
      if (write_g) {
        ge(r(n) >> 16);
      }
    }
    return nxt();
  }

  inline u32 msrImmApp() {
    if (cnd()) {
      u32 imm32 = expandImm(cur);
      u8 mask = (cur >> 18) & 0b11;
      bool write_nzcq = mask >> 1;
      bool write_g = mask & 1;
      if (write_nzcq) {
        apsr.back |= (imm32 & (0xf8000000));
      }
      if (write_g) {
        ge(imm32 >> 16);
      }
    }
    return nxt();
  }

  inline u32 mrs() {
    if (cnd()) {
      u8 d = cur >> 12;
      bool read_spsr = (cur & (1 << 22)) > 0;
      if (read_spsr) {
        assert("msr read_spsr" == nullptr);
      } else {
        r(d, apsr.back & 0b11111000111111110000001111011111);
      }
    }
    return nxt();
  }

  inline u32 mvnImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      bool setflags = (cur & (1 << 20)) > 0;
      auto imm = Arith::expandImmC(u16(cur));
      u32 res = ~imm.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(imm.c);
      }
    }
    return nxt();
  }

  inline u32 mvnReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      shifted.v.u = ~shifted.v.u;
      if (d == 15) {
        return aluWritePc(shifted.u());
      }
      r(d, shifted.u());
      if (setflags) {
        n(shifted.n());
        z(shifted.z());
        c(shifted.c);
      }
    }
    return nxt();
  }

  inline u32 mvnShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 s = cur >> 8;
      u8 d = (cur >> 12) & 0xf;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c());
      shifted.v.u = ~shifted.v.u;
      r(d, shifted.u());
      if (setflags) {
        n(shifted.n());
        z(shifted.z());
        c(shifted.c);
      }
    }
    return nxt();
  }

  inline u32 adcImm() {
    if (cnd()) {
      u8 d = (cur >> 12);
      u8 _n = (cur >> 16);
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Adc res = Arith::adc(r(_n), expandImm((u16)cur), apsr.b.c);
      if (d == 15) {
        return aluWritePc(res.r);
      }
      r(d, res.r);
      if (setflags) {
        n(res.n());
        z(res.z());
        c(res.c);
        v(res.f);
      }
    }
    return nxt();
  }

  inline u32 adcReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(r(_n), shifted.v.u, c());
      if (d == 15) {
        return aluWritePc(adc.r);
      }
      r(d, adc.r);
      if (setflags) {
        n(adc.n());
        z(adc.z());
        c(adc.c);
        v(adc.f);
      }
    }
    return nxt();
  }

  inline u32 adcShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(r(_n), shifted, c());
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 sbcImm() {
    if (cnd()) {
      u8 d = (cur >> 12);
      u8 _n = (cur >> 16);
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Adc res = Arith::adc(r(_n), ~expandImm((u16)cur), apsr.b.c);
      if (d == 15) {
        return aluWritePc(res.r);
      }
      r(d, res.r);
      if (setflags) {
        n(res.n());
        z(res.z());
        c(res.c);
        v(res.f);
      }
    }
    return nxt();
  }

  inline u32 sbcReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(r(_n), ~shifted.v.u, c());
      if (d == 15) {
        return aluWritePc(adc.r);
      }
      r(d, adc.r);
      if (setflags) {
        n(adc.n());
        z(adc.z());
        c(adc.c);
        v(adc.f);
      }
    }
    return nxt();
  }

  inline u32 sbcShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(r(_n), ~shifted, c());
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 addImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Adc a = Arith::adc(r(_n), expandImm(u16(cur)), false);
      if (d == 15)
        return aluWritePc(a.r);
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 addReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(r(_n), shifted.v.u, 0);
      if (d == 15) {
        return aluWritePc(adc.r);
      }
      r(d, adc.r);
      if (setflags) {
        n(adc.n());
        z(adc.z());
        c(adc.c);
        v(adc.f);
      }
    }
    return nxt();
  }

  inline u32 addShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(r(_n), shifted, 0);
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 subImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Adc a = Arith::adc(r(_n), ~expandImm(u16(cur)), 1);
      if (d == 15)
        return aluWritePc(a.r);
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 subReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(r(_n), ~shifted.u(), 1);
      if (d == 15) {
        return aluWritePc(adc.r);
      }
      r(d, adc.r);
      if (setflags) {
        n(adc.n());
        z(adc.z());
        c(adc.c);
        v(adc.f);
      }
    }
    return nxt();
  }

  inline u32 subShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(r(_n), ~shifted, 1);
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 rscImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Adc a = Arith::adc(~r(_n), expandImm(u16(cur)), c());
      if (d == 15)
        return aluWritePc(a.r);
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 rscReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(~r(_n), shifted.u(), c());
      if (d == 15) {
        return aluWritePc(adc.r);
      }
      r(d, adc.r);
      if (setflags) {
        n(adc.n());
        z(adc.z());
        c(adc.c);
        v(adc.f);
      }
    }
    return nxt();
  }

  inline u32 rscShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(~r(_n), shifted, c());
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 rsbImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Adc a = Arith::adc(~r(_n), expandImm(u16(cur)), 1);
      if (d == 15)
        return aluWritePc(a.r);
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 rsbReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(~r(_n), shifted.u(), 1);
      if (d == 15) {
        return aluWritePc(adc.r);
      }
      r(d, adc.r);
      if (setflags) {
        n(adc.n());
        z(adc.z());
        c(adc.c);
        v(adc.f);
      }
    }
    return nxt();
  }

  inline u32 rsbShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(~r(_n), shifted, 1);
      r(d, a.r);
      if (setflags) {
        n(a.n());
        z(a.z());
        c(a.c);
        v(a.f);
      }
    }
    return nxt();
  }

  inline u32 cmnImm() {
    if (cnd()) {
      u8 _n = cur >> 16;
      Arith::Adc a = Arith::adc(r(_n), expandImm(u16(cur)), 0);
      n(a.n());
      z(a.z());
      c(a.c);
      v(a.f);
    }
    return nxt();
  }

  inline u32 cmnReg() {
    if (cnd()) {
      u8 m = cur;
      u8 _n = cur >> 16;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(r(_n), shifted.u(), 0);
      n(adc.n());
      z(adc.z());
      c(adc.c);
      v(adc.f);
    }
    return nxt();
  }

  inline u32 cmnShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(r(_n), shifted, 0);
      n(a.n());
      z(a.z());
      c(a.c);
      v(a.f);
    }
    return nxt();
  }

  inline u32 cmpImm() {
    if (cnd()) {
      u8 _n = cur >> 16;
      Arith::Adc a = Arith::adc(r(_n), ~expandImm(u16(cur)), 1);
      n(a.n());
      z(a.z());
      c(a.c);
      v(a.f);
    }
    return nxt();
  }

  inline u32 cmpReg() {
    if (cnd()) {
      u8 m = cur;
      u8 _n = cur >> 16;
      Arith::Is s = Arith::decodeImmShift((cur >> 5), (cur >> 7));
      Arith::Res shifted = Arith::shiftC(r(m), s.t, s.n, c());
      Arith::Adc adc = Arith::adc(r(_n), ~shifted.u(), 1);
      n(adc.n());
      z(adc.z());
      c(adc.c);
      v(adc.f);
    }
    return nxt();
  }

  inline u32 cmpShiftedReg() {
    if (cnd()) {
      u8 m = cur;
      u8 s = cur >> 8;
      u8 _n = cur >> 16;
      u32 shifted =
          Arith::shiftC(r(m), decodeRegShift(cur >> 5), (u8)r(s), c()).u();
      Arith::Adc a = Arith::adc(r(_n), ~shifted, 1);
      n(a.n());
      z(a.z());
      c(a.c);
      v(a.f);
    }
    return nxt();
  }

  inline u32 adr() {
    if (cnd()) {
      u32 imm32 = expandImm((u16)cur);
      u32 d = (cur >> 12) & 0xf;
      u32 res =
          (cur & ((1) << 23)) ? align4(pc()) + imm32 : align4(pc()) - imm32;
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
    }
    return nxt();
  }

  inline u32 andImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Res shift = Arith::expandImmC(u16(cur));
      u32 res = r(_n) & shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 andReg() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift(cur >> 5, cur >> 7);
      Arith::Res shift = Arith::shiftC(r(m), s.t, s.n, c());
      u32 res = r(_n) & shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 andShiftedReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur >> 16;
      u8 m = cur;
      u8 s = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), decodeRegShift(cur >> 5), u8(r(s)), c());
      u32 res = r(_n) & shift.u();
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 bicImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Res shift = Arith::expandImmC(u16(cur));
      u32 res = r(_n) & ~shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 bicReg() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift(cur >> 5, cur >> 7);
      Arith::Res shift = Arith::shiftC(r(m), s.t, s.n, c());
      u32 res = r(_n) & ~shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 bicShiftedReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur >> 16;
      u8 m = cur;
      u8 s = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), decodeRegShift(cur >> 5), u8(r(s)), c());
      u32 res = r(_n) & ~shift.u();
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 eorImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Res shift = Arith::expandImmC(u16(cur));
      u32 res = r(_n) ^ shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 eorReg() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift(cur >> 5, cur >> 7);
      Arith::Res shift = Arith::shiftC(r(m), s.t, s.n, c());
      u32 res = r(_n) ^ shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 eorShiftedReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur >> 16;
      u8 m = cur;
      u8 s = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), decodeRegShift(cur >> 5), u8(r(s)), c());
      u32 res = r(_n) ^ shift.u();
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 orrImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Res shift = Arith::expandImmC(u16(cur));
      u32 res = r(_n) | shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 orrReg() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 _n = cur >> 16;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Is s = Arith::decodeImmShift(cur >> 5, cur >> 7);
      Arith::Res shift = Arith::shiftC(r(m), s.t, s.n, c());
      u32 res = r(_n) | shift.u();
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 orrShiftedReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur >> 16;
      u8 m = cur;
      u8 s = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), decodeRegShift(cur >> 5), u8(r(s)), c());
      u32 res = r(_n) | shift.u();
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 teqImm() {
    if (cnd()) {
      u8 _n = cur >> 16;
      Arith::Res shift = Arith::expandImmC(u16(cur));
      u32 res = r(_n) ^ shift.u();
      n((res & NEG) > 0);
      z(res == 0);
      c(shift.c);
    }
    return nxt();
  }

  inline u32 teqReg() {
    if (cnd()) {
      u8 _n = cur >> 16;
      u8 m = cur;
      Arith::Is s = Arith::decodeImmShift(cur >> 5, cur >> 7);
      Arith::Res shift = Arith::shiftC(r(m), s.t, s.n, c());
      u32 res = r(_n) ^ shift.u();
      n((res & NEG) > 0);
      z(res == 0);
      c(shift.c);
    }
    return nxt();
  }

  inline u32 teqShiftedReg() {
    if (cnd()) {
      u8 _n = cur >> 16;
      u8 m = cur;
      u8 s = cur >> 8;
      auto shift = Arith::shiftC(r(m), decodeRegShift(cur >> 5), u8(r(s)), c());
      u32 res = r(_n) ^ shift.u();
      n((res & NEG) > 0);
      z(res == 0);
      c(shift.c);
    }
    return nxt();
  }

  inline u32 tstImm() {
    if (cnd()) {
      u8 _n = cur >> 16;
      Arith::Res shift = Arith::expandImmC(u16(cur));
      u32 res = r(_n) & shift.u();
      n((res & NEG) > 0);
      z(res == 0);
      c(shift.c);
    }
    return nxt();
  }

  inline u32 tstReg() {
    if (cnd()) {
      u8 _n = cur >> 16;
      u8 m = cur;
      Arith::Is s = Arith::decodeImmShift(cur >> 5, cur >> 7);
      Arith::Res shift = Arith::shiftC(r(m), s.t, s.n, c());
      u32 res = r(_n) & shift.u();
      n((res & NEG) > 0);
      z(res == 0);
      c(shift.c);
    }
    return nxt();
  }

  inline u32 tstShiftedReg() {
    if (cnd()) {
      u8 _n = cur >> 16;
      u8 m = cur;
      u8 s = cur >> 8;
      auto shift = Arith::shiftC(r(m), decodeRegShift(cur >> 5), u8(r(s)), c());
      u32 res = r(_n) & shift.u();
      n((res & NEG) > 0);
      z(res == 0);
      c(shift.c);
    }
    return nxt();
  }

  inline u32 asrImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), 0b10, cur >> 7, c());
      if (d == 15)
        return aluWritePc(shift.u());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 asrReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur;
      u8 m = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(_n), 0b10, r(m), c());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 lslImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), 0, cur >> 7, c());
      if (d == 15)
        return aluWritePc(shift.u());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 lslReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur;
      u8 m = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(_n), 0, r(m), c());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 lsrImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), 1, cur >> 7, c());
      if (d == 15)
        return aluWritePc(shift.u());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 lsrReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur;
      u8 m = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(_n), 1, r(m), c());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 rorImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), 0b11, cur >> 7, c());
      if (d == 15)
        return aluWritePc(shift.u());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 rorReg() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur;
      u8 m = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(_n), 0b11, r(m), c());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 rrx() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 m = cur;
      bool setflags = (cur & (1 << 20)) > 0;
      auto shift = Arith::shiftC(r(m), 4, 1, c());
      if (d == 15)
        return aluWritePc(shift.u());
      r(d, shift.u());
      if (setflags) {
        n(shift.n());
        z(shift.z());
        c(shift.c);
      }
    }
    return nxt();
  }

  inline u32 movImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      auto imm = Arith::expandImmC(cur);
      bool setflags = (cur & (1 << 20)) > 0;
      if (d == 15)
        return aluWritePc(imm.u());
      r(d, imm.u());
      if (setflags) {
        n(imm.n());
        z(imm.z());
        c(imm.c);
      }
    }
    return nxt();
  }

  inline u32 movImm16() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u32 imm32 = (((cur >> 4) & 0xf000) | (cur & 0xfff));
      if (d == 15)
        return aluWritePc(imm32);
      r(d, imm32);
    }
    return nxt();
  }

  inline u32 movReg() {
    if (cnd()) {
      u8 m = cur;
      u8 d = (cur >> 12) & 0xf;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 res = r(m);
      if (d == 15)
        return aluWritePc(res);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 movt() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u32 imm32 = (((cur >> 4) & 0xf000) | (cur & 0xfff));
      r(d, (r(d) & 0xffff) | imm32 << 16);
    }
    return nxt();
  }

  inline u32 bfc() {
    if (cnd()) {
      u8 lsbit = (cur >> 7) & 0b11111;
      u8 d = cur >> 12;
      u8 msbit = (cur >> 16) & 0b11111;

      u32 a = u64(0xffffffff) << (msbit + 1);
      u32 b = u64(0xffffffff) >> (32 - lsbit);

      r(d, u32(a | b) & r(d));
    }
    return nxt();
  }

  inline u32 bfi() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 _n = cur;
      u8 msbit = ((cur >> 16) & 0b11111) + 1;
      u8 lsbit = (cur >> 7) & 0b11111;
      u32 a = u64(0xffffffff) >> (32 - (msbit - lsbit));
      u32 res = u64(r(_n) & a) << lsbit;

      u32 ma = u64(0xffffffff) << msbit;
      u32 mb = u64(0xffffffff) >> (32 - lsbit);

      u32 cl = u32(ma | mb) & r(d);

      r(d, res | cl);
    }
    return nxt();
  }

  inline u32 clz() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u32 res = 0;
      asm volatile("lzcntl %[a], %[b]" : [b] "=r"(res) : [a] "r"(r(m)));
      r(d, res);
    }
    return nxt();
  }

  inline u32 mla() {
    if (cnd()) {
      u8 d = cur >> 16;
      u8 _n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 res = r(_n) * r(m) + r(a);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 mls() {
    if (cnd()) {
      u8 d = cur >> 16;
      u8 _n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 res = r(a) - r(_n) * r(m);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 mul() {
    if (cnd()) {
      u8 d = cur >> 16;
      u8 _n = cur;
      u8 m = cur >> 8;
      bool setflags = (cur & (1 << 20)) > 0;
      u32 res = r(_n) * r(m);
      r(d, res);
      if (setflags) {
        n((res & NEG) > 0);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 pkh() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u8 m = cur;
      u8 tbform = (cur >> 6) & 1;
      auto shift = Arith::decodeImmShift(tbform << 1, cur >> 7);
      auto operand = Arith::shiftC(r(m), shift.t, shift.n, c());
      if (tbform) {
        r(d, (r(d) & 0xffff0000) | (operand.u() & 0xffff));
        r(d, (r(d) & 0xffff) | (r(n) & 0xffff0000));
      } else {
        r(d, (r(d) & 0xffff0000) | (r(n) & 0xffff));
        r(d, (r(d) & 0xffff) | (operand.u() & 0xffff0000));
      }
    }
    return nxt();
  }

  inline u32 qadd() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      auto sat = Arith::s32satAdd(r(m), r(n));
      r(d, sat.u());
      q(sat.sat());
    }
    return nxt();
  }

  inline u32 qadd16() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      auto sum1 = Arith::s16satAdd(r(n), r(m));
      auto sum2 = Arith::s16satAdd(r(n) >> 16, r(m) >> 16);
      r(d, (sum1.u() & 0xffff) | (sum2.u() << 16));
    }
    return nxt();
  }

  inline u32 qadd8() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      using namespace Arith;
      auto sum1 = s8satAdd(r(n), r(m));
      auto sum2 = s8satAdd(r(n) >> 8, r(m) >> 8);
      auto sum3 = s8satAdd(r(n) >> 16, r(m) >> 16);
      auto sum4 = s8satAdd(r(n) >> 24, r(m) >> 24);
      r(d, (sum1.u() & 0xff) | ((sum2.u() & 0xff) << 8) |
               ((sum3.u() & 0xff) << 16) | ((sum4.u() & 0xff) << 24));
    }
    return nxt();
  }

  inline u32 qsub() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      auto sat = Arith::s32satSub(r(m), r(n));
      r(d, sat.u());
      q(sat.sat());
    }
    return nxt();
  }

  inline u32 qsub16() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      auto diff1 = Arith::s16satSub(r(n), r(m));
      auto diff2 = Arith::s16satSub(r(n) >> 16, r(m) >> 16);
      r(d, (diff1.u() & 0xffff) | (diff2.u() << 16));
    }
    return nxt();
  }

  inline u32 qsub8() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      using namespace Arith;
      auto diff1 = s8satSub(r(n), r(m));
      auto diff2 = s8satSub(r(n) >> 8, r(m) >> 8);
      auto diff3 = s8satSub(r(n) >> 16, r(m) >> 16);
      auto diff4 = s8satSub(r(n) >> 24, r(m) >> 24);
      r(d, (diff1.u() & 0xff) | ((diff2.u() & 0xff) << 8) |
               ((diff3.u() & 0xff) << 16) | ((diff4.u() & 0xff) << 24));
    }
    return nxt();
  }

  inline u32 qasx() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      using namespace Arith;
      auto diff = s16satSub(r(n), r(m) >> 16);
      auto sum = s16satAdd(r(n) >> 16, r(m));
      r(d, (diff.u() & 0xffff) | (sum.u() << 16));
    }
    return nxt();
  }

  inline u32 qsax() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      using namespace Arith;
      auto sum = s16satAdd(r(n), r(m) >> 16);
      auto diff = s16satSub(r(n) >> 16, r(m));
      r(d, (sum.u() & 0xffff) | (diff.u() << 16));
    }
    return nxt();
  }

  inline u32 qdadd() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      using namespace Arith;
      auto sat1 = s32satAdd(r(n), r(n));
      auto sat2 = s32satAdd(r(m), sat1.u());
      r(d, sat2.u());
      if (sat1.sat() || sat2.sat())
        q(1);
    }
    return nxt();
  }

  inline u32 qdsub() {
    if (cnd()) {
      u8 m = cur;
      u8 n = cur >> 16;
      u8 d = cur >> 12;
      using namespace Arith;
      auto sat1 = s32satAdd(r(n), r(n));
      auto sat2 = s32satSub(r(m), sat1.u());
      r(d, sat2.u());
      if (sat1.sat() || sat2.sat())
        q(1);
    }
    return nxt();
  }

  inline u32 rbit() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u32 res = 0;
      for (int i = 0; i < 32; i++) {
        res |= ((r(m) >> i) & 1) << (31 - i);
      }
      r(d, res);
    }
    return nxt();
  }

  inline u32 rev() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 m = cur;
      u32 src = r(m);
      u32 dest = 0;
      char *src_ptr = (char *)&src;
      char *dest_ptr = (char *)&dest;

      dest_ptr[0] = src_ptr[3];
      dest_ptr[1] = src_ptr[2];
      dest_ptr[2] = src_ptr[1];
      dest_ptr[3] = src_ptr[0];

      r(d, dest);
    }
    return nxt();
  }

  inline u32 rev16() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 m = cur;

      u32 src = r(m);
      u32 dest = 0;
      char *src_ptr = (char *)&src;
      char *dest_ptr = (char *)&dest;

      dest_ptr[0] = src_ptr[1];
      dest_ptr[1] = src_ptr[0];
      dest_ptr[2] = src_ptr[3];
      dest_ptr[3] = src_ptr[2];

      r(d, dest);
    }

    return nxt();
  }

  inline u32 revsh() {
    if (cnd()) {
      u8 d = cur >> 12;
      u8 m = cur;

      u32 src = r(m);
      u32 dest = 0;
      char *src_ptr = (char *)&src;
      char *dest_ptr = (char *)&dest;

      dest_ptr[0] = src_ptr[1];
      dest_ptr[1] = src_ptr[0];

      r(d, uns32(i32(s16(dest))));
    }

    return nxt();
  }

  inline u32 sbfx() {
    if (cnd()) {
      u8 n = cur;
      u8 lsbit = (cur >> 7) & 0b11111;
      u8 d = cur >> 12;
      u8 widthminus1 = (cur >> 16) & 0b11111;

      u8 msbit = lsbit + widthminus1;

      u32 a = u64(0xffffffff) << (msbit + 1);
      u32 b = u64(0xffffffff) >> (32 - lsbit);

      u32 x = ~u32(a | b) & r(n);
      if (x & (u32(1 << msbit))) {
        x |= a;
      }
      x = s32(x) >> lsbit;
      r(d, x);
    }
    return nxt();
  }

  inline u32 ubfx() {
    if (cnd()) {
      u8 n = cur;
      u8 lsbit = (cur >> 7) & 0b11111;
      u8 d = cur >> 12;
      u8 widthminus1 = (cur >> 16) & 0b11111;

      u8 msbit = lsbit + widthminus1;

      u32 a = u64(0xffffffff) << (msbit + 1);
      u32 b = u64(0xffffffff) >> (32 - lsbit);

      u32 x = ~u32(a | b) & r(n);
      x = x >> lsbit;
      r(d, x);
    }
    return nxt();
  }

  inline u32 sdiv() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      i32 result;
      if (r(m) == 0) {
        if (intZeroDivTrapping()) {
          assert("division by zero" == nullptr);
        } else {
          result = 0;
        }
      } else {
        if (r(n) == 0x80000000 and r(m) == 0xffffffff) {
          result = 0x80000000;
        } else {
          result = s32(r(n)) / s32(r(m));
        }
      }
      r(d, uns32(result));
    }

    return nxt();
  }

  inline u32 udiv() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      u32 result;
      if (r(m) == 0) {
        if (intZeroDivTrapping()) {
          assert("division by zero" == nullptr);
        } else {
          result = 0;
        }
      } else {
        result = r(n) / r(m);
      }
      r(d, result);
    }
    return nxt();
  }

  inline u32 umaal() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;

      u64 res = u64(r(n)) * u64(r(m)) + u64(r(dhi)) + u64(r(dlo));
      r(dhi, res >> 32);
      r(dlo, res);
    }
    return nxt();
  }

  inline u32 umlal() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;

      bool setflags = (cur >> 20) & 1;

      u64 res = u64(r(n)) * u64(r(m)) + ((u64(r(dhi)) << 32) | u64(r(dlo)));
      r(dhi, res >> 32);
      r(dlo, res);

      if (setflags) {
        Cpu::n((res >> 63) & 1);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 umull() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;

      bool setflags = (cur >> 20) & 1;

      u64 res = u64(r(n)) * u64(r(m));
      r(dhi, res >> 32);
      r(dlo, res);
      if (setflags) {
        Cpu::n((res >> 63) & 1);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 uxtab() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, r(n) + (rotated.u() & 0xff));
    }
    return nxt();
  }

  inline u32 sxtab() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, r(n) + sx8(rotated.u()));
    }
    return nxt();
  }

  inline u32 uxtab16() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      auto rotated = Arith::ror32(r(m), rotation);
      printf("rotation = %d, %x \n", rotation,
             (r(n) % 0xffff) + (rotated.u() & 0xff));
      u32 a = (r(n) & 0xffff) + (rotated.u() & 0xff);
      u32 b = (r(n) >> 16) + ((rotated.u() >> 16) & 0xff);
      r(d, (a & 0xffff) | (b << 16));
    }
    return nxt();
  }

  inline u32 sxtab16() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      auto rotated = Arith::ror32(r(m), rotation);
      u32 res = (u16(r(n)) + sx8(rotated.u())) |
                (u32((u16(r(n) >> 16)) + sx8(rotated.u() >> 16)) << 16);
      r(d, res);
    }
    return nxt();
  }

  inline u32 uxtah() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, r(n) + (rotated.u() & 0xffff));
    }
    return nxt();
  }

  inline u32 sxtah() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, r(n) + sx16(rotated.u()));
    }
    return nxt();
  }

  inline u32 uxtb() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, rotated.u() & 0xff);
    }
    return nxt();
  }

  inline u32 sxtb() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, sx8(rotated.u()));
    }
    return nxt();
  }

  inline u32 uxth() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, rotated.u() & 0xffff);
    }
    return nxt();
  }

  inline u32 sxth() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, sx16(rotated.u()));
    }
    return nxt();
  }

  inline u32 uxtb16() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      auto rotated = Arith::ror32(r(m), rotation);
      r(d, (rotated.u() & 0xff00ff));
    }
    return nxt();
  }

  inline u32 sxtb16() {
    if (cnd()) {
      u8 m = cur;
      u8 rotation = (cur >> 7) & 0b11000;
      u8 d = cur >> 12;
      auto rotated = Arith::ror32(r(m), rotation);
      u32 lo = sx8(rotated.u());
      u32 hi = sx8(rotated.u() >> 16);
      r(d, (hi << 16) | (lo & 0xffff));
    }
    return nxt();
  }

  inline u32 smlabb() {
    if (cnd()) {
      u8 d = cur >> 16;
      u8 n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      bool n_high = (cur >> 5) & 1;
      bool m_high = (cur >> 6) & 1;

      u32 op1 = n_high ? r(n) >> 16 : r(n) & 0xffff;
      u32 op2 = m_high ? r(m) >> 16 : r(m) & 0xffff;
      i64 res = i64(s16(op1)) * i64(s16(op2)) + i64(s32(r(a)));
      r(d, uns32(res));
      if (res != (s32(r(d)))) {
        q(1);
      }
    }
    return nxt();
  }

  inline u32 smlad() {
    if (cnd()) {
      u8 d = cur >> 16;
      u8 n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      bool m_swap = (cur >> 5) & 1;
      u32 op2 = m_swap ? Arith::ror32(r(m), 16).u() : r(m);
      i32 p1 = i32(s16(r(n))) * i32(s16(op2));
      i32 p2 = i32(s16(r(n) >> 16)) * i32(s16(op2 >> 16));
      i64 res = i64(p1) + i64(p2) + i64(s32(r(a)));
      r(d, uns32(res));
      if (res != i32(res)) {
        q(1);
      }
    }
    return nxt();
  }

  inline u32 smlal() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;
      bool setflags = (cur >> 20) & 1;
      i64 res =
          i64(s32(r(n))) * i64(s32(r(m))) + s64((u64(r(dhi)) << 32) | r(dlo));
      r(dhi, res >> 32);
      r(dlo, res);
      if (setflags) {
        Cpu::n((res >> 63) & 1);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 smlalbb() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;

      bool n_high = (cur >> 5) & 1;
      bool m_high = (cur >> 6) & 1;

      u32 op1 = n_high ? r(n) >> 16 : r(n) & 0xffff;
      u32 op2 = m_high ? r(m) >> 16 : r(m) & 0xffff;
      i64 res =
          i64(s16(op1)) * i64(s16(op2)) + s64((u64(r(dhi)) << 32) | r(dlo));

      r(dhi, uns32(res >> 32));
      r(dlo, uns32(res));
    }
    return nxt();
  }

  inline u32 smlald() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;
      bool m_swap = (cur >> 5) & 1;
      u32 op2 = m_swap ? Arith::ror32(r(m), 16).u() : r(m);
      i32 p1 = i32(s16(r(n))) * i32(s16(op2));
      i32 p2 = i32(s16(r(n) >> 16)) * i32(s16(op2 >> 16));
      i64 res = i64(p1) + i64(p2) + s64((u64(r(dhi)) << 32) | r(dlo));
      r(dhi, uns32(res >> 32));
      r(dlo, uns32(res));
    }
    return nxt();
  }

  inline u32 smlawb() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      u8 d = cur >> 16;
      bool m_high = (cur >> 6) & 1;
      u32 op2 = m_high ? r(m) >> 16 : r(m) & 0xffff;
      i64 res = ((i64(s32(r(n))) * i64(s16(op2))) >> 16) + s32(r(a));
      r(d, uns32(res));
      if ((res) != s32(r(d))) {
        q(1);
      }
    }
    return nxt();
  }

  inline u32 smlsd() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      u8 d = cur >> 16;
      bool m_swap = (cur >> 5) & 1;
      u32 op2 = m_swap ? Arith::ror32(r(m), 16).u() : r(m);
      i64 p1 = i64(s16(r(n))) * i64(s16(op2));
      i64 p2 = i64(s16(r(n) >> 16)) * i64(s16(op2 >> 16));
      i64 res = p1 - p2 + s32(r(a));
      r(d, uns32(res));
      if (res != i32(res)) {
        q(true);
      }
    }
    return nxt();
  }

  inline u32 smlsld() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;
      bool m_swap = (cur >> 5) & 1;
      u32 op2 = m_swap ? Arith::ror32(r(m), 16).u() : r(m);
      i64 p1 = i64(s16(r(n))) * i64(s16(op2));
      i64 p2 = i64(s16(r(n) >> 16)) * i64(s16(op2 >> 16));
      i64 res = p1 - p2 + s64((u64(r(dhi)) << 32) | r(dlo));
      r(dhi, uns32(res >> 32));
      r(dlo, uns32(res));
    }
    return nxt();
  }

  inline u32 smmla() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      u8 d = cur >> 16;
      bool round = (cur >> 5) & 1;
      u64 res = (i64(s32(r(a))) << 32) + (i64(s32(r(n))) * i64(s32(r(m))));
      if (round) {
        res += 0x80000000;
      }
      r(d, uns32(res >> 32));
    }
    return nxt();
  }

  inline u32 smmls() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 a = cur >> 12;
      u8 d = cur >> 16;
      bool round = (cur >> 5) & 1;
      u64 res = (i64(s32(r(a))) << 32) - (i64(s32(r(n))) * i64(s32(r(m))));
      if (round) {
        res += 0x80000000;
      }
      r(d, uns32(res >> 32));
    }
    return nxt();
  }

  inline u32 smmul() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      bool round = (cur >> 5) & 1;
      u64 res = i64(s32(r(n))) * i64(s32(r(m)));
      if (round) {
        res += 0x80000000;
      }
      r(d, uns32(res >> 32));
    }
    return nxt();
  }

  inline u32 smuad() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      bool m_swap = (cur >> 5) & 1;
      u32 op2 = m_swap ? Arith::ror32(r(m), 16).u() : r(m);
      i64 p1 = i64(s16(r(n))) * i64(s16(op2));
      i64 p2 = i64(s16(r(n) >> 16)) * i64(s16(op2 >> 16));
      i64 res = p1 + p2;
      r(d, uns32(res));
      if (res != i32(res)) {
        q(1);
      }
    }
    return nxt();
  }

  inline u32 smulbb() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      bool m_high = (cur >> 6) & 1;
      bool n_high = (cur >> 5) & 1;
      u32 op1 = n_high ? r(n) >> 16 : r(n) & 0xffff;
      u32 op2 = m_high ? r(m) >> 16 : r(m) & 0xffff;
      i32 res = i32(s16(op1)) * i32(s16(op2));
      r(d, uns32(res));
    }
    return nxt();
  }

  inline u32 smull() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 dlo = cur >> 12;
      u8 dhi = cur >> 16;
      bool setflags = (cur >> 20) & 1;
      i64 res = i64(s32(r(n))) * i64(s32(r(m)));
      r(dhi, uns32(res >> 32));
      r(dlo, uns32(res));
      if (setflags) {
        Cpu::n((res >> 63) & 1);
        z(res == 0);
      }
    }
    return nxt();
  }

  inline u32 smulwb() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      bool m_high = (cur >> 6) & 1;
      u32 op2 = m_high ? r(m) >> 16 : r(m) & 0xffff;
      i64 p = i64(s32(r(n))) * i64(s16(op2));
      r(d, uns32(p >> 16));
    }
    return nxt();
  }

  inline u32 smusd() {
    if (cnd()) {
      u8 n = cur;
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      bool m_swap = (cur >> 5) & 1;
      u32 op2 = m_swap ? Arith::ror32(r(m), 16).u() : r(m);
      i32 p1 = i32(s16(r(n))) * i32(s16(op2));
      i32 p2 = i32(s16(r(n) >> 16)) * i32(s16(op2 >> 16));
      i32 res = p1 - p2;
      r(d, uns32(res));
    }
    return nxt();
  }

  inline u32 ssat() {
    if (cnd()) {
      u8 n = cur;
      u8 d = cur >> 12;
      u8 saturate_to = ((cur >> 16) & 0b11111) + 1;
      u8 imm5 = (cur >> 7) & 0b11111;
      u8 sh = (cur >> 5) & 0b10;
      u32 op = Arith::shiftC(r(n), sh, imm5, c()).u();
      auto sat = Arith::ssat32(op, saturate_to);
      r(d, sat.i());
      q(sat.sat());
    }
    return nxt();
  }

  inline u32 ssat16() {
    if (cnd()) {
      u8 n = cur;
      u8 d = cur >> 12;
      u8 saturate_to = ((cur >> 16) & 0b1111) + 1;
      auto r1 = Arith::ssat32(s16(r(n)), saturate_to);
      auto r2 = Arith::ssat32(s16(r(n) >> 16), saturate_to);
      r(d, (r1.u() & 0xffff) | (r2.u() << 16));
      q(r1.sat() or r2.sat());
    }
    return nxt();
  }

  inline u32 usat() {
    if (cnd()) {
      u8 n = cur;
      u8 d = cur >> 12;
      u8 saturate_to = ((cur >> 16) & 0b11111);
      u8 imm5 = (cur >> 7) & 0b11111;
      u8 sh = (cur >> 5) & 0b10;
      u32 op = Arith::shiftC(r(n), sh, imm5, c()).u();
      auto sat = Arith::usat32(s32(op), saturate_to);
      r(d, sat.u());
      q(sat.sat());
    }
    return nxt();
  }

  inline u32 usat16() {
    if (cnd()) {
      u8 n = cur;
      u8 d = cur >> 12;
      u8 saturate_to = ((cur >> 16) & 0b1111);
      auto r1 = Arith::usat32((s16(r(n))), saturate_to);
      auto r2 = Arith::usat32((s16(r(n) >> 16)), saturate_to);
      r(d, (r1.u() & 0xffff) | (r2.u() << 16));
      q(r1.sat() or r2.sat());
    }
    return nxt();
  }

  inline u32 sadd16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s16(r(n))) + i32(s16(r(m)));
      i32 sum2 = i32(s16(r(n) >> 16)) + i32(s16(r(m) >> 16));
      r(d, (uns32(sum1) & 0xffff) | (uns32(sum2) << 16));
      u8 g = 0;
      g |= sum1 >= 0 ? 0b11 : 0;
      g |= sum2 >= 0 ? 0b1100 : 0;
      ge(g);
    }
    return nxt();
  }

  inline u32 uadd16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u16(r(n))) + i32(u16(r(m)));
      i32 sum2 = i32(u16(r(n) >> 16)) + i32(u16(r(m) >> 16));
      r(d, (uns32(sum1) & 0xffff) | (uns32(sum2) << 16));
      u8 g = 0;
      g |= sum1 >= 0x10000 ? 0b11 : 0;
      g |= sum2 >= 0x10000 ? 0b1100 : 0;
      ge(g);
    }
    return nxt();
  }

  inline u32 uqadd16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u16 sum1 = Arith::u16satAdd(u16(r(n)), u16(r(m))).u();
      u16 sum2 = Arith::u16satAdd(u16(r(n) >> 16), u16(r(m) >> 16)).u();
      r(d, (sum1) | (u32(sum2) << 16));
    }
    return nxt();
  }

  inline u32 ssub16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s16(r(n))) - i32(s16(r(m)));
      i32 sum2 = i32(s16(r(n) >> 16)) - i32(s16(r(m) >> 16));
      r(d, (uns32(sum1) & 0xffff) | (uns32(sum2) << 16));
      u8 g = 0;
      g |= sum1 >= 0 ? 0b11 : 0;
      g |= sum2 >= 0 ? 0b1100 : 0;
      ge(g);
    }
    return nxt();
  }

  inline u32 usub16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u16(r(n))) - i32(u16(r(m)));
      i32 sum2 = i32(u16(r(n) >> 16)) - i32(u16(r(m) >> 16));
      r(d, (uns32(sum1) & 0xffff) | (uns32(sum2) << 16));
      u8 g = 0;
      g |= sum1 >= 0 ? 0b11 : 0;
      g |= sum2 >= 0 ? 0b1100 : 0;
      ge(g);
    }
    return nxt();
  }

  inline u32 uqsub16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u16 sum1 = Arith::u16satSub(u16(r(n)), u16(r(m))).u();
      u16 sum2 = Arith::u16satSub(u16(r(n) >> 16), u16(r(m) >> 16)).u();
      r(d, u32(sum1) | (u32(sum2) << 16));
    }
    return nxt();
  }

  inline u32 sadd8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s8(r(n))) + i32(s8(r(m)));
      i32 sum2 = i32(s8(r(n) >> 8)) + i32(s8(r(m) >> 8));
      i32 sum3 = i32(s8(r(n) >> 16)) + i32(s8(r(m) >> 16));
      i32 sum4 = i32(s8(r(n) >> 24)) + i32(s8(r(m) >> 24));
      u32 res = (uns32(sum1) & 0xff) | ((uns32(sum2) & 0xff) << 8) |
                ((uns32(sum3) & 0xff) << 16) | ((uns32(sum4) & 0xff) << 24);
      r(d, res);
      u8 g = 0;
      if (sum1 >= 0)
        g |= 1;
      if (sum2 >= 0)
        g |= 0b10;
      if (sum3 >= 0)
        g |= 0b100;
      if (sum4 >= 0)
        g |= 0b1000;
      ge(g);
    }
    return nxt();
  }

  inline u32 uadd8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u8(r(n))) + i32(u8(r(m)));
      i32 sum2 = i32(u8(r(n) >> 8)) + i32(u8(r(m) >> 8));
      i32 sum3 = i32(u8(r(n) >> 16)) + i32(u8(r(m) >> 16));
      i32 sum4 = i32(u8(r(n) >> 24)) + i32(u8(r(m) >> 24));
      u32 res = (uns32(sum1) & 0xff) | ((uns32(sum2) & 0xff) << 8) |
                ((uns32(sum3) & 0xff) << 16) | ((uns32(sum4) & 0xff) << 24);
      r(d, res);
      u8 g = 0;
      if (sum1 >= 0x100)
        g |= 1;
      if (sum2 >= 0x100)
        g |= 0b10;
      if (sum3 >= 0x100)
        g |= 0b100;
      if (sum4 >= 0x100)
        g |= 0b1000;
      ge(g);
    }
    return nxt();
  }

  inline u32 uqadd8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u8 sum1 = Arith::u8satAdd(u8(r(n)), u8(r(m))).u();
      u8 sum2 = Arith::u8satAdd(u8(r(n) >> 8), u8(r(m) >> 8)).u();
      u8 sum3 = Arith::u8satAdd(u8(r(n) >> 16), u8(r(m) >> 16)).u();
      u8 sum4 = Arith::u8satAdd(u8(r(n) >> 24), u8(r(m) >> 24)).u();
      u32 res = (uns32(sum1) & 0xff) | ((uns32(sum2) & 0xff) << 8) |
                ((uns32(sum3) & 0xff) << 16) | ((uns32(sum4) & 0xff) << 24);
      r(d, res);
    }
    return nxt();
  }

  inline u32 usad8() {
    if (cnd()) {
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      u8 n = cur;
      i32 sum1 = abs32(i32(u8(r(n))) - i32(u8(r(m))));
      i32 sum2 = abs32(i32(u8(r(n) >> 8)) - i32(u8(r(m) >> 8)));
      i32 sum3 = abs32(i32(u8(r(n) >> 16)) - i32(u8(r(m) >> 16)));
      i32 sum4 = abs32(i32(u8(r(n) >> 24)) - i32(u8(r(m) >> 24)));
      u32 res = sum1 + sum2 + sum3 + sum4;
      r(d, res);
    }
    return nxt();
  }

  inline u32 usada8() {
    if (cnd()) {
      u8 m = cur >> 8;
      u8 d = cur >> 16;
      u8 n = cur;
      u8 a = cur >> 12;
      i32 sum1 = abs32(i32(u8(r(n))) - i32(u8(r(m))));
      i32 sum2 = abs32(i32(u8(r(n) >> 8)) - i32(u8(r(m) >> 8)));
      i32 sum3 = abs32(i32(u8(r(n) >> 16)) - i32(u8(r(m) >> 16)));
      i32 sum4 = abs32(i32(u8(r(n) >> 24)) - i32(u8(r(m) >> 24)));
      u32 res = sum1 + sum2 + sum3 + sum4;
      r(d, res + r(a));
    }
    return nxt();
  }

  inline u32 ssub8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s8(r(n))) - i32(s8(r(m)));
      i32 sum2 = i32(s8(r(n) >> 8)) - i32(s8(r(m) >> 8));
      i32 sum3 = i32(s8(r(n) >> 16)) - i32(s8(r(m) >> 16));
      i32 sum4 = i32(s8(r(n) >> 24)) - i32(s8(r(m) >> 24));
      u32 res = (uns32(sum1) & 0xff) | ((uns32(sum2) & 0xff) << 8) |
                ((uns32(sum3) & 0xff) << 16) | ((uns32(sum4) & 0xff) << 24);
      r(d, res);
      u8 g = 0;
      if (sum1 >= 0)
        g |= 1;
      if (sum2 >= 0)
        g |= 0b10;
      if (sum3 >= 0)
        g |= 0b100;
      if (sum4 >= 0)
        g |= 0b1000;
      ge(g);
    }
    return nxt();
  }

  inline u32 usub8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u8(r(n))) - i32(u8(r(m)));
      i32 sum2 = i32(u8(r(n) >> 8)) - i32(u8(r(m) >> 8));
      i32 sum3 = i32(u8(r(n) >> 16)) - i32(u8(r(m) >> 16));
      i32 sum4 = i32(u8(r(n) >> 24)) - i32(u8(r(m) >> 24));
      u32 res = (uns32(sum1) & 0xff) | ((uns32(sum2) & 0xff) << 8) |
                ((uns32(sum3) & 0xff) << 16) | ((uns32(sum4) & 0xff) << 24);
      r(d, res);
      u8 g = 0;
      if (sum1 >= 0)
        g |= 1;
      if (sum2 >= 0)
        g |= 0b10;
      if (sum3 >= 0)
        g |= 0b100;
      if (sum4 >= 0)
        g |= 0b1000;
      ge(g);
    }
    return nxt();
  }

  inline u32 uqsub8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u8 sum1 = Arith::u8satSub(u8(r(n)), u8(r(m))).u();
      u8 sum2 = Arith::u8satSub(u8(r(n) >> 8), u8(r(m) >> 8)).u();
      u8 sum3 = Arith::u8satSub(u8(r(n) >> 16), u8(r(m) >> 16)).u();
      u8 sum4 = Arith::u8satSub(u8(r(n) >> 24), u8(r(m) >> 24)).u();
      u32 res =
          u32(sum1) | (u32(sum2) << 8) | (u32(sum3) << 16) | (u32(sum4) << 24);
      r(d, res);
    }
    return nxt();
  }

  inline u32 sasx() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 dif = i32(s16(r(n))) - i32(s16(r(m) >> 16));
      i32 sum = i32(s16(r(n) >> 16)) + i32(s16(r(m)));
      u32 res = (uns32(dif) & 0xffff) | (uns32(sum) << 16);
      r(d, res);
      u8 g = 0;
      if (dif >= 0)
        g |= 0b11;
      if (sum >= 0)
        g |= 0b1100;
      ge(g);
    }
    return nxt();
  }

  inline u32 uasx() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 dif = i32(u16(r(n))) - i32(u16(r(m) >> 16));
      i32 sum = i32(u16(r(n) >> 16)) + i32(u16(r(m)));
      u32 res = (uns32(dif) & 0xffff) | (uns32(sum) << 16);
      r(d, res);
      u8 g = 0;
      if (dif >= 0)
        g |= 0b11;
      if (sum >= 0x10000)
        g |= 0b1100;
      ge(g);
    }
    return nxt();
  }

  inline u32 uqasx() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u16 dif = Arith::u16satSub(u16(r(n)), u16(r(m) >> 16)).u();
      u16 sum = Arith::u16satAdd(u16(r(n) >> 16), u16(r(m))).u();
      u32 res = (u32(dif)) | (u32(sum) << 16);
      r(d, res);
    }
    return nxt();
  }

  inline u32 sel() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u32 res = 0;
      u8 g = ge();
      res |= (g & 1 ? r(n) : r(m)) & 0xff;
      res |= (g & 0b10 ? r(n) : r(m)) & 0xff00;
      res |= (g & 0b100 ? r(n) : r(m)) & 0xff0000;
      res |= (g & 0b1000 ? r(n) : r(m)) & 0xff000000;
      r(d, res);
    }
    return nxt();
  }

  inline u32 shadd16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s16(r(n))) + i32(s16(r(m)));
      i32 sum2 = i32(s16(r(n) >> 16)) + i32(s16(r(m) >> 16));
      r(d,
        ((uns32(sum1) >> 1) & 0xffff) | (((uns32(sum2) >> 1) & 0xffff) << 16));
    }
    return nxt();
  }

  inline u32 uhadd16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u16(r(n))) + i32(u16(r(m)));
      i32 sum2 = i32(u16(r(n) >> 16)) + i32(u16(r(m) >> 16));
      r(d,
        ((uns32(sum1) >> 1) & 0xffff) | (((uns32(sum2) >> 1) & 0xffff) << 16));
    }
    return nxt();
  }

  inline u32 shsub16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s16(r(n))) - i32(s16(r(m)));
      i32 sum2 = i32(s16(r(n) >> 16)) - i32(s16(r(m) >> 16));
      r(d,
        ((uns32(sum1) >> 1) & 0xffff) | (((uns32(sum2) >> 1) & 0xffff) << 16));
    }
    return nxt();
  }

  inline u32 uhsub16() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u16(r(n))) - i32(u16(r(m)));
      i32 sum2 = i32(u16(r(n) >> 16)) - i32(u16(r(m) >> 16));
      r(d,
        ((uns32(sum1) >> 1) & 0xffff) | (((uns32(sum2) >> 1) & 0xffff) << 16));
    }
    return nxt();
  }

  inline u32 shadd8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s8(r(n))) + i32(s8(r(m)));
      i32 sum2 = i32(s8(r(n) >> 8)) + i32(s8(r(m) >> 8));
      i32 sum3 = i32(s8(r(n) >> 16)) + i32(s8(r(m) >> 16));
      i32 sum4 = i32(s8(r(n) >> 24)) + i32(s8(r(m) >> 24));
      u32 res = ((uns32(sum1) >> 1) & 0xff) |
                (((uns32(sum2) >> 1) & 0xff) << 8) |
                (((uns32(sum3) >> 1) & 0xff) << 16) |
                (((uns32(sum4) >> 1) & 0xff) << 24);
      r(d, res);
    }
    return nxt();
  }

  inline u32 uhadd8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u8(r(n))) + i32(u8(r(m)));
      i32 sum2 = i32(u8(r(n) >> 8)) + i32(u8(r(m) >> 8));
      i32 sum3 = i32(u8(r(n) >> 16)) + i32(u8(r(m) >> 16));
      i32 sum4 = i32(u8(r(n) >> 24)) + i32(u8(r(m) >> 24));
      u32 res = ((uns32(sum1) >> 1) & 0xff) |
                (((uns32(sum2) >> 1) & 0xff) << 8) |
                (((uns32(sum3) >> 1) & 0xff) << 16) |
                (((uns32(sum4) >> 1) & 0xff) << 24);
      r(d, res);
    }
    return nxt();
  }

  inline u32 shsub8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(s8(r(n))) - i32(s8(r(m)));
      i32 sum2 = i32(s8(r(n) >> 8)) - i32(s8(r(m) >> 8));
      i32 sum3 = i32(s8(r(n) >> 16)) - i32(s8(r(m) >> 16));
      i32 sum4 = i32(s8(r(n) >> 24)) - i32(s8(r(m) >> 24));
      u32 res = ((uns32(sum1) >> 1) & 0xff) |
                (((uns32(sum2) >> 1) & 0xff) << 8) |
                (((uns32(sum3) >> 1) & 0xff) << 16) |
                (((uns32(sum4) >> 1) & 0xff) << 24);
      r(d, res);
    }
    return nxt();
  }

  inline u32 uhsub8() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum1 = i32(u8(r(n))) - i32(u8(r(m)));
      i32 sum2 = i32(u8(r(n) >> 8)) - i32(u8(r(m) >> 8));
      i32 sum3 = i32(u8(r(n) >> 16)) - i32(u8(r(m) >> 16));
      i32 sum4 = i32(u8(r(n) >> 24)) - i32(u8(r(m) >> 24));
      u32 res = ((uns32(sum1) >> 1) & 0xff) |
                (((uns32(sum2) >> 1) & 0xff) << 8) |
                (((uns32(sum3) >> 1) & 0xff) << 16) |
                (((uns32(sum4) >> 1) & 0xff) << 24);
      r(d, res);
    }
    return nxt();
  }

  inline u32 shasx() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 dif = i32(s16(r(n))) - i32(s16(r(m) >> 16));
      i32 sum = i32(s16(r(n) >> 16)) + i32(s16(r(m)));
      u32 res = ((uns32(dif) >> 1) & 0xffff) | ((uns32(sum) >> 1) << 16);
      r(d, res);
    }
    return nxt();
  }

  inline u32 uhasx() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 dif = i32(u16(r(n))) - i32(u16(r(m) >> 16));
      i32 sum = i32(u16(r(n) >> 16)) + i32(u16(r(m)));
      u32 res = ((uns32(dif) >> 1) & 0xffff) | ((uns32(sum) >> 1) << 16);
      r(d, res);
    }
    return nxt();
  }

  inline u32 ssax() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum = i32(s16(r(n))) + i32(s16(r(m) >> 16));
      i32 diff = i32(s16(r(n) >> 16)) - i32(s16(r(m)));
      u32 res = (uns32(sum) & 0xffff) | (uns32(diff) << 16);
      r(d, res);
      u8 g = 0;
      if (sum >= 0)
        g |= 0b11;
      if (diff >= 0)
        g |= 0b1100;
      ge(g);
    }
    return nxt();
  }

  inline u32 usax() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u32 sum = u32(u16(r(n))) + u32(u16(r(m) >> 16));
      i32 diff = i32(u16(r(n) >> 16)) - i32(u16(r(m)));
      u32 res = (sum & 0xffff) | (u32(diff) << 16);
      r(d, res);
      u8 g = 0;
      if (sum >= 0x10000)
        g |= 0b11;
      if (diff >= 0)
        g |= 0b1100;
      ge(g);
    }
    return nxt();
  }

  inline u32 uqsax() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      u16 sum = Arith::u16satAdd(u16(r(n)), u16(r(m) >> 16)).u();
      u16 diff = Arith::u16satSub(u16(r(n) >> 16), u16(r(m))).u();
      u32 res = u32(sum) | (u32(diff) << 16);
      r(d, res);
    }
    return nxt();
  }

  inline u32 shsax() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum = i32(s16(r(n))) + i32(s16(r(m) >> 16));
      i32 diff = i32(s16(r(n) >> 16)) - i32(s16(r(m)));
      u32 res = ((uns32(sum) >> 1) & 0xffff) | ((uns32(diff) >> 1) << 16);
      r(d, res);
      u8 g = 0;
      if (sum >= 0)
        g |= 0b11;
      if (diff >= 0)
        g |= 0b1100;
      ge(g);
    }
    return nxt();
  }

  inline u32 uhsax() {
    if (cnd()) {
      u8 m = cur;
      u8 d = cur >> 12;
      u8 n = cur >> 16;
      i32 sum = i32(u16(r(n))) + i32(u16(r(m) >> 16));
      i32 diff = i32(u16(r(n) >> 16)) - i32(u16(r(m)));
      u32 res = ((uns32(sum) >> 1) & 0xffff) | ((uns32(diff) >> 1) << 16);
      r(d, res);
    }
    return nxt();
  }

  static void test();
};
