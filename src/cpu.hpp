#pragma once
#include "stuff.hpp"
#include <bit>
#include <cassert>
#include <cstdint>
#include <string>
#include <system_error>
#include <type_traits>

struct Cpu {
  struct Res {
    union {
      i32 i;
      u32 u;
    } v;
    bool c;
  };

  static inline  Res lsl32(u32 v, u8 n) {
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

  static Adc adc(u32 a, u32 b, bool carry) {
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

  static Res asr32(u32 v, u8 n) {
    Res res = {.v = {.u = v}, .c = false};
    asm volatile("movb %[a], %%cl\n"
                 "sarl %%cl, %[b]\n"
                 "setc %[c]\n"
                 : [b] "+r"(res.v.u), [c] "=r"(res.c)
                 : [a] "r"(n));
    return res;
  }

  static Res ror32(u32 v, u8 n) {    
    Res res = {.v = {.u = v}, .c = false};
    asm volatile("movb %[a], %%cl\n"
                 "rorl %%cl, %[b]\n"
                 "setc %[c]\n"
                 : [b] "+r"(res.v.u), [c] "=r"(res.c)
                 : [a] "r"(n));
    return res;
  }

  static Res rrx32(u32 v, u8 carry_in) {
    u32 i = v >> 1;
    if (carry_in)
      i |= u32(1) << 31;
    return Res{.v = {.u = i}, .c = (v & 1) > 0};
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
    assert(ror32(1,1).c);
    assert(!ror32(2, 1).c);
  }

  static void asrTest() {
    assert((asr32(128, 1).v.u) == 0b1000000);
    assert(asr32(1 << 31, 3).v.u == 0xf0000000);
    assert(asr32(1,1).c);
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
  }
};
