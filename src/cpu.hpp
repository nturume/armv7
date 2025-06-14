#pragma once
#include "arith.hpp"
#include "assert.h"
#include "decoder.hpp"
#include "mem.hpp"
#include "stuff.hpp"
#include <cmath>
#include <functional>
#include <iosfwd>
#include <string>

struct Cpu {

  union APSR {
    struct {
      u32 _1 : 16;
      u32 ge : 4;
      u32 _2 : 7;
      u32 q : 1;
      u32 v : 1;
      u32 c : 1;
      u32 z : 1;
      u32 n : 1;
    } b;
    u32 back = 0;
  };

  APSR apsr;
  u32 regs[16] = {0};
  Memory<64> mem;

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
      result = apsr.b.n and apsr.b.v;
      break;
    case 0b110:
      result = apsr.b.n and !apsr.b.z;
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

  inline u32 r(u8 pos) {
    if ((pos & 0xf) == 15)
      return regs[15] + 8;
    return regs[pos & 0xf];
  }

  inline i32 rs(u8 pos) {
    union {
      u32 u;
      i32 i;
    } x = {.u = r(pos)};
    return x.i;
  }

  inline void r(u8 pos, u32 value) { regs[pos & 0xf] = value; }

  inline u8 currentInstrSet() { return 0; }

  inline u32 pcStoreValue() { return regs[15] + 8; }

  inline void branchTo(u32 ptr) { regs[15] = ptr; }

  inline void branchWritePc(u32 ptr) { branchTo(ptr & (0xffffffff << 2)); }

  inline u32 sp() { return regs[13]; }

  inline void sp(u32 v) { regs[13] = v; }

  inline u32 rl() { return regs[14]; }

  inline void rl(u32 v) { regs[14] = v; }

  inline u32 pc() { return regs[15] + 8; }

  inline u32 pcReal() { return regs[15]; }

  inline void bxWritePc(u32 ptr) {
    if ((ptr & 0b11) == 0) {
      return branchTo(ptr);
    }

    assert(true ? false : "bad address" == nullptr);
  }

  inline u32 aluWritePc(u32 ptr) {
    bxWritePc(ptr);
    return regs[15];
  }

  inline void loadWritePc(u32 ptr) { return bxWritePc(ptr); }

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

  inline u32 nxt() { return r(15) + 4; }

  static constexpr u32 bm(u8 n) { return u32(0xffffffff) >> (32 - n); }

  inline u8 decodeRegShift(u8 type) { return type & 0b11; }

  inline bool curModeIsUsrSys() {
    // TODO
    return true;
  }

  inline bool curModeIsntUser() {
    // TODO
    return false;
  }

  bool intZeroDivTrapping() {
    // TODO
    return false;
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
      u32 res = ~expandImm(u16(cur));
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
      r(d, res | r(d));
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
      x = s32(x) >> lsbit;
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
      u32 res = (u16(r(n)) + u16(u8(rotated.u()))) |
                (u32((u16(r(n) >> 16)) + u16(u8(rotated.u() >> 16))) << 16);
      r(d, res);
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
