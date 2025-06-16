#pragma once
#include "stuff.hpp"
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>
#include <system_error>
#include <type_traits>

namespace Arith {
struct Res {
  union {
    i32 i;
    u32 u;
  } v;
  bool c;

  inline u32 u() { return v.u; }

  inline i32 i() { return v.i; }
  inline bool n() { return (v.u & 0x80000000) > 0; }

  inline bool z() { return v.u == 0; }

  inline bool sat() { return c; }
};

static inline Res lsl32(u32 v, u8 n) {
  Res res{.v = {.u = v}, .c = false};
  if (n > 32) {
    res.v.u = 0;
    return res;
  }
  u64 w = u64(v)<<n;
  res.v.u = u32(w);
  res.c = (w&(1ul<<32))>0;
  return res;
}

static inline Res lsr32(u32 v, u8 n) {
  Res res = {.v = {.u = v}, .c = false};
  if (n > 32) {
    res.v.u = 0;
    return res;
  }
  u64 w = (u64(v)<<1)>>n;
  res.v.u = u32(w>>1);
  res.c = (w&1);
  return res;
}

struct Adc {
  u32 r;
  bool f;
  bool c;

  inline bool n() { return (r & 0x80000000) > 0; }

  inline bool z() { return r == 0; }
};

static inline Adc adc(u32 a, u32 b, bool carry) {
  Adc res = {.r = 0, .f = false, .c = false};
  asm volatile("btl $0, %[e]\n"
               "adcl %[a], %[b]\n"
               "seto %[c]\n"
               "setc %[d]\n"
               : [b] "+r"(b), [c] "=m"(res.f), [d] "=m"(res.c)
               : [a] "r"(a), [e] "r"((u32)carry));
  res.r = b;
  return res;
};

static inline Res asr32(u32 v, u8 n) {
  Res res = {.v = {.u = v}, .c = false};
  if (n > 32) {
    res.v.u = 0;
    return res;
  }
  i32 w = (res.i()>>(n-1));
  res.c = (w&1);
  res.v.i = w>>1;
  return res;
}

static inline Res ror32(u32 v, u8 n) {
  Res res = {.v = {.u = v}, .c = false};
  asm volatile("movb %[a], %%cl\n"
               "rorl %%cl, %[b]\n"
               : [b] "+r"(res.v.u), [c] "=r"(res.c)
               : [a] "r"(n));
  res.c = (res.n());
  return res;
}

static inline Res rrx32(u32 v, u8 carry_in) {
  u32 i = v >> 1;
  if (carry_in)
    i |= u32(1) << 31;
  return Res{.v = {.u = i}, .c = (v & 1) > 0};
}

static inline Res signedSat32(u32 v, u32 n) {
  Res r = Res{.v = {.u = v}, .c = false};
  i64 p = i64(1) << (n - 1);
  i64 integer = i64(r.i());
  if (integer > (p - 1)) {
    r.c = true;
    r.v.i = p - 1;
  } else if (integer < -p) {
    r.c = true;
    r.v.i = -p;
  }
  return r;
}

static inline Res unsignedSat32(i32 v, u32 n) {
  Res r = Res{.v = {.i = v}, .c = false};
  i64 p = u64(1) << n;
  i64 integer = r.v.i;
  if (integer > p - 1) {
    r.c = true;
    r.v.i = p - 1;
  } else if (integer < 0) {
    r.c = true;
    r.v.u = 0;
  }
  return r;
}

inline Res usat32(u32 v, u32 n) { return unsignedSat32(v, n); }

inline Res ssat32(i32 v, u32 n) { return signedSat32(v, n); }

static inline Res sat32(u32 v, u32 n, bool sign) {
  if (sign) {
    Res r = {.v = {.u = v}, .c = 0};
    return signedSat32(r.v.i, n);
  }
  return unsignedSat32(v, n);
}

static inline Res s32satAdd(u32 a, u32 b) {
  u32 res = a + b;
  bool saturated = false;
  if (((res ^ a) & 0x80000000) && !((a ^ b) & 0x80000000)) {
    if (a & 0x80000000) {
      res = 0x80000000;
    } else {
      res = 0x7fffffff;
    }
    saturated = true;
  }
  return {.v = {.u = res}, .c = saturated};
}

static inline Res s32satSub(u32 a, u32 b) {
  u32 res = a - b;
  bool saturated = false;
  if (((res ^ a) & 0x80000000) && ((a ^ b) & 0x80000000)) {
    if (a & 0x80000000) {
      res = 0x80000000;
    } else {
      res = 0x7fffffff;
    }
    saturated = true;
  }
  return {.v = {.u = res}, .c = saturated};
}

static inline Res s16satAdd(u16 a, u16 b) {
  u32 res = a + b;
  bool saturated = false;
  if (((res ^ a) & 0x8000) && !((a ^ b) & 0x8000)) {
    if (a & 0x8000) {
      res = 0x8000;
    } else {
      res = 0x7fff;
    }
    saturated = true;
  }
  return {.v = {.u = res}, .c = saturated};
}

static inline Res s16satSub(u16 a, u16 b) {
  u32 res = a - b;
  bool saturated = false;
  if (((res ^ a) & 0x8000) && ((a ^ b) & 0x8000)) {
    if (a & 0x8000) {
      res = 0x8000;
    } else {
      res = 0x7fff;
    }
    saturated = true;
  }
  return {.v = {.u = res}, .c = saturated};
}

static inline Res s8satAdd(u8 a, u8 b) {
  u32 res = a + b;
  bool saturated = false;
  if (((res ^ a) & 0x80) && !((a ^ b) & 0x80)) {
    if (a & 0x80) {
      res = 0x80;
    } else {
      res = 0x7f;
    }
    saturated = true;
  }
  return {.v = {.u = res}, .c = saturated};
}

static inline Res s8satSub(u8 a, u8 b) {
  u32 res = a - b;
  bool saturated = false;
  if (((res ^ a) & 0x80) && ((a ^ b) & 0x80)) {
    if (a & 0x80) {
      res = 0x80;
    } else {
      res = 0x7f;
    }
    saturated = true;
  }
  return {.v = {.u = res}, .c = saturated};
}

static inline Res u16satAdd(u16 a, u16 b) {
  u16 res = a + b;
  bool saturated = false;
  if (res < a) {
    saturated = true;
    res = 0xffff;
  }
  return Res{.v = {.u = res}, .c = saturated };
}

static inline Res u16satSub(u16 a, u16 b) {
  u16 res = 0;
  bool saturated = false;
  if (a>b) {
    res = a-b;
  } else {
    saturated = true;
  }
  return Res{.v = {.u = res}, .c = saturated };
}

static inline Res u8satSub(u8 a, u8 b) {
  u8 res = 0;
  bool saturated = false;
  if (a>b) {
    res = a-b;
  } else {
    saturated = true;
  }
  return Res{.v = {.u = res}, .c = saturated };
}

static inline Res u8satAdd(u8 a, u8 b) {
  u8 res = a + b;
  bool saturated = false;
  if (res < a) {
    saturated = true;
    res = 0xff;
  }
  return Res{.v = {.u = res}, .c = saturated };
}

/// type must be pre masked
static Res shiftC(u32 value, u8 type, u8 amount, bool carry_in) {
  if (amount == 0)
    return Res{.v = {.u = value}, .c = carry_in};
  switch (type) {
  case 0:
    return lsl32(value, amount);
  case 1:
    return lsr32(value, amount);
  case 2:
    return asr32(value, amount);
  case 3:
    return ror32(value, amount);
  case 4:
    return rrx32(value, carry_in);
  }
  // unreachable
  assert(false);
}

struct Is {
  u8 t;
  u8 n;
};
static inline Is decodeImmShift(u8 type, u8 imm5) {
  Is i;
  switch (type & 0b11) {
  case 0: {
    i.t = 0;
    i.n = (imm5 & 0b11111);
    break;
  }
  case 1: {
    i.t = 1;
    i.n = (imm5 & 0b11111) == 0 ? 32 : (imm5 & 0b11111);
    break;
  }
  case 2: {
    i.t = 2;
    i.n = (imm5 & 0b11111) == 0 ? 32 : (imm5 & 0b11111);
    break;
  }
  case 3: {
    if ((imm5 & 0b11111) == 0) {
      i.t = 4;
      i.n = 1;
    } else {
      i.t = 3;
      i.n = (imm5 & 0b11111);
    }
    break;
  }
  }
  return i;
}

static inline Res expandImmC(u16 imm12) {
  // just rotate direct
  // return shiftC(imm12&0xff, 3, 2*((imm12>>8)&0xf), in);
  return ror32(imm12 & 0xff, 2 * ((imm12 >> 8) & 0xf));
}

#ifdef TESTING
static void usatTest() {
  assert(unsignedSat32(0xff, 8).c == false);
  assert(unsignedSat32(256, 8).c);
  assert(unsignedSat32(256, 8).v.u == 255);
  assert(sat32(256, 8, false).v.u == 255);
}

static void ssatTest() {
  assert(signedSat32(3, 1).c);
  assert(signedSat32(3, 1).v.i == 0);
  assert(signedSat32(0xffffffff, 32).c == false);
  assert(signedSat32(129, 8).v.u == 127);
  assert(signedSat32(0xff, 8).v.u == 127);
  assert(sat32(0xff, 8, true).v.u == 127);
  assert(signedSat32(-150, 8).v.i == -128);
}

static void adcTest() {
  assert(adc(1, 2, false).r == 3);
  assert(adc(INT32_MAX, 1, false).f);
  assert(adc(INT32_MAX, 0, true).f);

  assert(adc(INT32_MAX, 1, true).f);
  assert(adc(UINT32_MAX, 1, true).c);

  assert(adc(UINT32_MAX, 0, true).c);
  assert(adc(UINT32_MAX, 1, false).c);
}

static void rrxTest() {
  assert(rrx32(0, 1).v.u == u32(1) << 31);
  assert(rrx32(1, 1).c);
}

static void rorTest() {
  assert(ror32(1, 1).v.u == u32(1 << 31));
  assert(ror32(3, 1).v.u == (u32(1 << 31) | 1));
  assert(ror32(1, 1).c);
  assert(!ror32(2, 1).c);
}

static void asrTest() {
  assert((asr32(128, 1).v.u) == 0b1000000);
  assert(asr32(1 << 31, 3).v.u == 0xf0000000);
  assert(asr32(1, 1).c);
}

static void lsrTest() {
  assert(lsr32(1, 1).c);
  assert(lsr32(1, 1).v.u == 0);
}

static void lslTest() {
  assert(lsl32(0xff, 1).v.u == (0xff) << 1);
  assert(lsl32(1, 3).v.u == 0b1000);
  assert(lsl32(1, 7).v.u == (1 << 7));
  assert(lsl32(0xffffffff, 1).c == true);
  assert(lsl32(0b11, 31).c == true);
}

static void test() {
  lslTest();
  lsrTest();
  asrTest();
  rorTest();
  rrxTest();
  adcTest();
  ssatTest();
  usatTest();
}
#endif
}; // namespace Arith
