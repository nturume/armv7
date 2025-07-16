
#pragma once
#include "stuff.hpp"
#include <cassert>

template <typename T, u32 N>
struct FIFO {
  T buf[N];
  
  u32 rpos = 0;
  u32 wpos = 0;

  bool _full = false;
  bool _empty = true;
  // bool _hfull = false;
  // bool _hempty = true;

  void drain() {
    *this = {};
  }

  bool empty() {
    return _empty;
  }

  bool full() {
    return _full;
  }

  u32 incIdx(i32 idx) {
    return (idx+1)%N;
  }
  
  T read() {
    assert(!empty());
    T data = buf[rpos];
    rpos = incIdx(rpos);
    _full = false;
    if(wpos==rpos) {
      _empty = true;
    } 
    return data;
  }

  void write(T v) {
    assert(!full());
    buf[wpos] = v;
    wpos = incIdx(wpos);
    _empty = false;
    if(wpos==rpos) {
      _full = true;
    } 
  }
};
