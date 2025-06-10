#pragma once
#include "stuff.hpp"
#include "mem.hpp"
#include "assert.h"
#include "arith.hpp"

struct Cpu{

  union APSR {
    struct {
      u32 _1: 28;
      u32 q: 1;
      u32 v: 1;
      u32 c: 1;
      u32 z: 1;
      u32 n: 1;
    } b;
    u32 back = 0;
  };

  APSR apsr;
  u32 regs[16];
  Memory<64> mem;

  inline u32 r(u8 pos) {
    return regs[pos];
  } 

  inline void r(u8 pos, u32 value) {
    regs[pos] = value;
  }

  inline u8 currentInstrSet(){ return 0; }

  inline u32 pcStoreValue() {
    return regs[15]+8;
  }

  inline void branchTo(u32 ptr) {
    
  }

  inline void branchWritePc(u32 ptr) {
    branchTo(ptr&(0xffffffff<<2));
  } 

  inline void bxWritePc(u32 ptr) {
    (void)ptr;
    printf("bx write pc detetcted...\n");
    exit(1);
  }

  inline void aluWritePc(u32 ptr) {
    return bxWritePc(ptr);
  }

  inline void loadWritePc(u32 ptr) {
    return bxWritePc(ptr);
  }

  inline u32 expandImm(u16 imm12) {
    return Arith::expandImmC(imm12, apsr.b.c).v.u;
  }
};
