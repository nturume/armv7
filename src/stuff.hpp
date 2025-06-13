
#pragma once
#include <cstdio>
#include <cstdlib>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long;

using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long;

#define fn auto

struct FileReader {
  FILE *f;

  FileReader(FILE *file) : f(file) {}

  u64 read(u8 *buf, u64 len) {
    u64 read = 0;
    while (read < len) {
      u64 n = fread(buf + read, 1, len - read, f);
      read += n;
      if (n == 0) {
        if (ferror(f)) {
          printf("FileReader::read() failed\n");
          std::exit(1);
        }
        break;
      }
    }
    return read;
  }

  fn seekBy(i64 n) {
    if (fseek(f, n, SEEK_CUR)) {
      printf("FileReader::seekBy() failed.\n");
      std::exit(1);
    }
  }

  fn seekTo(i64 n) {
    if (fseek(f, n, SEEK_SET)) {
      printf("FileReader::seekBy() failed.\n");
      std::exit(1);
    }
  }
};

inline u32 align4(u32 addr) { return addr & (u32(0xffffffff) << 2); }

inline u32 align2(u32 addr) { return addr & (u32(0xffffffff) << 1); }

inline i8 s8(u8 v) {
  union {
    i8 i;
    u8 u;
  } x = {.u = v};
  return x.i;
}

inline i16 s16(u16 v) {
  union {
    i16 i;
    u16 u;
  } x = {.u = v};
  return x.i;
}

inline i32 s32(u32 v) {
  union {
    i32 i;
    u32 u;
  } x = {.u = v};
  return x.i;
}

inline u32 uns32(i32 v) {
  union {
    i32 i;
    u32 u;
  } x = {.i = v};
  return x.u;
}
