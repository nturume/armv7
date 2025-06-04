#pragma once
#include "./stuff.hpp"
#include "elf.hpp"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <elf.h>

template <int size> struct Memory {
  u8 buf[size];

  template <typename T> T readBE(u64 pos) {
    T tmp;
    pos = pos % size;
    const u64 r = size - pos;
    memcpy(&tmp, buf + pos, sizeof(T) < r ? sizeof(T) : r);
    return tmp;
  }

  template <typename T> void writeBE(u64 pos, T value) {
    pos %= size;
    const u64 r = size - pos;
    memcpy(buf + pos, &value, sizeof(T) < r ? sizeof(T) : r);
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

  template <typename T> T readLE(u64 pos) {
    T tmp;
    pos = pos % size;
    const u64 r = size - pos;
    memcpy(&tmp, buf + pos, sizeof(T) < r ? sizeof(T) : r);
    u8 *b = (u8 *)&tmp;
    int half = sizeof(T) / 2;
    for (int j = 0; j < half; j++) {
      char c = b[j];
      b[j] = b[sizeof(T) - j - 1];
      b[sizeof(T) - j - 1] = c;
    }

    return tmp;
  }

  template <typename T> void writeLE(u64 pos, T value) {
    pos %= size;

    u8 *b = (u8 *)&value;
    int half = sizeof(T) / 2;
    for (int j = 0; j < half; j++) {
      char c = b[j];
      b[j] = b[sizeof(T) - j - 1];
      b[sizeof(T) - j - 1] = c;
    }

    const u64 r = size - pos;
    memcpy(buf + pos, &value, sizeof(T) < r ? sizeof(T) : r);
  }

  static void test() {
    Memory<64> m = {};
    m.writeBE<u8>(0, 0xff);
    assert(m.readBE<u8>(0) == 0xff);
    m.writeBE<u16>(0, 0xff01);
    assert(m.readBE<u16>(0) == 0xff01);
    m.writeBE<u32>(0, 0xff0000ff);
    assert(m.readBE<u32>(0) == 0xff0000ff);
    m.writeBE<u64>(0, 0xff00000000000000);
    assert(m.readBE<u64>(0) == 0xff00000000000000);

    m.writeLE<u8>(0, 0xff);
    assert(m.readLE<u8>(0) == 0xff);
    m.writeLE<u16>(0, 0xff01);
    assert(m.readLE<u16>(0) == 0xff01);
    m.writeLE<u32>(0, 0xff0000ff);
    assert(m.readLE<u32>(0) == 0xff0000ff);
    m.writeLE<u64>(0, 0xff00000000000000);
    assert(m.readLE<u64>(0) == 0xff00000000000000);

    printf("Memory::test() passed...\n");
  }
};
