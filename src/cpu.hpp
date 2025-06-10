#pragma once
#include "arith.hpp"
#include "assert.h"
#include "decoder.hpp"
#include "mem.hpp"
#include "stuff.hpp"

struct Cpu {

  union APSR {
    struct {
      u32 _1 : 28;
      u32 q : 1;
      u32 v : 1;
      u32 c : 1;
      u32 z : 1;
      u32 n : 1;
    } b;
    u32 back = 0;
  };

  APSR apsr;
  u32 regs[16] = {0};
  Memory<64> mem;

  u32 cur;

  inline bool cnd() {
    u8 cond = cur >> 28;
    bool result = false;
    switch (cond >> 1) {
    case 0:
      result = apsr.b.z;
      break;
    case 1:
      result = apsr.b.c;
      break;
    case 0b10:
      result = apsr.b.n;
      break;
    case 0b11:
      result = apsr.b.v;
      break;
    case 0b100:
      result = apsr.b.c and !apsr.b.z;
      break;
    case 0b101:
      result = apsr.b.n and apsr.b.v;
      break;
    case 0b110:
      result = apsr.b.n and !apsr.b.z;
      break;
    case 0b111:
      result = true;
      break;
    }
    if ((cond & 1) and cond != 0xf) {
      result = !result;
    }
    return result;
  }

  inline u32 r(u8 pos) { return regs[pos]; }

  inline void r(u8 pos, u32 value) { regs[pos] = value; }

  inline u8 currentInstrSet() { return 0; }

  inline u32 pcStoreValue() { return regs[15] + 8; }

  inline void branchTo(u32 ptr) { regs[15] = ptr; }

  inline void branchWritePc(u32 ptr) { branchTo(ptr & (0xffffffff << 2)); }

  inline u32 sp() { return regs[13]; }

  inline void sp(u32 v) { regs[13] = v; }

  inline u32 rl() { return regs[14]; }

  inline void rl(u32 v) { regs[14] = v; }

  inline u32 pc() { return regs[15] + 8; }

  inline u32 pcReal() { return regs[15]; }

  inline void bxWritePc(u32 ptr) {
    (void)ptr;
    printf("bx write pc detetcted...\n");
    exit(1);
  }

  inline void aluWritePc(u32 ptr) { return bxWritePc(ptr); }

  inline void loadWritePc(u32 ptr) { return bxWritePc(ptr); }

  inline u32 expandImm(u16 imm12) {
    return Arith::expandImmC(imm12, apsr.b.c).v.u;
  }

  void printRegisters() {
    printf("=========regs==========\n");
    for (u32 i = 0; i < 16; i++) {
      printf("r%d = %d\n", i, r(i));
    }
    printf("=========flags=========\n");
    printf("n: %d, z: %d, c: %d, v: %d, q: %d\n", (u32)apsr.b.n, (u32)apsr.b.z,
           (u32)apsr.b.c, (u32)apsr.b.v, (u32)apsr.b.q);
  }

  u32 exec(u32 word);

  inline u32 adcImm() {
    if (cnd()) {
      u8 d = (cur >> 12) & 0xf;
      u8 n = (cur >> 16) & 0xf;
      bool setflags = (cur & (1 << 20)) > 0;
      Arith::Adc res = Arith::adc(r(n), expandImm((u16)cur), apsr.b.c);
      if (d == 15) {
        aluWritePc(res.res);
      } else {
        r(d, res.res);
        if (setflags) {
          apsr.b.n = (res.res & 0x80000000) > 0;
          apsr.b.z = (res.res == 0);
          apsr.b.c = res.c;
          apsr.b.v = res.of;
        }
      }
    }
    return pcReal() + 4;
  }
};
