#pragma once
#include "arith.hpp"
#include "assert.h"
#include "decoder.hpp"
#include "mem.hpp"
#include "stuff.hpp"
#include <cmath>
#include <iosfwd>
#include <string>

struct Cpu {

  union APSR {
    struct {
      u32 _1 : 16;
      u32 ge : 3;
      u32 _2 : 8;
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
      auto sat = Arith::signedSat32(r(m) + r(n), 32);
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
      assert(false);
    }
    return nxt();
  }

  static void test();
};
