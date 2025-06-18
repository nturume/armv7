#pragma once
#include "mem.hpp"

struct UART {

  static u32 read(u32 addr, u8 width) {
    printf("UART was read!!!! width: %d\n", width);
    return 0;
  }

  static void write(u32 addr, u32 value, u8 width) {
    printf("UART was written!!! width: %d\n", width);
  }

  static Region getRegion() {
    return {
        .start = 0x40000000,
        .end = 0x40000100,
        .r = &read,
        .w = &write,
    };
  }
};
