
#pragma once
#include <cassert>
#include <cstddef>
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
  
  u64 write(u8 *buf, u64 len) {
    u64 written = 0;
    while (written < len) {
      u64 amnt = len - written;
      u64 n = fwrite(buf + written, 1, amnt, f);
      if (n != amnt) {
        if (ferror(f)) {
          printf("FileReader::write() failed\n");
          std::exit(1);
        }
        break;
      }
      written += n;
    }
    return written;
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
    assert(ftell(f)==n);
  }

  u64 getPos() {
    return ftell(f);
  }
  
  u32 getFileSize() {
    if (fseek(f, 0, SEEK_END)) {
      printf("FileReader::getFileSize() failed.\n");
      std::exit(1);
    }
    u32 size = ftell(f);
    seekTo(0);
    return size;
  }

  void close () {
    fclose(f);
  }
};

inline u32 align4(u32 addr) { return addr & (u32(0xffffffff) << 2); }

inline u32 align2(u32 addr) { return addr & (u32(0xffffffff) << 1); }

inline size_t alignB(size_t addr, size_t target) {
  return addr & ~(target-1);
}

inline size_t alignF(size_t addr, size_t target) {
  return alignB(addr-1, target) + target;
}

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

inline i64 s64(u64 v) {
  union {
    i64 i;
    u64 u;
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

inline u64 uns64(i64 v) {
  union {
    i64 i;
    u64 u;
  } x = {.i = v};
  return x.u;
}


inline u32 sx8(u8 v) {
  union {
    i8 i;
    u8 u;
  } x = {.u = v};
  return uns32(i32(x.i));
}

inline u32 sx16(u16 v) {
  union {
    i16 i;
    u16 u;
  } x = {.u = v};
  return uns32(i32(x.i));
}

static inline i32 abs32(i32 v) {
  return v>=0?v:-v;
}

inline u8 bitcount16(u16 v) {
  u8 bits = 0;
  while(v) {
    bits += (v&1);
    v >>= 1;
  }
  return bits;
}

inline u8 bitcount32(u32 v) {
  u8 bits = 0;
  while(v) {
    bits += (v&1);
    v >>= 1;
  }
  return bits;
}

inline u8 clz32(u32 v) {
  u8 bits = 0;
  while((v&1)==0 and bits<32) {
    bits+=1;
    v >>= 1;
  }
  return bits;
}

inline u8 u5(u8 v) {
  return v&0b11111;
}

inline u32 swap32(u32 v) {
  u32 res;
  u8 *src = (u8*)&v;
  u8 *dest = (u8*)&res;
  dest[0] = src[3];
  dest[1] = src[2];
  dest[2] = src[1];
  dest[3] = src[0];
  return res;
}

static void swap8(u8 *buf, unsigned int len) {
    u32 half = len/2;
    for(u32 j  = 0; j < half; j++) {
        u8 c = buf[j];
        buf[j] = buf[len-j-1];
        buf[len-j-1] = c;
    }
}
