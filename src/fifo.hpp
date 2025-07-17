
#pragma once
#include "stuff.hpp"
#include <cassert>
#include <condition_variable>

template <typename T, u32 N> struct FIFO {
  T buf[N];

  u32 rpos = 0;
  u32 wpos = 0;

  bool _full = false;
  bool _empty = true;
  // bool _hfull = false;
  // bool _hempty = true;

  void drain() { *this = {}; }

  bool empty() { return _empty; }

  bool full() { return _full; }

  u32 _size() {
    if(wpos > rpos) {
      return wpos-rpos;
    }
    return rpos-wpos;
  }

  bool halfEmpty() {
    return _size()<=(N/2);
  }

  u32 incIdx(u32 idx) { return (idx + 1) % N; }

  T read() {
    assert(!empty());
    T data = buf[rpos];
    rpos = incIdx(rpos);
    _full = false;
    if (wpos == rpos) {
      _empty = true;
    }
    return data;
  }

  void write(T v) {
    assert(!full());
    buf[wpos] = v;
    wpos = incIdx(wpos);
    _empty = false;
    if (wpos == rpos) {
      _full = true;
    }
  }
};

template <typename T, u32 N> struct SharedFIFO {
  T buf[N];

  u32 rpos = 0;
  u32 wpos = 0;

  bool _full = false;
  bool _empty = true;

  std::mutex mut;
  std::condition_variable cvne;
  std::condition_variable cvnf;

  // bool _hfull = false;
  // bool _hempty = true;

  void drain() {
    std::unique_lock<std::mutex> lock(mut);
    rpos = 0;
    wpos = 0;
    _full = false;
    _empty = true;
    cvne.notify_all();
    cvnf.notify_all();
  }

  bool empty() {
    std::unique_lock<std::mutex> lock(mut);
    return _empty;
  }

  bool full() {
    std::unique_lock<std::mutex> lock(mut);
    return _full;
  }
  
  u32 _size() {
    if(wpos > rpos) {
      return wpos-rpos;
    }
    return rpos-wpos;
  }

  bool halfEmpty() {
    std::unique_lock<std::mutex> lock(mut);
    return _size()<=(N/2);
  }

  bool __empty() { return _empty; }

  bool __full() { return _full; }

  u32 incIdx(u32 idx) { return (idx + 1) % N; }

  T read() {
    std::unique_lock<std::mutex> lock(mut);
    cvne.wait(lock, [this] { return !__empty(); });
    T data = buf[rpos];
    rpos = incIdx(rpos);
    _full = false;
    if (wpos == rpos) {
      _empty = true;
    }
    cvnf.notify_one();
    return data;
  }

  void write(T v) {
    std::unique_lock<std::mutex> lock(mut);
    cvnf.wait(lock, [this] { return !__full(); });
    buf[wpos] = v;
    wpos = incIdx(wpos);
    _empty = false;
    if (wpos == rpos) {
      _full = true;
    }
    cvne.notify_one();
  }
};
