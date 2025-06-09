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

  static Res lsl32(u32 v, u8 n) {
    // remove TODO
    assert(n > 0);
    constexpr u32 mask = u32(1) << 31;
    v <<= n - 1;
    Res res;
    res.c = (v & mask) > 0;
    res.v.u = v << 1;
    return res;
  }

  static Res lsr32(u32 v, u8 n) {
    assert(n > 0);
    v >>= n - 1;
    Res res;
    res.c = v & 1;
    res.v.u = v >> 1;
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
    assert(n > 0);
    Res i = {.v = {.u = v}, .c = false};
    i.v.i >>= n - 1;
    Res res;
    i.c = i.v.i & 1;
    res.v.i = i.v.i >> 1;
    return res;
  }

  static Res ror32(u32 v, u8 n) {
    u8 m = n % 32;
    u32 rot = lsr32(v, m).v.u | lsl32(v, 32 - n).v.u;
    return Res{.v = {.u = rot}, .c = (rot & (1 << 31)) > 0};
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
  }

  static void asrTest() {
    assert((asr32(128, 1).v.u) == 0b1000000);
    assert(asr32(1 << 31, 3).v.u == 0xf0000000);
  }

  static void lsrTest() {
    assert(lsr32(1, 1).c);
    assert(lsr32(1, 1).v.u == 0);
  }

  static void lslTest() {
    // assert(lsl<u8>(0, 0).v == 0);
    assert(lsl32(0xff, 1).v.u == (0xff) << 1);
    assert(lsl32(1, 32).v.u == 0);
    assert(lsl32(1, 7).v.u == (1 << 7));
    assert(lsl32(0xffffffff, 1).c == true);
    assert(lsl32(1, 32).c == true);
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
