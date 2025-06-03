#pragma once
#include "./stuff.hpp"
#include "elf.hpp"
#include <cstring>

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
};
