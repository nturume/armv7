
#pragma once
#include "mem.hpp"
#include "stuff.hpp"
#include <cassert>
#include <cstdio>

struct SP804 {
  struct Timer {
    union Ctl {
      u32 back = 0;

      bool enabled() { return (back >> 7) & 1; }
      bool periodic() { return (back >> 6) & 1; }

      bool intEnable() { return (back >> 5) & 1; }

      u8 prescale() { return (back >> 2) & 0b11; }

      u8 is32bit() { return (back >> 1) & 1; }

      u8 oneShot() { return (back) & 1; }
    };

    Ctl __ctl{};
    u32 __load;
    u32 cur = ~u32(0);

    void load(u32 value) {
      __load = value;
      cur = value;
      // printf("LOADED %x __load = %x\n",value, __load);
    }

    u32 load() { return __load; }

    u32 ctlb() { return __ctl.back; }

    void ctlb(u32 value) {
      __ctl.back = value;
      // printf("value %x load %x cur %x enabled %x\n",value, __load, cur,
      // __ctl.enabled());
      assert(__ctl.prescale() == 0);
      assert(__ctl.is32bit());
    }

    bool tick() {
      cur -= 1;
      if (cur == 0) {
        if (__ctl.periodic()) {
          cur = load();
        } else {
          cur = ~u32(0);
        }
        return __ctl.enabled();
      }
      return false;
    }
  };

  Timer one;
  Timer two;

  bool tick1() {
    return one.tick();
  }

  bool tick2() {
    return two.tick();
  }

  static u32 read(u32 addr, u8 width, SP804 *ctx) {
    // assert(false);
    u32 offt = addr & 0xfff;
    switch (offt) {
    case 8:
      return ctx->one.ctlb();
    case 4:
      return ctx->one.cur;
    default:
      printf("Unimplemented sp804 reg %x\n", offt);
      assert(false);
    }
  }
  static void write(u32 addr, u32 value, u8 width, SP804 *ctx) {
    // assert(false);
    u32 offt = addr & 0xfff;
    switch (offt) {
    case 0:
      return ctx->one.load(value);
    case 4: // read only reg
      return;
    case 8:
      return ctx->one.ctlb(value);
    default:
      printf("Unimplemented sp804 reg %x\n", offt);
      assert(false);
    }
  }

  Region getRegion(u32 start) {
    return Region{
        .start = start,
        .len = 0x1000,
        .isram = false,
        .ctx = this,
        .r = (u32(*)(u32, u8, void *)) & read,
        .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
