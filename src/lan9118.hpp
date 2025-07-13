
#pragma once
#include "mem.hpp"
#include "stuff.hpp"
#include <cassert>

struct LAN9118 {
  static u32 read(u32 addr, u8 width, LAN9118 *ctx) { 
   // assert(false); 
      return 0; 
  }
  static void write(u32 addr, u32 value, u8 width, LAN9118 *ctx) {
    // assert(false);
  }

  Region getRegion(u32 start) {
    return Region {
      .start = start, .len = 0x1000, .isram = false, .ctx = this,
      .r = (u32(*)(u32, u8, void *)) & read,
      .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
