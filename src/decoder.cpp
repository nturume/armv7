#include "./decoder.hpp"
#include <cstdio>

namespace Decoder {

static Instr dataProcAndMisc(u32 word) { return Instr::undefined; }

static Instr loadStoreWordAndUnsignedByte(u32 word) {
  struct __attribute__((packed)) I {
    u32 _1 : 4;
    u32 B : 1;
    u32 _2 : 11;
    // half
    u32 Rn : 4;
    u32 op1 : 5;
    u32 A : 1;
    u32 _3 : 2;
    u32 cond : 4;
  };
  I *instr = reinterpret_cast<I *>(&word);
  bool B = instr->B;
  bool A = instr->A;
  u8 Rn = instr->Rn;
  u8 op1 = instr->op1;

  printf("load store word and unsigned byte.. B: %u, A: %u\n", (u32)B, (u32)A);
  if (A) {
    if (!(op1 & 0b101) and !B and op1 != 0b1010 and op1 != 0b10) {
      return Instr::strReg;
    }
    if ((op1 == 0b10 or op1 == 0b1010) and !B)
      return Instr::strt2;
    if ((op1 & 0b101) == 1 and !B and op1 != 0b1011 and op1 != 0b11 and !B) {
      return Instr::ldrReg;
    }
    if ((op1 == 0b1011 or op1 == 0b11) and !B)
      return Instr::ldrt2;

    if ((op1 & 0b101) == 0b100 and !B and op1 != 0b1110 and op1 != 0b110)
      return Instr::strbReg;

    if ((op1 == 0b1110 or op1 == 0b110) and !B)
      return Instr::strbt2;

    if ((op1 & 0b101) == 0b101 and (op1 & 0b111) != 0b111 and !B) {
      return Instr::ldrbReg;
    }

    if ((op1 & 0b111) == 0b111 and !B)
      return Instr::ldrbt2;
  } else {
    if (!(op1 & 0b101) and op1 != 0b1010 and op1 != 0b10) {
      return Instr::strImm;
    }

    if ((op1 == 0b10 or op1 == 0b1010))
      return Instr::strt1;

    if ((op1 & 0b101) == 1 and op1 != 0b1011 and op1 != 0b11) {
      if (Rn == 0xf)
        return Instr::ldrLit;
      return Instr::ldrImm;
    }

    if (op1 == 0b1011 or op1 == 0b11)
      return Instr::ldrt1;

    if ((op1 & 0b101) == 0b100 and op1 != 0b1110 and op1 != 0b110)
      return Instr::strbImm;

    if (op1 == 0b1110 or op1 == 0b110)
      return Instr::strbt1;
    if ((op1 & 0b101) == 0b101 and op1 != 0b1111 and op1 != 0b111) {
      if (Rn == 0xf)
        return Instr::ldrbLit;
      return Instr::ldrbImm;
    }
    if ((op1 & 0b111) == 0b111)
      return Instr::ldrbt1;
  }
  return Instr::undefined;
}

static Instr parallelAddSub(u32 word) {
  struct I {
    u32 _1 : 5;
    u32 op2 : 3;
    u32 _2 : 12;
    u32 op1 : 2;
    u32 _3 : 6;
    u32 cond : 4;
  };

  I *i = reinterpret_cast<I *>(&word);
  switch (i->op1) {
  case 0b1:
    switch (i->op2) {
    case 0:
      return Instr::sadd16;
    case 1:
      return Instr::sasx;
    case 0b10:
      return Instr::ssax;
    case 0b11:
      return Instr::ssub16;
    case 0b100:
      return Instr::sadd8;
    case 0b111:
      return Instr::ssub8;
    }
    break;
  case 0b10:
    switch (i->op2) {
    case 0:
      return Instr::qadd16;
    case 1:
      return Instr::qasx;
    case 0b10:
      return Instr::qsax;
    case 0b11:
      return Instr::qsub16;
    case 0b100:
      return Instr::qadd8;
    case 0b111:
      return Instr::qsub8;
    }
    break;
  case 0b11:
    switch (i->op2) {
    case 0:
      return Instr::shadd16;
    case 1:
      return Instr::shasx;
    case 0b10:
      return Instr::shsax;
    case 0b11:
      return Instr::shsub16;
    case 0b100:
      return Instr::shadd8;
    case 0b111:
      return Instr::shsub8;
    }
    break;
  }

  return Instr::undefined;
}

static Instr packingUnpacking(u32 word) {
  return Instr::undefined;
}

static Instr signedMultDiv(u32 word) {}

static Instr media(u32 word) {
  struct __attribute__((packed)) I {
    u32 Rn : 4;
    u32 _1 : 1;
    u32 op2 : 3;
    u32 _2 : 4;
    u32 Rd : 4;
    u32 _3 : 4;
    u32 op1 : 5;
    u32 _4 : 3;
    u32 cond : 4;
  };

  I *i = reinterpret_cast<I *>(&word);
  u8 Rn = i->Rn;
  u8 op2 = i->op2;
  u8 Rd = i->Rd;
  u8 op1 = i->op1;

  if ((op1 & 0b11100) == 0) {
    return parallelAddSub(word);
  }

  if ((op1 & 0b11100) == 0b100) {
    return parallelAddSub(word);
  }

  if ((op1 & 0b11000) == 0b1000) {
    return packingUnpacking(word);
  }

  if ((op1 & 0b11000) == 0b10000) {
    return signedMultDiv(word);
  }

  if (op1 == 0b11000 and op2 == 0 and Rd == 0xf) {
    return Instr::usad8;
  }

  if (op1 == 0b11000 and op2 == 0) {
    return Instr::usada8;
  }

  if ((op1 & 0b11110) == 0b11010 and (op2 & 0b11) == 0b10) {
    return Instr::sbfx;
  }
  if ((op1 & 0b11110) == 0b11100 and (op2 & 0b11) == 0) {
    if (Rn == 0xf)
      return Instr::bfc;
    return Instr::bfi;
  }
  if ((op1 & 0b11110) == 0b11110 and (op2 & 0b11) == 0b10) {
    return Instr::ubfx;
  }

  return Instr::undefined;
}

static Instr branchAndBlockDataTransfer(u32 word) {
  struct __attribute__((packed)) I {
    u32 _1 : 15;
    u32 R : 1;
    u32 Rn : 4;
    u32 op : 6;
    u32 _2 : 2;
    u32 cond : 4;
  };
  I *i = reinterpret_cast<I *>(&word);
  u8 R = i->R;
  u8 op = i->op;
  u8 Rn = i->Rn;

  if (op == 0b10 or op == 0b0)
    return Instr::stmda;
  if (op == 0b11 or op == 0b1)
    return Instr::ldmda;
  if (op == 0b1010 or op == 0b1000)
    return Instr::stm;
  if (op == 0b1001)
    return Instr::ldm;
  if (op == 0b1011 and Rn != 0b1101)
    return Instr::ldm;
  if (op == 0b1011)
    return Instr::pop;
  if (op == 0b10000)
    return Instr::stmdb;
  if (op == 0b10010 and Rn != 0b1101)
    return Instr::stmdb;
  if (op == 0b10010)
    return Instr::push;
  if (op == 0b10001 or op == 0b10011)
    return Instr::ldmdb;
  if (op == 0b11010 or op == 0b11000)
    return Instr::stmib;
  if (op == 0b11001 or op == 0b11011)
    return Instr::ldmib;
  if ((op & 0b100101) == 0b100)
    return Instr::stmUser;
  if ((op & 0b100101) == 0b101 and !R)
    return Instr::ldmUser;
  if ((op & 0b100101) == 0b101)
    return Instr::ldmExRet;
  if ((op & 0b110000) == 0b100000)
    return Instr::b;
  if ((op & 0b110000) == 0b110000) {
    if (i->cond == 0xf)
      return Instr::blx; // unreachable
    return Instr::bl;
  }
  return Instr::undefined;
}

static Instr coprocessorAndSVC(u32 word) {
  struct __attribute__((packed)) I {
    u32 _1 : 4;
    u32 op : 1;
    u32 _2 : 3;
    u32 coproc : 4;
    u32 _3 : 4;
    u32 Rn : 4;
    u32 op1 : 6;
    u32 _4 : 2;
    u32 cond : 4;
  };
  I *i = reinterpret_cast<I *>(&word);
  u8 coproc = i->coproc;
  u8 op1 = i->op1;
  u8 op = i->op;
  u8 Rn = i->Rn;
  if (op1 == 0b0 or op1 == 0b1)
    return Instr::undefined;
  if ((op1 & 0b110000) == 0b110000)
    return Instr::svc;
  if (coproc == 0b1011 or coproc == 0b1010) {
    // not implementing floats and simd
  } else {
    if ((op1 & 0b100001) == 0 and (op1 & 0b111011) != 0)
      return Instr::stc1;
    if ((op1 & 0b100001) == 1 and (op1 & 0b111011) != 1) {
      if (Rn != 0xf)
        return Instr::ldc1Imm;
      return Instr::ldc1Lit;
    }
    if (op1 == 0b100) {
      return Instr::mcrr1;
    }
    if (op1 == 0b101) {
      return Instr::mrrc1;
    }
    if ((op1 & 0b110000) == 0b100000 and !op)
      return Instr::cdp1;
    if ((op1 & 0b110001) == 0b100000 and op)
      return Instr::mcr1;
    if ((op1 & 0b110001) == 0b100001 and op)
      return Instr::mrc1;
  }

  return Instr::undefined;
}

struct __attribute__((packed)) Gen {
  u32 _1 : 4;
  u32 op : 1;
  u32 _2 : 20;
  u32 op1 : 3;
  u32 cond : 4;
};

static_assert(sizeof(Gen) == 4, "");

static Instr conditional(Gen instr) {
  printf("conditional instruction... op1: %hhb\n", instr.op1);
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
  u32 _1 : 4;
  u32 op : 1;
  u32 _2 : 11;
  // half
  u32 Rn : 4;
  u32 op1 : 8;
  u32 cond : 4;
};

static Instr memoryHints(Uncond uncond) {
  struct __attribute__((packed)) MemHint {
    u32 _1 : 4;
    u32 op2 : 4;
    u32 _2 : 8;
    u32 Rn : 4;
    u32 op1 : 8;
    u32 cond : 4;
  };

  MemHint *memhint = reinterpret_cast<MemHint *>(&uncond);
  printf("memory Hints..\n");

  u8 op1 = memhint->op1;
  u8 op2 = memhint->op2;
  u8 Rn = memhint->Rn;

  switch (op1) {
  case 0b10000:
    if (!(op2 & 0b10) and !(Rn & 1)) {
      return Instr::cps;
    } else if (op2 == 0 and Rn & 1) {
      return Instr::setend;
    }
    break;
  case 0b10010:
    if (op2 == 0b111)
      return Instr::unpredictable;
    break;
  case 0b1001001:
  case 0b1000001:
    return Instr::nop;
  case 0b1001101:
  case 0b1000101:
    return Instr::pliImm;
  case 0b1011001:
  case 0b1010001:
    if (Rn == 0xf)
      return Instr::unpredictable;
    return Instr::pldImm;
  case 0b1011101:
  case 0b1010101:
    if (Rn == 0xf)
      return Instr::pldLit;
    return Instr::pldImm;
  case 0b1010011:
    return Instr::unpredictable;
  case 0b1010111:
    switch (op2) {
    case 0:
      return Instr::unpredictable;
    case 1:
      return Instr::clrex;
    case 0b100:
      return Instr::dsb;
    case 0b101:
      return Instr::dmb;
    case 0b110:
      return Instr::isb;
    default:
      return Instr::unpredictable;
    }
  case 0b1011111:
  case 0b1011011:
    return Instr::unpredictable;
  case 0b1100001:
  case 0b1101001:
    if (!(op2 & 1))
      return Instr::nop;
    break;
  case 0b1101101:
  case 0b1100101:
    if (!(op2 & 1))
      return Instr::pliReg;
    break;
  case 0b1111001:
  case 0b1110001:
  case 0b1110101:
  case 0b1111101:
    if (!(op2 & 1))
      return Instr::pldReg;
    break;
  case 0b1111111:
    if (op2 == 0xf)
      return Instr::undefined;
    break;
  default:
    if ((op1 >> 5) == 0b100 and (op1 & 0b11) == 0b11) {
      return Instr::unpredictable;
    };

    if ((op1 & 0b1100011) == 0b1100011 and !(op2 & 1))
      return Instr::unpredictable;
  }
  return Instr::undefined;
}

static Instr unconditional(Gen instr) {
  Uncond *uncond = reinterpret_cast<Uncond *>(&instr);
  printf("unconditional instruction...\n");

  if (!(uncond->op1 & 128)) {
    return memoryHints(*uncond);
  }
  u8 op1 = uncond->op1;
  if ((op1 >> 5 == 0b100) and (op1 & 0b100) and !(op1 & 1)) {
    return Instr::src;
  }

  if ((op1 >> 5 == 0b100) and !(op1 & 0b100) and (op1 & 1)) {
    return Instr::rfe;
  }

  if ((op1 >> 5 == 0b101)) {
    if (instr.cond == 0b1111)
      return Instr::blx;
    // unreachable
    return Instr::bl;
  }

  if ((op1 >> 5 == 0b110) and !(op1 & 1) and (op1 != 0b11000000) and
      (op1 != 0b11000100)) {
    return Instr::stc2;
  }

  if ((op1 >> 5 == 0b110) and (op1 & 1) and (op1 != 0b11000001) and
      (op1 != 0b11000101)) {
    if (uncond->Rn == 0b1111) {
      return Instr::ldc2Lit;
    }
    return Instr::ldc2Imm;
  }

  if (op1 == 0b11000100) {
    return Instr::mcrr2;
  }

  if (op1 == 0b11000101) {
    return Instr::mrrc2;
  }

  if (op1 >> 4 == 0b1110 and !instr.op) {
    return Instr::cdp2;
  }

  if (op1 >> 4 == 0b1110 and !(op1 & 1) and instr.op) {
    return Instr::mcr2;
  }

  if (op1 >> 4 == 0b1110 and (op1 & 1) and instr.op) {
    return Instr::mrc2;
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
