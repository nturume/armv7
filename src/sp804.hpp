
#pragma once
#include "mem.hpp"
#include "stuff.hpp"
#include <cassert>
#include <cstdio>

struct SP804 {
  struct Timer {
    union Ctl {
      u32 back = 0;

      struct __packed {
        u32 oneshot : 1; // vs wrapping
        u32 timersize32 : 1;
        u32 pre : 2;
        u32 _1 : 1;
        u32 int_enable : 1;
        u32 periodic : 1;
        u32 en : 1;
      } b;

      // void set(u8 n) {
      //   back |= (1u<<n);
      // }

      // void clear(u8 n) {
      //   back &= ~(1u<<n);
      // }

      // bool get(u8 n) {
      //   return (back>>n)&1;
      // }
    };

    Ctl __ctl{};
    u32 __load;
    u32 cur = ~u32(0);
    bool shotdone = false;

    void reset() {
      // state after reset:
      //  • the counter is disabled
      //  • free-running mode is selected
      //  • 16-bit counter mode is selected
      //  • prescalers are set to divide by 1
      //  • interrupts are cleared but enabled
      //  • the Load Register is set to zero
      //  • the counter Value is set to 0xFFFFFFFF.
      __ctl.back = 0x02;
    }

    void load(u32 value) {
      __load = value;
      cur = value;
      shotdone = false;
      // printf("LOADED %x __load = %x\n",value, __load);
    }

    u32 load() { return __ctl.b.timersize32 ? __load : __load & 0xffff; }

    inline u32 ctlb() { return __ctl.back; }

    inline void ctlb(u32 value) {
      __ctl.back = value;
      // printf("value %x load %x cur %x enabled %x\n",value, __load, cur,
      // __ctl.enabled());
      assert(__ctl.b.pre == 0);
      // assert(!__ctl.is16bit());
    }

    inline bool periodic() {
      return __ctl.b.oneshot == 0 and __ctl.b.periodic == 1;
    }

    inline bool freerunning() {
      return __ctl.b.oneshot == 0 and __ctl.b.periodic == 0;
    }

    inline bool oneshot() { return !periodic() and !freerunning(); }

    inline bool enabled() { return __ctl.b.en; }

    inline bool intenabled() { return __ctl.b.int_enable; }

    inline u32 dec() {
      if (shotdone)
        return cur;
      return __ctl.b.timersize32 ? cur - 1 : (cur & 0xffff) - 1;
    }

    bool tick() {
      u32 tmp = dec();
      // printf(" cur: %u load %u\n", cur, load());
      if (tmp > cur) {
        if (periodic()) {
          cur = load();
        } else if (freerunning()) {
          cur = ~u32(0);
        } else {
          shotdone = true;
        }
        // todo set int status
        return enabled() and intenabled();
      }
      cur = tmp;
      return false;
    }
  };

  Timer one;
  Timer two;

  bool tick1() { return one.tick(); }

  bool tick2() { return two.tick(); }

  static u32 read(u32 addr, u8 width, SP804 *ctx) {
    // assert(false);
    u32 offt = addr & 0xfff;
    // printf("SP804 read %x\n", offt);
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
    case 0x20:
      return ctx->two.load(value);
    case 0xc:
    case 0x2c: // clear int
    case 0x24:
    case 4: // read only regs
      return;
    case 8:
      return ctx->one.ctlb(value);
    case 0x28:
      return ctx->two.ctlb(value);
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
