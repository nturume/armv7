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
};

static inline Res lsl32(u32 v, u8 n) {
  Res res{.v = {.u = v}, .c = false};
  asm volatile("movb %[a], %%cl\n"
               "shll %%cl, %[b]\n"
               "setc %[c]\n"
               : [b] "+r"(res.v.u), [c] "=r"(res.c)
               : [a] "r"(n));
  return res;
}

static inline Res lsr32(u32 v, u8 n) {
  Res res = {.v = {.u = v}, .c = false};
  asm volatile("movb %[a], %%cl\n"
               "shrl %%cl, %[b]\n"
               "setc %[c]\n"
               : [b] "+r"(res.v.u), [c] "=r"(res.c)
               : [a] "r"(n));
  return res;
}

struct Adc {
  u32 res;
  bool of;
  bool c;
};

static inline Adc adc(u32 a, u32 b, bool carry) {
  Adc res = {.res = 0, .of = false, .c = false};
  asm volatile("btl $0, %[e]\n"
               "adcl %[a], %[b]\n"
               "seto %[c]\n"
               "setc %[d]\n"
               : [b] "+r"(b), [c] "=m"(res.of), [d] "=m"(res.c)
               : [a] "r"(a), [e] "r"((u32)carry));
  res.res = b;
  return res;
};

static inline Res asr32(u32 v, u8 n) {
  Res res = {.v = {.u = v}, .c = false};
  asm volatile("movb %[a], %%cl\n"
               "sarl %%cl, %[b]\n"
               "setc %[c]\n"
               : [b] "+r"(res.v.u), [c] "=r"(res.c)
               : [a] "r"(n));
  return res;
}

static inline Res ror32(u32 v, u8 n) {
  Res res = {.v = {.u = v}, .c = false};
  asm volatile("movb %[a], %%cl\n"
               "rorl %%cl, %[b]\n"
               "setc %[c]\n"
               : [b] "+r"(res.v.u), [c] "=r"(res.c)
               : [a] "r"(n));
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
  i32 p = (std::pow(int(2), n - 1));
  if (r.v.i > p - 1) {
    r.c = true;
    r.v.i = p - 1;
  } else if (r.v.i < -p) {
    r.c = true;
    r.v.i = -p;
  }
  return r;
}

static inline Res unsignedSat32(u32 v, u32 n) {
  Res r = Res{.v = {.u = v}, .c = false};
  u32 p = (std::pow(u32(2), n));
  if (r.v.u > p - 1) {
    r.c = true;
    r.v.i = p - 1;
  }
  // else if(r.v.u < 0) {
  //   // unreachable
  //   r.c = true;
  //   r.v.u = 0;
  // }
  return r;
}

static inline Res sat32(u32 v, u32 n, bool sign) {
  if (sign) {
    Res r = {.v = {.u = v}, .c = 0};
    return signedSat32(r.v.i, n);
  }
  return unsignedSat32(v, n);
}

static Res shiftC(u32 value, u8 type, u8 amount, bool carry_in) {
  if (amount == 0)
    return Res{.v = {.u = value}, .c = carry_in};
  switch (type & 0b11) {
  case 0:
    return lsl32(value, amount);
  case 1:
    return lsr32(value, amount);
  case 2:
    return asr32(value, amount);
  case 3:
    return ror32(value, amount);
  }
  // unreachable
  return Res{};
}

struct ImmShift {
  u8 type;
  u8 amount;
};
static ImmShift decodeImmShift(u8 type, u8 imm5) {
  ImmShift i;
  switch (type & 0b11) {
  case 0: {
    i.type = 0;
    break;
  }
  case 1: {
    i.type = 1;
    i.amount = (imm5 & 0b11111) == 0 ? 32 : 0;
    break;
  }
  case 2: {
    i.type = 2;
    i.amount = (imm5 & 0b11111) == 0 ? 32 : 0;
    break;
  }
  case 3: {
    if ((imm5 & 0b11111) == 0) {
      i.type = 4;
      i.amount = 1;
    } else {
      i.type = 3;
      i.amount = imm5;
    }
    break;
  }
  }
  return i;
}

static inline Res expandImmC(u16 imm12, bool in) {
   return shiftC(imm12&0xff, 3, 2*((imm12>>8)&0xf), in);
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
  assert(signedSat32(0xffffffff, 32).c);
  assert(signedSat32(129, 8).v.u == 127);
  assert(signedSat32(0xff, 8).v.u == 127);
  assert(sat32(0xff, 8, true).v.u == 127);
}

static void adcTest() {
  assert(adc(1, 2, false).res == 3);
  assert(adc(INT32_MAX, 1, false).of);
  assert(adc(INT32_MAX, 0, true).of);

  assert(adc(INT32_MAX, 1, true).of);
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
