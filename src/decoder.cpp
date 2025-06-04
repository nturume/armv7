#include "./decoder.hpp"
#include <cstdio>

namespace Decoder {

static Instr dataProcAndMisc(u32 word) {}

static Instr loadStoreWordAndUnsignedByte(u32 word) {}

static Instr media(u32 word) {}

static Instr branchAndBlockDataTransfer(u32 word) {}

static Instr coprocessorAndSVC(u32 word) {}

struct __attribute__((packed)) Gen {
  u32 _1 : 4;
  u32 op : 1;
  u32 op1 : 3;
  u32 _2 : 20;
  u32 cond : 4;
};

static_assert(sizeof(Gen) == 4, "");

static Instr conditional(Gen instr) {
  printf("conditional instruction...\n");
  switch (instr.op1) {
  case 0b10:
    return loadStoreWordAndUnsignedByte(*reinterpret_cast<u32 *>(&instr));
  case 0b11:
    switch (instr.op) {
    case 0:
      return loadStoreWordAndUnsignedByte(*reinterpret_cast<u32 *>(&instr));
    case 1:
      return media(*reinterpret_cast<u32 *>(&instr));
    }
  default:
    switch ((instr.op1 >> 1) & 0b11) {
    case 0:
      return dataProcAndMisc(*reinterpret_cast<u32 *>(&instr));
    case 2:
      return branchAndBlockDataTransfer(*reinterpret_cast<u32 *>(&instr));
    case 3:
      return coprocessorAndSVC(*reinterpret_cast<u32 *>(&instr));
    default:
      printf("Unexpected Gen::op1: %u", instr.op1);
      exit(1);
    }
  }
}


struct __attribute__((packed)) Uncond {
  u32 _1: 4;
  u32 op: 1;
  u32 _2: 11;
  // half
  u32 Rn: 4;
  u32 op1: 8;
  u32 cond: 4;
};


static Instr memoryHints(Uncond uncond) {
  printf("memory Hints..\n");  
}

static Instr unconditional(Gen instr) {
  Uncond *uncond = reinterpret_cast<Uncond*>(&instr);
  printf("unconditional instruction...\n");

  if(!(uncond->op1&128)) {
    return memoryHints(*uncond);
  }
  u8 op1 = uncond->op1;
  if((op1>>5==0b100) and (op1&0b100) and !(op1&1)){
    return Instr:: src;
  }

  
  if((op1>>5==0b100) and !(op1&0b100) and (op1&1)){
    return Instr:: rfe;
  }
}

Instr decodeA(u32 instr) {
  printf("decoding stuff...\n");
  const Gen *gen = reinterpret_cast<Gen *>(&instr);
  Instr res;
  switch (gen->cond) {
  case 0b1111:
    res = unconditional(*gen);
    break;
  default:
    res = conditional(*gen);
  }
  printInstr(res);
  return res;
}
} // namespace Decoder
