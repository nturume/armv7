#pragma once
#include "./elf.hpp"
#include "./stuff.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>
#include <vector>

#define DEV_BASE 0x40000000

struct Region {
  u32 start;
  u32 len;

  u32 (*r)(u32 addr, u8 width) = nullptr;
  void (*w)(u32 addr, u32 value, u8 width) = nullptr;

  u32 (*ra)(u32 addr, u8 width) = nullptr;
  void (*wa)(u32 addr, u32 value, u8 width) = nullptr;

  bool has(u32 addr) { return addr >= start and addr < (start + len); }

  u32 read(u32 addr, u8 width) {
    if (r != nullptr) {
      return r(addr, width);
    }
    assert("region read() is arsenal UCL" == nullptr);
  }

  void write(u32 addr, u32 value, u8 width) {
    if (w != nullptr) {
      return w(addr, value, width);
    }
    assert("region write() is arsenal UCL" == nullptr);
  }
};

struct InvalidRegion : std::exception {
  u32 address;
  InvalidRegion(u32 addr) :address(addr) {};
};

template <int size> struct Memory {
  u8 buf[size + 4];
  u32 rambase = 0;

  std::vector<Region> regions = {};

  // Memory(u32 rambase) : rambase(rambase) {}

  inline bool isRam(u32 addr) {
    return addr >= rambase and addr < (rambase + size);
  }

  inline u32 ramOfft(u32 addr) { return addr - rambase; }

  void writeRegion(u32 addr, u32 value, u8 width) {
    for (u32 i = 0; i < regions.size(); i++) {
      Region region = regions[i];
      if (region.has(addr)) {
        return region.write(addr, value, width);
      }
    }
    throw InvalidRegion(addr);
  }

  u32 readRegion(u32 addr, u8 width) {
    for (u32 i = 0; i < regions.size(); i++) {
      Region region = regions[i];
      if (region.has(addr)) {
        return region.read(addr, width);
      }
    }
    throw InvalidRegion(addr);
  }

  u32 a32u(u64 pos) {
    if (isRam(pos)) {
      return *reinterpret_cast<u32 *>(&buf[ramOfft(pos)]);
    }
    return readRegion(pos, 4);
  }

  void a32u(u64 pos, u32 value) {
    if (isRam(pos)) {
      *reinterpret_cast<u32 *>(&buf[ramOfft(pos)]) = value;
    }
    return writeRegion(pos, value, 4);
  }

  u16 a16u(u64 pos) {
    if (isRam(pos)) {
      return *reinterpret_cast<u16 *>(&buf[ramOfft(pos)]);
    }
    return readRegion(pos, 2);
  }

  void a16u(u64 pos, u16 value) {
    if (isRam(pos)) {
      *reinterpret_cast<u16 *>(&buf[ramOfft(pos)]) = value;
    }
    return writeRegion(pos, value, 2);
  }

  u8 a8u(u64 pos) {
    if (isRam(pos)) {
      return buf[ramOfft(pos)];
    }
    return readRegion(pos, 1);
  }

  void a8u(u64 pos, u8 value) {
    if (isRam(pos)) {
      buf[ramOfft(pos)] = value;
    }
    return writeRegion(pos, value, 1);
  }

  u32 a32a(u64 pos) {
    // TODO
    if (isRam(pos)) {
      return *reinterpret_cast<u32 *>(&buf[ramOfft(pos)]);
    }
    return readRegion(pos, 4);
  }

  void a32a(u64 pos, u32 value) {
    // TODO
    if (isRam(pos)) {
      *reinterpret_cast<u32 *>(&buf[ramOfft(pos)]) = value;
    }
    return writeRegion(pos, value, 4);
  }

  u16 a16a(u64 pos) {
    // TODO
    if (isRam(pos)) {
      return *reinterpret_cast<u16 *>(&buf[ramOfft(pos)]);
    }
    return readRegion(pos, 2);
  }

  void a16a(u64 pos, u16 value) {
    // TODO
    if (isRam(pos)) {
      *reinterpret_cast<u16 *>(&buf[ramOfft(pos)]) = value;
    }
    return writeRegion(pos, value, 2);
  }

  u8 a8a(u64 pos) {
    // TODO
    if (isRam(pos)) {
      return buf[ramOfft(pos)];
    }
    return readRegion(pos, 1);
  }

  void a8a(u64 pos, u8 value) {
    // TODO
    if (isRam(pos)) {
      buf[ramOfft(pos)] = value;
    }
    return writeRegion(pos, value, 1);
  }

  void loadElf(Elf::ProgramHeaderIterator it) {
    Elf::Ph ph = {};
    while (it.next(&ph)) {
      if (ph.p_type != PT_LOAD)
        continue;
      FileReader fr(it.file);
      fr.seekTo((i64)ph.p_offset);
      printf("  loading ph at: %u to %u (%u)bytes\n", ph.p_vaddr,
             ph.p_vaddr + ph.p_filesz, ph.p_filesz);
      if (size < ph.p_vaddr + ph.p_filesz) {
        printf("  attempt to load program out of memory. memsize: (%u) vaddr "
               "end: (%u)\n",
               size, ph.p_vaddr + ph.p_filesz);
        exit(1);
      }
      u64 r = fr.read(buf + ph.p_vaddr, ph.p_filesz);
      if (r != ph.p_filesz) {
        printf("Error loading program headers\n");
        exit(1);
      }
    }
  }

  // template <typename T> T readLE(u64 pos) {
  //   T tmp;
  //   pos = pos % size;
  //   const u64 r = size - pos;
  //   memcpy(&tmp, buf + pos, sizeof(T) < r ? sizeof(T) : r);
  //   u8 *b = (u8 *)&tmp;
  //   int half = sizeof(T) / 2;
  //   for (int j = 0; j < half; j++) {
  //     char c = b[j];
  //     b[j] = b[sizeof(T) - j - 1];
  //     b[sizeof(T) - j - 1] = c;
  //   }

  //   return tmp;
  // }

  // template <typename T> void writeLE(u64 pos, T value) {
  //   pos %= size;

  //   u8 *b = (u8 *)&value;
  //   int half = sizeof(T) / 2;
  //   for (int j = 0; j < half; j++) {
  //     char c = b[j];
  //     b[j] = b[sizeof(T) - j - 1];
  //     b[sizeof(T) - j - 1] = c;
  //   }

  //   const u64 r = size - pos;
  //   memcpy(buf + pos, &value, sizeof(T) < r ? sizeof(T) : r);
  // }

  static void test() {
    // Memory<64> m = {};
    // m.writeBE<u8>(0, 0xff);
    // assert(m.readBE<u8>(0) == 0xff);
    // m.writeBE<u16>(0, 0xff01);
    // assert(m.readBE<u16>(0) == 0xff01);
    // m.writeBE<u32>(0, 0xff0000ff);
    // assert(m.readBE<u32>(0) == 0xff0000ff);
    // m.writeBE<u64>(0, 0xff00000000000000);
    // assert(m.readBE<u64>(0) == 0xff00000000000000);

    // m.writeLE<u8>(0, 0xff);
    // assert(m.readLE<u8>(0) == 0xff);
    // m.writeLE<u16>(0, 0xff01);
    // assert(m.readLE<u16>(0) == 0xff01);
    // m.writeLE<u32>(0, 0xff0000ff);
    // assert(m.readLE<u32>(0) == 0xff0000ff);
    // m.writeLE<u64>(0, 0xff00000000000000);
    // assert(m.readLE<u64>(0) == 0xff00000000000000);

    // printf("Memory::test() passed...\n");
  }
};
