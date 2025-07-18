#include "mem.hpp"
#include <cassert>
#include <cstdlib>

struct SYS {
  static u32 read(u32 addr, u8 width, void *ctx) { 
       u32 offt = addr&0xfff;
       switch(offt) {
         default:
         case 0x84: return 0xc000191;
         printf("unhandled offt: %x\n", offt);
         assert(false);
       }
      return 0; 
  }
  static void write(u32 addr, u32 value, u8 width, void *ctx) {
    assert(false);
  }

  static Region getRegion(u32 start) {
    return Region {
      .start = start, .len = 0x1000, .isram = false, .ctx = nullptr,
      .r = (u32(*)(u32, u8, void *)) & read,
      .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
