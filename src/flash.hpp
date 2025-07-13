#pragma once
#include "mem.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>

struct Flash {
  u32 len;
  u8 *buf;

  enum class Mode {
    read,
    cfi,
    unlock1,
    unlock2,
    id,
    readstatus,
  };

  Mode mode = Mode::read;

  u8 getstatus() {
    // TODO
    return 0x81;
  }

  void setmode(Mode m) { mode = m; }

  Mode getmode() { return mode; }

  u8 r8(u32 pos) { return buf[pos]; }

  u16 r16(u32 pos) { return *reinterpret_cast<u16 *>(&buf[pos]); }

  u32 r32(u32 pos) { return *reinterpret_cast<u32 *>(&buf[pos]); }

  static u32 format(u32 value, u8 width) {
    u32 ret = value;
    switch (width) {
    case 1: {
      ret = u8(value);
      ret |= ret << 8;
      ret |= ret << 16;
      break;
    }
    case 2: {
      ret = u16(value);
      ret |= ret << 16;
      break;
    }
    case 4:
      break;
    default:
      printf("Bad width: %d\n", width);
      assert(false);
    }
    return ret;
  }

  static u32 read(u32 addr, u8 width, Flash *ctx) {
    u32 offt = addr >> 2;
    // assert(false);
    // printf("FLASH was read --> addr %x width: %d\n", addr, width);
    if (ctx->mode == Mode::read) {
    // printf("FLASH was read --> addr %x width: %d\n", addr, width);
      switch (width) {
      case 1:
        return ctx->r8(addr);
      case 2:
        return ctx->r16(addr);
      case 4:
        return ctx->r32(addr);
      default:
        assert(1 == 2);
      }
    }

    if (ctx->mode == Mode::readstatus) {
      ctx->setmode(Mode::read);
      return format(ctx->getstatus(), 1);
    }

    if (ctx->mode == Mode::cfi) {
      // printf("========OFFT: %x size %d\n", offt, width);
      switch (offt) {
      case 0x10:
        return format('Q', 1);
      case 0x11:
        return format('R', 1);
      case 0x12:
        return format('Y', 1);
      case 0x13:
        return format(2, 1);
      case 0x14:
        return 0;
      case 0x15:
        return format(0x31, 1);
      case 0x16:
      case 0x17:
      case 0x18:
      case 0x19:
      case 0x1a:
      case 0x1b:
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f:
      case 0x20:
      case 0x21:
      case 0x22:
      case 0x23:
      case 0x24:
      case 0x25:
      case 0x26:
        return 0;
      case 0x27:
        return format(26, 1);
      case 0x28:
        return format(7, 1);
      case 0x29:
        return 0;
      case 0x2a:
        return format(8, 1);
      case 0x2b:
        return 0;
      case 0x2c:
        return format(1, 1);
      case 0x2d:
        return format(0xff, 1);
      case 0x2e:
        return format(0x3, 1);
      case 0x2f:
        return 0;
      case 0x30:
        return format(1, 1);
      case 0x31:
        return format('P', 1);
      case 0x32:
        return format('R', 1);
      case 0x33:
        return format('I', 1);
      case 0x34:
        return format(0x1, 1);
      case 0x35:
        return format(0x0, 1);
      case 0x36:
        return format(0x0, 1);
      case 0x37:
        return format(0x0, 1);
      case 0x38:
        return format(0x0, 1);
      case 0x39:
        return format(0x0, 1);
      case 0x3a:
        return format(0x0, 1);
      case 0x3b:
        return format(0x0, 1);
      case 0x3c:
        return format(0x0, 1);
      case 0x3d:
        return format(0x0, 1);
      default:
        assert(false);
      }
    }

    if (ctx->mode == Mode::id) {
      switch (offt) {
      case 0:
        return format(0x1, 1);
      case 1:
        return format(0xa4, 1);
      }
    }

    printf("FLASH was read -->addr %x width: %d\n", offt, width);
    assert(false);
  }

  static void write(u32 addr, u32 value, u8 width, Flash *ctx) {
    u32 pos = addr >> 2;
    if (ctx->mode == Mode::read) {
      switch (value) {
      case 0xf0f0f0f0:
      case 0xf0f0:
      case 0xf0:
        return ctx->setmode(Mode::read);
      case 0xffffffff:
      case 0xffff:
      case 0xff:
        return; // nop
      default: {
        if (pos == 0x55 and (value & 0xff) == 0x98) {
          return ctx->setmode(Mode::cfi);
        }

        if (pos == 0x555 and (value & 0xff) == 0xaa) {
          return ctx->setmode(Mode::unlock1);
        }

        if ((value & 0xff) == 0x70) {
          return ctx->setmode(Mode::readstatus);
        }

        printf("BAD MODE: offt %x value %x, width %x\n", pos, value, width);
        abort();
      }
      }
    }

    if (ctx->mode == Mode::cfi) {
      switch (value) {
      case 0xf0f0f0f0:
      case 0xf0f0:
      case 0xf0:
        return ctx->setmode(Mode::read);
      default: {
        printf("BAD MODE: addr %x value %x, width %x\n", addr, value, width);
        abort();
      }
      }
    }

    if (ctx->mode == Mode::unlock1) {
      if (pos == 0x2aa and (value & 0xff) == 0x55) {
        return ctx->setmode(Mode::unlock2);
      }
    }

    if (ctx->mode == Mode::unlock2) {
      if (pos == 0x555) {
        switch (value & 0xff) {
        case 0x90:
          return ctx->setmode(Mode::id);
        case 0xf0:
          return ctx->setmode(Mode::read);
        }
      }
    }

    if (ctx->mode == Mode::id) {
      switch (value & 0xff) {
      case 0xf0:
        return ctx->setmode(Mode::read);
      }
    }

    printf("FLASH was written -->addr %x value %x width: %d\n", pos, value,
           width);
    assert(false);
  }

  Flash(u32 len) : len(len) { buf = new u8[len]; }

  Region getRegion(u32 start) {
    return Region{
        .start = start,
        .len = len,
        .isram = false,
        .ctx = this,
        .r = (u32(*)(u32, u8, void *)) & read,
        .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
