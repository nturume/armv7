#include "./decoder.hpp"
#include <cstdio>
#include <cstring>

namespace Decoder {

static Instr satAddSub(u32 word) {
  switch (word & 0b11000000000000000000000) {
  case 0b00000000000000000000000:
    return Instr::qadd;
  case 0b01000000000000000000000:
    return Instr::qsub;
  case 0b10000000000000000000000:
    return Instr::qdadd;
  case 0b11000000000000000000000:
    return Instr::qdsub;
  }
  return Instr::undefined;
}

static Instr halfMult(u32 word) {
  u8 op1 = ((word >> 21) & 0b11);
  u8 op = word & (1 << 5);

  switch (op1) {
  case 0:
    return Instr::smlabb;
  case 1:
    if (!op)
      return Instr::smlawb;
    else
      return Instr::smulwb;
  case 2:
    return Instr::smlalbb;
  case 3:
    return Instr::smulbb;
  }

  return Instr::undefined;
}

static Instr mult(u32 word) {
  u8 op = (word >> 20) & 0xf;
  if (!(op & 0b1110))
    return Instr::mul;
  if ((op & 0b1110) == 0b10)
    return Instr::mla;
  switch (op) {
  case 0b100:
    return Instr::umaal;
  case 0b110:
    return Instr::mls;
  }
  switch (op >> 1) {
  case 0b100:
    return Instr::umull;
  case 0b101:
    return Instr::umlal;
  case 0b110:
    return Instr::smull;
  case 0b111:
    return Instr::smlal;
  }
  return Instr::undefined;
}

static Instr misc(u32 word) {
  union {
    struct {
      u32 _1 : 4;
      u32 op2 : 3;
      u32 _2 : 2;
      u32 B : 1;
      u32 _3 : 6;
      u32 op1 : 4;
      u32 _4 : 1;
      u32 op : 2;
      u32 _5 : 9;
    } b;
    u32 v;
  } i = {.v = word};
  u8 op2 = i.b.op2;
  u8 op1 = i.b.op1;
  u8 op = i.b.op;
  u8 B = i.b.B;
  if (op2 == 0) {
    if (B and !(op & 1))
      return Instr::mrsBanked;
    if (B and (op & 1))
      return Instr::msrBanked;
    if (!B and !(op & 1))
      return Instr::mrs;
    if (!B and (op & 0b11) == 1 and !(op1 & 0b11))
      return Instr::msrApp;
    if (!B and (op & 0b11) == 1 and ((op1 & 0b11) == 1 or (op1 & 0b10)))
      return Instr::msrSys;
    if (op == 0b11)
      return Instr::msrSys;
  }

  if (op2 == 1 and op == 1)
    return Instr::bx;
  if (op2 == 1 and op == 0b11)
    return Instr::clz;

  if (op2 == 0b10 and op == 1)
    return Instr::bxj;
  if (op2 == 0b11 and op == 1)
    return Instr::blxReg;

  if (op2 == 0b101)
    return satAddSub(word);

  if (op2 == 0b110 and op == 0b11)
    return Instr::eret;
  if (op2 == 0b111 and op == 0b1)
    return Instr::bkpt;

  if (op2 == 0b111 and op == 0b10)
    return Instr::hvc;
  if (op2 == 0b111 and op == 0b11)
    return Instr::smc;

  return Instr::undefined;
}

static Instr dataProcShiftedReg(u32 word) {
  union {
    struct {
      u32 _1 : 5;
      u32 op2 : 2;
      u32 _2 : 13;
      u32 op1 : 5;
      u32 _3 : 7;
    } b;
    u32 v;
  } i = {.v = word};
  u8 op2 = i.b.op2;
  u8 op = i.b.op1;
  if (!(op & 0b11110))
    return Instr::andShiftedReg;
  if ((op & 0b11110) == 0b10)
    return Instr::eorShiftedReg;
  if ((op & 0b11110) == 0b100)
    return Instr::subShiftedReg;
  if ((op & 0b11110) == 0b110)
    return Instr::rsbShiftedReg;
  if ((op & 0b11110) == 0b1000)
    return Instr::addShiftedReg;
  if ((op & 0b11110) == 0b1010)
    return Instr::adcShiftedReg;
  if ((op & 0b11110) == 0b1100)
    return Instr::sbcShiftedReg;
  if ((op & 0b11110) == 0b1110)
    return Instr::rscShiftedReg;

  if (op == 0b10001)
    return Instr::tstShiftedReg;
  if (op == 0b10011)
    return Instr::teqShiftedReg;
  if (op == 0b10101)
    return Instr::cmpShiftedReg;
  if (op == 0b10111)
    return Instr::cmnShiftedReg;

  if ((op & 0b11110) == 0b11000)
    return Instr::orrShiftedReg;

  if ((op & 0b11110) == 0b11010) {
    if (!op2)
      return Instr::lslReg;
    if (op2 == 1)
      return Instr::lsrReg;
    if (op2 == 0b10)
      return Instr::asrReg;
    if (op2 == 0b11)
      return Instr::rorReg;
  }

  if ((op & 0b11110) == 0b11100)
    return Instr::bicShiftedReg;
  if ((op & 0b11110) == 0b11110)
    return Instr::mvnShiftedReg;
  return Instr::undefined;
}
static Instr dataProcReg(u32 word) {
  union {
    struct {
      u32 _1 : 5;
      u32 op2 : 2;
      u32 imm5 : 5;
      u32 _2 : 8;
      u32 op : 5;
      u32 _3 : 7;
    } b;
    u32 v;
  } i = {.v = word};
  u8 op2 = i.b.op2;
  u8 op = i.b.op;
  u8 imm5 = i.b.imm5;
  if (!(op & 0b11110))
    return Instr::andReg;
  if ((op & 0b11110) == 0b10)
    return Instr::eorReg;
  if ((op & 0b11110) == 0b100)
    return Instr::subReg;
  if ((op & 0b11110) == 0b110)
    return Instr::rsbReg;
  if ((op & 0b11110) == 0b1000)
    return Instr::addReg;
  if ((op & 0b11110) == 0b1010)
    return Instr::adcReg;
  if ((op & 0b11110) == 0b1100)
    return Instr::sbcReg;
  if ((op & 0b11110) == 0b1110)
    return Instr::rscReg;

  if (op == 0b10001)
    return Instr::tstReg;
  if (op == 0b10011)
    return Instr::teqReg;
  if (op == 0b10101)
    return Instr::cmpReg;
  if (op == 0b10111)
    return Instr::cmnReg;

  if ((op & 0b11110) == 0b11000)
    return Instr::orrReg;

  if ((op & 0b11110) == 0b11010) {
    if (!op2 and !imm5)
      return Instr::movReg;
    if (!op2)
      return Instr::lslImm;
    if (op2 == 1)
      return Instr::lsrImm;
    if (op2 == 0b10)
      return Instr::asrImm;
    if (op2 == 0b11 and !imm5)
      return Instr::rrx;
    if (op2 == 0b11)
      return Instr::rorImm;
  }

  if ((op & 0b11110) == 0b11100)
    return Instr::bicReg;
  if ((op & 0b11110) == 0b11110)
    return Instr::mvnReg;

  return Instr::undefined;
}

static Instr syncPrim(u32 word) {
  u8 op = (word >> 20) & 0xf;
  if ((op & 0b1011) == 0)
    return Instr::swp;
  switch (op) {
  case 0b1000:
    return Instr::strex;
  case 0b1001:
    return Instr::ldrex;
  case 0b1010:
    return Instr::strexd;
  case 0b1011:
    return Instr::ldrexd;
  case 0b1100:
    return Instr::strexb;
  case 0b1101:
    return Instr::ldrexb;
  case 0b1110:
    return Instr::strexh;
  case 0b1111:
    return Instr::ldrexh;
  }
  return Instr::undefined;
}

static Instr extraLoadStore(u32 word) {
  u8 op2 = (word >> 5) & 0b11;
  u8 Rn = (word >> 16) & 0b1111;
  u8 op1 = (word >> 20) & 0b11111;
  switch (op2) {
  case 1: {
    if (!(op1 & 0b101))
      return Instr::strhReg;
    if ((op1 & 0b101) == 1)
      return Instr::ldrhReg;
    if ((op1 & 0b101) == 0b100)
      return Instr::strhImm;
    if ((op1 & 0b101) == 0b101 and Rn != 0xf)
      return Instr::ldrhImm;
    if ((op1 & 0b101) == 0b101)
      return Instr::ldrhLit;
    break;
  }
  case 2: {
    if (!(op1 & 0b101))
      return Instr::ldrdReg;
    if ((op1 & 0b101) == 1)
      return Instr::ldrsbReg;
    if ((op1 & 0b101) == 0b100 and Rn != 0xf)
      return Instr::ldrdImm;
    if ((op1 & 0b101) == 0b100)
      return Instr::ldrdLit;

    if ((op1 & 0b101) == 0b101 and Rn != 0xf)
      return Instr::ldrsbImm;
    if ((op1 & 0b101) == 0b101)
      return Instr::ldrsbLit;
    break;
  }
  case 3: {
    if (!(op1 & 0b101))
      return Instr::strd;
    if ((op1 & 0b101) == 1)
      return Instr::ldrsh;
    if ((op1 & 0b101) == 0b100)
      return Instr::strdImm;
    if ((op1 & 0b101) == 0b101 and Rn != 0xf)
      return Instr::ldrshImm;
    if ((op1 & 0b101) == 0b101)
      return Instr::ldrshLit;
    break;
  }
  }
  return Instr::undefined;
}

static Instr extraLoadStoreUnpriv(u32 word) {
  u8 op2 = (word >> 5) & 0b11;
  bool op = (word & (1 << 20)) != 0;
  if (op2 == 1 and !op and (word & (1 << 22)))
    return Instr::strht;
  if (op2 == 1 and !op)
    return Instr::strht2;
  if (op2 == 1)
    return Instr::ldrht;
  if (op2 == 0b10 and op)
    return Instr::ldrsbt;
  if (op2 == 0b11 and op)
    return Instr::ldrsht;
  return Instr::undefined;
}

static Instr dataProcImm(u32 word) {
  u8 op = (word >> 20) & 0b11111;
  u8 Rn = (word >> 16) & 0b1111;
  switch (op >> 1) {
  case 0:
    return Instr::andImm;
  case 1:
    return Instr::eorImm;
  case 2:
    if (Rn != 0xf)
      return Instr::subImm;
    else
      return Instr::adr;
  case 3:
    return Instr::rsbImm;
  case 4:
    if (Rn != 0xf)
      return Instr::addImm;
    else
      return Instr::adr;
  case 5:
    return Instr::adcImm;
  case 6:
    return Instr::sbcImm;
  case 7:
    return Instr::rscImm;
  }
  switch (op) {
  case 0b10001:
    return Instr::tstImm;
  case 0b10011:
    return Instr::teqImm;
  case 0b10101:
    return Instr::cmpImm;
  case 0b10111:
    return Instr::cmnImm;
  }
  switch (op >> 1) {
  case 0b1100:
    return Instr::orrImm;
  case 0b1101:
    return Instr::movImm;
  case 0b1110:
    return Instr::bicImm;
  case 0b1111:
    return Instr::mvnImm;
  }
  return Instr::undefined;
}

static Instr hints(u32 word) {
  bool op = (word & (1 << 22)) != 0;
  u8 op1 = (word >> 16) & 0b1111;
  u8 op2 = word;
  if (!op) {
    if (!op1) {
      if (!op2)
        return Instr::nop;
      if (op2 == 1)
        return Instr::yield;
      if (op2 == 2)
        return Instr::wfe;
      if (op2 == 3)
        return Instr::wfi;
      if (op2 == 4)
        return Instr::sev;
      if (op2 == 0b10100)
        return Instr::csdb;
      if ((op2 & 0b11110000) == 0b11110000)
        return Instr::dbg;
    }

    if (op1 == 0b100 or op1 == 0b1000 or op1 == 0b1100)
      return Instr::msrImmApp;
    if ((op1 & 0b11) == 0b1)
      return Instr::msrImmSys;
    if ((op1 & 0b10))
      return Instr::msrImmSys;
  } else {
    return Instr::msrImmSys;
  }
  return Instr::undefined;
}

static Instr dataProcAndMisc(u32 word) {
  union {
    struct {
      u32 _1 : 4;
      u32 op2 : 4;
      u32 _2 : 12;
      u32 op1 : 5;
      u32 op : 1;
      u32 _3 : 6;
    } b;
    u32 v;
  } i = {.v = word};
  u8 op = i.b.op;
  u8 op1 = i.b.op1;
  u8 op2 = i.b.op2;
  if (!op) {
    if ((op1 & 0b11001) != 0b10000 and !(op2 & 1)) {
      return dataProcReg(word);
    }
    if ((op1 & 0b11001) != 0b10000 and (op2 & 0b1001) == 1) {
      return dataProcShiftedReg(word);
    }
    if ((op1 & 0b11001) == 0b10000 and (op2 & 0b1000) == 0) {
      return misc(word);
    }
    if ((op1 & 0b11001) == 0b10000 and (op2 & 0b1001) == 0b1000) {
      return halfMult(word);
    }

    if (!(op1 & 0b10000) and op2 == 0b1001)
      return mult(word);
    if ((op1 & 0b10000) and op2 == 0b1001)
      return syncPrim(word);
    if ((op1 & 0b10010) != 0b10 and
        (op2 == 0b1011 or (op2 & 0b1101) == 0b1101)) {
      return extraLoadStore(word);
    }
    if ((op1 & 0b10011) == 0b10 and (op2 & 0b1101) == 0b1101) {
      return extraLoadStore(word);
    }

    if ((op1 & 0b10010) == 0b10 and op2 == 0b1011)
      return extraLoadStoreUnpriv(word);
    if ((op1 & 0b10011) == 0b11 and (op2 & 0b1101) == 0b1101)
      return extraLoadStoreUnpriv(word);
  } else {
    if ((op1 & 0b11001) != 0b10000)
      return dataProcImm(word);
    if (op1 == 0b10000)
      return Instr::movImm16;
    if (op1 == 0b10100)
      return Instr::movt;
    if ((op1 & 0b11011) == 0b10010)
      return hints(word);
  }

  return Instr::undefined;
}

static Instr loadStoreWordAndUnsignedByte(u32 word) {
  union {
    struct {
      u32 _1 : 4;
      u32 B : 1;
      u32 _2 : 11;
      // half
      u32 Rn : 4;
      u32 op1 : 5;
      u32 A : 1;
      u32 _3 : 2;
      u32 cond : 4;
    } b;
    u32 v;
  } instr = {.v = word};
  bool B = instr.b.B;
  bool A = instr.b.A;
  u8 Rn = instr.b.Rn;
  u8 op1 = instr.b.op1;
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
  union {
    struct {
      u32 _1 : 5;
      u32 op2 : 3;
      u32 _2 : 12;
      u32 op1 : 2;
      u32 _3 : 6;
      u32 cond : 4;
    } b;
    u32 v;
  } i = {.v = word};
  switch (i.b.op1) {
  case 0b1:
    switch (i.b.op2) {
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
    switch (i.b.op2) {
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
    switch (i.b.op2) {
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

static Instr parallelAddSubUnsigned(u32 word) {
  union {
    struct {
      u32 _1 : 5;
      u32 op2 : 3;
      u32 _2 : 12;
      u32 op1 : 2;
      u32 _3 : 6;
      u32 cond : 4;
    } b;
    u32 v;
  } i = {.v = word};
  switch (i.b.op1) {
  case 0b1:
    switch (i.b.op2) {
    case 0:
      return Instr::uadd16;
    case 1:
      return Instr::uasx;
    case 0b10:
      return Instr::usax;
    case 0b11:
      return Instr::usub16;
    case 0b100:
      return Instr::uadd8;
    case 0b111:
      return Instr::usub8;
    }
    break;
  case 0b10:
    switch (i.b.op2) {
    case 0:
      return Instr::uqadd16;
    case 1:
      return Instr::uqasx;
    case 0b10:
      return Instr::uqsax;
    case 0b11:
      return Instr::uqsub16;
    case 0b100:
      return Instr::uqadd8;
    case 0b111:
      return Instr::uqsub8;
    }
    break;
  case 0b11:
    switch (i.b.op2) {
    case 0:
      return Instr::uhadd16;
    case 1:
      return Instr::uhasx;
    case 0b10:
      return Instr::uhsax;
    case 0b11:
      return Instr::uhsub16;
    case 0b100:
      return Instr::uhadd8;
    case 0b111:
      return Instr::uhsub8;
    }
    break;
  }

  return Instr::undefined;
}

static Instr packingUnpacking(u32 word) {
  union {
    struct {
      u32 _1 : 5;
      u32 op2 : 3;
      u32 _2 : 8;
      u32 A : 4;
      u32 op1 : 3;
      u32 _3 : 5;
      u32 cond : 4;
    } b;
    u32 v;
  } i = {.v = word};
  u8 op1 = i.b.op1;
  u8 op2 = i.b.op2;
  u8 A = i.b.A;
  if (op1 == 0) {
    if (!(op2 & 1))
      return Instr::pkh;
    if (op2 == 0b11 && A != 0xf)
      return Instr::sxtab16;
    if (op2 == 0b11)
      return Instr::sxtb16;
    if (op2 == 0b101)
      return Instr::sel;
  }

  if ((op1 & 0b110) == 0b10 and !(op2 & 1))
    return Instr::ssat;

  if (op1 == 0b10) {
    if (op2 == 1)
      return Instr::ssat16;
    if (op2 == 0b11 and A != 0xf)
      return Instr::sxtab;
    if (op2 == 0b11)
      return Instr::sxtb;
  }

  if (op1 == 0b11) {
    if (op2 == 1)
      return Instr::rev;
    if (op2 == 0b11 and A != 0xf)
      return Instr::sxtah;
    if (op2 == 0b11)
      return Instr::sxth;
    if (op2 == 0b101)
      return Instr::rev16;
  }

  if (op1 == 0b100 and op2 == 0b11) {
    if (A != 0xf)
      return Instr::uxtab16;
    return Instr::uxtb16;
  }

  if ((op1 & 0b110) == 0b110 and !(op2 & 1))
    return Instr::usat;

  if (op1 == 0b110) {
    if (op2 == 1)
      return Instr::usat16;
    if (op2 == 0b11 and A != 0xf)
      return Instr::uxtab;
    if (op2 == 0b11)
      return Instr::uxtb;
  }

  if (op1 == 0b111) {
    if (op2 == 1)
      return Instr::rbit;
    if (op2 == 0b11 and A != 0xf)
      return Instr::uxtah;
    if (op2 == 0b11)
      return Instr::uxth;
    if (op2 == 0b101)
      return Instr::revsh;
  }

  return Instr::undefined;
}

static Instr signedMultDiv(u32 word) {
  union {
    struct {
      u32 _1 : 5;
      u32 op2 : 3;
      u32 _2 : 4;
      u32 A : 4;
      u32 _3 : 4;
      u32 op1 : 3;
      u32 _4 : 9;
    } b;
    u32 v;
  } i = {.v = word};
  u8 op2 = i.b.op2;
  u8 A = i.b.A;
  u8 op1 = i.b.op1;
  if (op1 == 0) {
    if ((op2 & 0b110) == 0 and A != 0xf)
      return Instr::smlad;
    if ((op2 & 0b110) == 0)
      return Instr::smuad;
    if ((op2 & 0b110) == 0b10 and A != 0xf)
      return Instr::smlsd;
    if ((op2 & 0b110) == 0b10)
      return Instr::smusd;
  }

  if (op1 == 0b1 and !op2)
    return Instr::sdiv;
  if (op1 == 0b11 and !op2)
    return Instr::udiv;

  if (op1 == 0b100 and (op2 & 0b110) == 0)
    return Instr::smlald;
  if (op1 == 0b100 and (op2 & 0b110) == 0b10)
    return Instr::smlsld;

  if (op1 == 0b101) {
    if (!(op2 & 0b110) and A != 0xf)
      return Instr::smmla;
    if (!(op2 & 0b110))
      return Instr::smmul;
    if ((op2 & 0b110) == 0b110)
      return Instr::smmls;
  }

  return Instr::undefined;
}

static Instr media(u32 word) {
  union {
    struct {
      u32 Rn : 4;
      u32 _1 : 1;
      u32 op2 : 3;
      u32 _2 : 4;
      u32 Rd : 4;
      u32 _3 : 4;
      u32 op1 : 5;
      u32 _4 : 3;
      u32 cond : 4;
    } b;
    u32 v;
  } i = {.v = word};
  u8 Rn = i.b.Rn;
  u8 op2 = i.b.op2;
  u8 Rd = i.b.Rd;
  u8 op1 = i.b.op1;

  if ((op1 & 0b11100) == 0) {
    return parallelAddSub(word);
  }

  if ((op1 & 0b11100) == 0b100) {
    return parallelAddSubUnsigned(word);
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
  union {
    struct {
      u32 _1 : 15;
      u32 R : 1;
      u32 Rn : 4;
      u32 op : 6;
      u32 _2 : 2;
      u32 cond : 4;
    } b;
    u32 v;
  } i = {.v = word};
  u8 R = i.b.R;
  u8 op = i.b.op;
  u8 Rn = i.b.Rn;
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
    if (i.b.cond == 0xf)
      return Instr::blx; // unreachable
    return Instr::bl;
  }
  return Instr::undefined;
}

static Instr coprocessorAndSVC(u32 word) {
  union {
    struct {
      u32 _1 : 4;
      u32 op : 1;
      u32 _2 : 3;
      u32 coproc : 4;
      u32 _3 : 4;
      u32 Rn : 4;
      u32 op1 : 6;
      u32 _4 : 2;
      u32 cond : 4;
    } b;
    u32 v;
  } i = {.v = word};
  u8 coproc = i.b.coproc;
  u8 op1 = i.b.op1;
  u8 op = i.b.op;
  u8 Rn = i.b.Rn;
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

union Gen {
  struct {
    u32 _1 : 4;
    u32 op : 1;
    u32 _2 : 20;
    u32 op1 : 3;
    u32 cond : 4;
  } b;
  u32 v;
};

static_assert(sizeof(Gen) == 4, "");

static Instr conditional(Gen instr) {
  switch (instr.b.op1) {
  case 0b10:
    return loadStoreWordAndUnsignedByte(instr.v);
  case 0b11:
    switch (instr.b.op) {
    case 0:
      return loadStoreWordAndUnsignedByte(instr.v);
    case 1:
      return media(instr.v);
    }
  default:
    switch ((instr.b.op1 >> 1) & 0b11) {
    case 0:
      return dataProcAndMisc(instr.v);
    case 2:
      return branchAndBlockDataTransfer(instr.v);
    case 3:
      return coprocessorAndSVC(instr.v);
    }
  }
  return Instr::undefined;
}

union Uncond {
  struct {
    u32 _1 : 4;
    u32 op : 1;
    u32 _2 : 11;
    // half
    u32 Rn : 4;
    u32 op1 : 8;
    u32 cond : 4;
  } b;
  u32 v;
};

static Instr memoryHints(Uncond uncond) {
  union {
    struct {
      u32 _1 : 4;
      u32 op2 : 4;
      u32 _2 : 8;
      u32 Rn : 4;
      u32 op1 : 8;
      u32 cond : 4;
    } b;
    u32 v;
  } memhint = {.v = uncond.v};

  u8 op1 = memhint.b.op1;
  u8 op2 = memhint.b.op2;
  u8 Rn = memhint.b.Rn;

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

static Instr unconditional(Gen i) {
  Uncond instr = {.v = i.v};
  u8 op1 = instr.b.op1;
  u8 op = instr.b.op;
  if (!(op1 & 128)) {
    return memoryHints(instr);
  }

  if ((op1 >> 5 == 0b100) and (op1 & 0b100) and !(op1 & 1)) {
    return Instr::srs;
  }

  if ((op1 >> 5 == 0b100) and !(op1 & 0b100) and (op1 & 1)) {
    return Instr::rfe;
  }

  if ((op1 >> 5 == 0b101)) {
    if (instr.b.cond == 0b1111)
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
    if (instr.b.Rn == 0b1111) {
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

  if (op1 >> 4 == 0b1110 and !op) {
    return Instr::cdp2;
  }

  if (op1 >> 4 == 0b1110 and !(op1 & 1) and op) {
    return Instr::mcr2;
  }

  if (op1 >> 4 == 0b1110 and (op1 & 1) and op) {
    return Instr::mrc2;
  }
  return Instr::undefined;
}

Instr decodeA(u32 instr) {
  const Gen gen = {.v = instr};
  Instr res;
  switch (gen.b.cond) {
  case 0b1111:
    res = unconditional(gen);
    break;
  default:
    res = conditional(gen);
  }
  return res;
}

static bool is32bit(u16 word) {
  if ((word & 0x1800) == 0)
    return false;
  return (word & 0xe000) == 0xe000;
}

Instr shiftImm16(u16 word) {
  u8 opcode = (word >> 9) & 0b11111;
  switch (opcode) {
  case 0b1100:
    return Instr::addregT1;
  case 0b1101:
    return Instr::subregT1;
  case 0b1110:
    return Instr::addimmT1;
  case 0b1111:
    return Instr::subimmT1;
  default:
    switch (opcode >> 2) {
    case 0:
      return Instr::lslimmT1;
    case 1:
      return Instr::lsrimmT1;
    case 2:
      return Instr::asrimmT1;
    case 4:
      return Instr::movimmT1;
    case 5:
      return Instr::cmpimmT1;
    case 6:
      return Instr::addimmT2;
    case 7:
      return Instr::subimmT2;
    }
  }
  return Instr::undefined;
}

Instr dataProc16(u16 word) {
  u8 opcode = (word >> 6) & 0b1111;
  switch (opcode) {
  case 0:
    return Instr::andregT1;
  case 1:
    return Instr::eorregT1;
  case 0b10:
    return Instr::lslregT1;
  case 0b11:
    return Instr::lsrregT1;
  case 0b100:
    return Instr::asrregT1;
  case 0b101:
    return Instr::adcregT1;
  case 0b110:
    return Instr::sbcregT1;
  case 0b111:
    return Instr::rorregT1;
  case 0b1000:
    return Instr::tstregT1;
  case 0b1001:
    return Instr::rsbimmT1;
  case 0b1010:
    return Instr::cmpregT1;
  case 0b1011:
    return Instr::cmnregT1;
  case 0b1100:
    return Instr::orrregT1;
  case 0b1101:
    return Instr::mulT1;
  case 0b1110:
    return Instr::bicregT1;
  case 0b1111:
    return Instr::mvnregT1;
  };
  return Instr::undefined;
}

Instr specDataBranch(u16 word) {
  u8 opcode = (word >> 6) & 0b1111;
  if (opcode == 0) {
    return Instr::addregT1;
  }
  if (opcode == 0b1)
    return Instr::addregT2;
  if (opcode & 0b10)
    return Instr::addregT2;
  if ((opcode & 0b1100) == 0b100)
    return Instr::cmpregT2;
  if (opcode == 0b1000 or opcode == 0b1001)
    return Instr::movregT1;
  if ((opcode >> 1) == 0b101)
    return Instr::movregT1;
  if ((opcode >> 1) == 0b110)
    return Instr::bxT1;
  if ((opcode >> 1) == 0b111)
    return Instr::blxT1;
  return Instr::undefined;
}

Instr loadStoreSingle16(u16 word) {
  switch (word >> 9) {
  case 0b101000:
    return Instr::strregT1;
  case 0b101001:
    return Instr::strhregT1;
  case 0b101010:
    return Instr::strbregT1;
  case 0b101011:
    return Instr::ldrsbregT1;
  case 0b101100:
    return Instr::ldrregT1;
  case 0b101101:
    return Instr::ldrhregT1;
  case 0b101110:
    return Instr::ldrbregT1;
  case 0b101111:
    return Instr::ldrshregT1;
  default:
    switch (word >> 11) {
    case 0b1100:
      return Instr::strimmT1;
    case 0b1101:
      return Instr::ldrimmT1;
    case 0b1110:
      return Instr::strbimmT1;
    case 0b1111:
      return Instr::ldrbimmT1;
    case 0b10000:
      return Instr::strhimmT1;
    case 0b10001:
      return Instr::ldrhimmT1;
    case 0b10010:
      return Instr::strimmT2;
    case 0b10011:
      return Instr::ldrimmT2;
    }
  }
  return Instr::undefined;
}

Instr misc16(u16 word) {
  u8 opcode = (word >> 5) & 0x7f;
  if ((opcode >> 3) == 0b1111) {
    if (word & 0xf) {
      return Instr::itT1;
    } else {
      switch ((word >> 4) & 0xf) {
      case 0:
        return Instr::nopT1;
      case 1:
        return Instr::yieldT1;
      case 2:
        return Instr::wfeT1;
      case 3:
        return Instr::wfiT1;
      case 4:
        return Instr::sevT1;
      }
    }
  } else if (opcode == 0b110011) {
    return Instr::cps;
  } else if (opcode == 0b110010) {
    return Instr::setendT1;
  } else if (opcode >> 3 == 0b1110) {
    return Instr::bkptT1;
  } else if (opcode >> 2 == 0) {
    return Instr::addspimmT2;
  } else if (opcode >> 2 == 1) {
    return Instr::subspimmT1;
  } else if (opcode >> 1 == 0b001000) {
    return Instr::sxthT1;
  } else if (opcode >> 1 == 0b001001) {
    return Instr::sxtbT1;
  } else if (opcode >> 1 == 0b001010) {
    return Instr::uxthT1;
  } else if (opcode >> 1 == 0b001011) {
    return Instr::uxtbT1;
  } else if (opcode >> 1 == 0b101000) {
    return Instr::revT1;
  } else if (opcode >> 1 == 0b101001) {
    return Instr::rev16T1;
  } else if (opcode >> 1 == 0b101011) {
    return Instr::revshT1;
  } else if (opcode >> 4 == 0b010) {
    return Instr::pushT1;
  } else if (opcode >> 4 == 0b110) {
    return Instr::popT1;
  } else if ((opcode >> 3) == 0b0001 or //
             (opcode >> 3) == 0b0011 or //
             (opcode >> 3) == 0b1001 or //
             (opcode >> 3) == 0b1011) {
    return Instr::cbzT1;
  }
  return Instr::undefined;
}

Instr condBranch16(u16 word) {
  u8 opcode = (word >> 8) & 0xf;
  switch (opcode) {
  case 0b1110:
    return Instr::undefined;
  case 0b1111:
    return Instr::svcT1;
  default:
    return Instr::bT1;
  }
  return Instr::undefined;
}

Instr T32(u32 word) { return Instr::undefined; }

Instr T16(u16 word) {
  u8 opcode = word >> 10;
  if ((opcode >> 7) == 0) {
    return shiftImm16(word);
  }
  if (opcode == 0b10000) {
    return dataProc16(word);
  }
  if (opcode == 0b10001) {
    return specDataBranch(word);
  }
  if ((opcode >> 1) == 0b1001) {
    return Instr::ldrlitT1;
  }
  if ((opcode >> 1) == 0b10100) {
    return Instr::adrT1;
  }
  if ((opcode >> 1) == 0b10101) {
    return Instr::addspimmT1;
  }
  if ((opcode >> 1) == 0b11000) {
    return Instr::stmT1;
  }
  if ((opcode >> 1) == 0b11001) {
    return Instr::ldmT1;
  }
  if ((opcode >> 1) == 0b11100) {
    return Instr::bT2;
  }
  if ((opcode >> 2) == 0b101)
    return loadStoreSingle16(word);
  if ((opcode >> 3) == 0b11)
    return loadStoreSingle16(word);
  if ((opcode >> 3) == 0b100)
    return loadStoreSingle16(word);
  if ((opcode >> 2) == 0b1011)
    return misc16(word);
  if ((opcode >> 2) == 0b1101)
    return condBranch16(word);
  return Instr::undefined;
}

Instr decodeT(u32 word) {
  if (is32bit(word)) {
    printf("THUMB32\n");
  } else {
    printf("THUMB16\n");
    return T16(word);
  }
  return Instr::undefined;
}

int printInstr(Instr instr) {
  switch (instr) {
  case Instr::cpsT1:
    return printf("Instr::cpsT1\n");
  case Instr::addspimmT2:
    return printf("Instr::addspimmT2\n");
  case Instr::subspimmT1:
    return printf("Instr::subspimmT1\n");
  case Instr::sxthT1:
    return printf("Instr::sxthT1\n");
  case Instr::sxtbT1:
    return printf("Instr::sxtbT1\n");
  case Instr::uxthT1:
    return printf("Instr::uxthT1\n");
  case Instr::uxtbT1:
    return printf("Instr::uxtbT1\n");
  case Instr::revT1:
    return printf("Instr::revT1\n");
  case Instr::rev16T1:
    return printf("Instr::rev16T1\n");
  case Instr::revshT1:
    return printf("Instr::revshT1\n");
  case Instr::pushT1:
    return printf("Instr::pushT1\n");
  case Instr::popT1:
    return printf("Instr::popT1\n");
  case Instr::cbzT1:
    return printf("Instr::cbzT1\n");
  case Instr::bkptT1:
    return printf("Instr::bkptT1\n");
  case Instr::setendT1:
    return printf("Instr::setendT1\n");
  case Instr::svcT1:
    return printf("Instr::svcT1\n");
  case Instr::bT1:
    return printf("Instr::bT1\n");
  case Instr::nopT1:
    return printf("Instr::nopT1\n");
  case Instr::yieldT1:
    return printf("Instr::yieldT1\n");
  case Instr::wfeT1:
    return printf("Instr::wfeT1\n");
  case Instr::wfiT1:
    return printf("Instr::wfiT1\n");
  case Instr::sevT1:
    return printf("Instr::sevT1\n");
  case Instr::itT1:
    return printf("Instr::itT1\n");
  case Instr::strregT1:
    return printf("Instr::strregT1\n");
  case Instr::strhregT1:
    return printf("Instr::strhregT1\n");
  case Instr::strbregT1:
    return printf("Instr::strbregT1\n");
  case Instr::ldrsbregT1:
    return printf("Instr::ldrsbregT1\n");
  case Instr::ldrregT1:
    return printf("Instr::ldrregT1\n");
  case Instr::ldrhregT1:
    return printf("Instr::ldrhregT1\n");
  case Instr::ldrbregT1:
    return printf("Instr::ldrbregT1\n");
  case Instr::ldrshregT1:
    return printf("Instr::ldrshregT1\n");
  case Instr::strimmT1:
    return printf("Instr::strimmT1\n");
  case Instr::ldrimmT1:
    return printf("Instr::ldrimmT1\n");
  case Instr::strbimmT1:
    return printf("Instr::strbimmT1\n");
  case Instr::ldrbimmT1:
    return printf("Instr::ldrbimmT1\n");
  case Instr::strhimmT1:
    return printf("Instr::strhimmT1\n");
  case Instr::ldrhimmT1:
    return printf("Instr::ldrhimmT1\n");
  case Instr::strimmT2:
    return printf("Instr::strimmT2\n");
  case Instr::ldrimmT2:
    return printf("Instr::ldrimmT2\n");
  case Instr::ldrlitT1:
    return printf("Instr::ldrlitT1\n");
  case Instr::adrT1:
    return printf("Instr::adrT1\n");
  case Instr::addspimmT1:
    return printf("Instr::addspimmT1\n");
  case Instr::stmT1:
    return printf("Instr::stmT1\n");
  case Instr::ldmT1:
    return printf("Instr::ldmT1\n");
  case Instr::bT2:
    return printf("Instr::bT2\n");
  case Instr::addregT2:
    return printf("Instr::addregT2\n");
  case Instr::cmpregT2:
    return printf("Instr::cmpregT2\n");
  case Instr::movregT1:
    return printf("Instr::movregT1\n");
  case Instr::movregT2:
    return printf("Instr::movregT2\n");
  case Instr::bxT1:
    return printf("Instr::bxT1\n");
  case Instr::blxT1:
    return printf("Instr::blxT1\n");
  case Instr::andregT1:
    return printf("Instr::andregT1\n");
  case Instr::eorregT1:
    return printf("Instr::eorregT1\n");
  case Instr::lslregT1:
    return printf("Instr::lslregT1\n");
  case Instr::lsrregT1:
    return printf("Instr::lsrregT1\n");
  case Instr::asrregT1:
    return printf("Instr::asrregT1\n");
  case Instr::adcregT1:
    return printf("Instr::adcregT1\n");
  case Instr::sbcregT1:
    return printf("Instr::sbcregT1\n");
  case Instr::rorregT1:
    return printf("Instr::rorregT1\n");
  case Instr::tstregT1:
    return printf("Instr::tstregT1\n");
  case Instr::rsbimmT1:
    return printf("Instr::rsbimmT1\n");
  case Instr::cmpregT1:
    return printf("Instr::cmpregT1\n");
  case Instr::cmnregT1:
    return printf("Instr::cmnregT1\n");
  case Instr::orrregT1:
    return printf("Instr::orrregT1");
  case Instr::mulT1:
    return printf("Instr::mulT1\n");
  case Instr::bicregT1:
    return printf("Instr::bicregT1\n");
  case Instr::mvnregT1:
    return printf("Instr::mvnregT1\n");
  case Instr::lslimmT1:
    return printf("Instr::lslimmT1\n");
  case Instr::lsrimmT1:
    return printf("Instr::lsrimmT1\n");
  case Instr::asrimmT1:
    return printf("Instr::asrimmT1\n");
  case Instr::addregT1:
    return printf("Instr::addregT1\n");
  case Instr::subregT1:
    return printf("Instr::subregT1\n");
  case Instr::addimmT1:
    return printf("Instr::addimmT1\n");
  case Instr::subimmT1:
    return printf("Instr::subimmT1\n");
  case Instr::movimmT1:
    return printf("Instr::movimmT1\n");
  case Instr::cmpimmT1:
    return printf("Instr::cmpimmT1\n");
  case Instr::addimmT2:
    return printf("Instr::addimmT2\n");
  case Instr::subimmT2:
    return printf("Instr::subimmT2\n");
  case Instr::strd:
    return printf("Instr::strd\n");
  case Instr::ldrsh:
    return printf("Instr::ldrsh\n");
  case Instr::strdImm:
    return printf("Instr::strdImm\n");
  case Instr::ldrshImm:
    return printf("Instr::ldrshImm\n");
  case Instr::ldrshLit:
    return printf("Instr::ldrshLit\n");
  case Instr::yield:
    return printf("Instr::yield\n");
  case Instr::wfe:
    return printf("Instr::wfe\n");
  case Instr::wfi:
    return printf("Instr::wfi\n");
  case Instr::sev:
    return printf("Instr::sev\n");
  case Instr::csdb:
    return printf("Instr::csdb\n");
  case Instr::dbg:
    return printf("Instr::dbg\n");
  case Instr::msrImmApp:
    return printf("Instr::msrImmApp\n");
  case Instr::movt:
    return printf("Instr::movt\n");
  case Instr::msrImmSys:
    return printf("Instr::msrImmSys\n");
  case Instr::movImm16:
    return printf("Instr::movImm16\n");
  case Instr::eorImm:
    return printf("Instr::eorImm\n");
  case Instr::subImm:
    return printf("Instr::subImm\n");
  case Instr::adr:
    return printf("Instr::adr\n");
  case Instr::rsbImm:
    return printf("Instr::rsbImm\n");
  case Instr::addImm:
    return printf("Instr::addImm\n");
  case Instr::adcImm:
    return printf("Instr::adcImm\n");
  case Instr::sbcImm:
    return printf("Instr::sbcImm\n");
  case Instr::rscImm:
    return printf("Instr::rscImm\n");
  case Instr::tstImm:
    return printf("Instr::tstImm\n");
  case Instr::teqImm:
    return printf("Instr::teqImm\n");
  case Instr::cmpImm:
    return printf("Instr::cmpImm\n");
  case Instr::cmnImm:
    return printf("Instr::cmnImm\n");
  case Instr::orrImm:
    return printf("Instr::orrImm\n");
  case Instr::movImm:
    return printf("Instr::movImm\n");
  case Instr::bicImm:
    return printf("Instr::bicImm\n");
  case Instr::mvnImm:
    return printf("Instr::mvnImm\n");
  case Instr::andImm:
    return printf("Instr::andImm\n");
  case Instr::strht:
    return printf("Instr::strht\n");
  case Instr::strht2:
    return printf("Instr::strht2\n");
  case Instr::ldrht:
    return printf("Instr::ldrht\n");
  case Instr::ldrsbt:
    return printf("Instr::ldrsbt\n");
  case Instr::ldrsht:
    return printf("Instr::ldrsht\n");
  case Instr::strhReg:
    return printf("Instr::strhReg\n");
  case Instr::ldrhReg:
    return printf("Instr::ldrhReg\n");
  case Instr::strhImm:
    return printf("Instr::strhImm\n");
  case Instr::ldrhImm:
    return printf("Instr::ldrhImm\n");
  case Instr::ldrhLit:
    return printf("Instr::ldrhLit\n");
  case Instr::ldrdReg:
    return printf("Instr::ldrdReg\n");
  case Instr::ldrsbReg:
    return printf("Instr::ldrsbReg\n");
  case Instr::ldrdImm:
    return printf("Instr::ldrdImm\n");
  case Instr::ldrdLit:
    return printf("Instr::ldrdLit\n");
  case Instr::ldrsbImm:
    return printf("Instr::ldrsbImm\n");
  case Instr::ldrsbLit:
    return printf("Instr::ldrsbLit\n");
  case Instr::swp:
    return printf("Instr::swp\n");
  case Instr::strex:
    return printf("Instr::strex\n");
  case Instr::ldrex:
    return printf("Instr::ldrex\n");
  case Instr::strexd:
    return printf("Instr::strexd\n");
  case Instr::ldrexd:
    return printf("Instr::ldrexd\n");
  case Instr::strexb:
    return printf("Instr::strexb\n");
  case Instr::ldrexb:
    return printf("Instr::ldrexb\n");
  case Instr::strexh:
    return printf("Instr::strexh\n");
  case Instr::ldrexh:
    return printf("Instr::ldrexh\n");
  case Instr::mul:
    return printf("Instr::mul\n");
  case Instr::mla:
    return printf("Instr::mla\n");
  case Instr::umaal:
    return printf("Instr::umaal\n");
  case Instr::mls:
    return printf("Instr::mls\n");
  case Instr::umull:
    return printf("Instr::umull\n");
  case Instr::umlal:
    return printf("Instr::umlal\n");
  case Instr::smull:
    return printf("Instr::smull\n");
  case Instr::smlal:
    return printf("Instr::smlal\n");
  case Instr::smlabb:
    return printf("Instr::smlabb\n");
  case Instr::smlawb:
    return printf("Instr::smlawb\n");
  case Instr::smulwb:
    return printf("Instr::smulwb\n");
  case Instr::smlalbb:
    return printf("Instr::smlalbb\n");
  case Instr::smulbb:
    return printf("Instr::smulbb\n");
  case Instr::qadd:
    return printf("Instr::qadd\n");
  case Instr::qsub:
    return printf("Instr::qsub\n");
  case Instr::qdadd:
    return printf("Instr::qdadd\n");
  case Instr::qdsub:
    return printf("Instr::qdsub\n");
  case Instr::bx:
    return printf("Instr::bx\n");
  case Instr::clz:
    return printf("Instr::clz\n");
  case Instr::bxj:
    return printf("Instr::bxj\n");
  case Instr::blxReg:
    return printf("Instr::blxReg\n");
  case Instr::eret:
    return printf("Instr::eret\n");
  case Instr::bkpt:
    return printf("Instr::bkpt\n");
  case Instr::hvc:
    return printf("Instr::hvc\n");
  case Instr::smc:
    return printf("Instr::smc\n");
  case Instr::mrsBanked:
    return printf("Instr::mrsBanked\n");
  case Instr::msrBanked:
    return printf("Instr::msrBanked\n");
  case Instr::mrs:
    return printf("Instr::mrs\n");
  // case Instr::msr:
  //   return printf("Instr::msr\n");
  case Instr::msrApp:
    return printf("Instr::msrApp\n");
  case Instr::msrSys:
    return printf("Instr::msrSys\n");
  case Instr::orrShiftedReg:
    return printf("Instr::orrShiftedReg\n");
  case Instr::lslReg:
    return printf("Instr::lslReg\n");
  case Instr::lsrReg:
    return printf("Instr::lsrReg\n");
  case Instr::asrReg:
    return printf("Instr::asrReg\n");
  case Instr::rorReg:
    return printf("Instr::rorReg\n");
  case Instr::bicShiftedReg:
    return printf("Instr::bicShiftedReg\n");
  case Instr::mvnShiftedReg:
    return printf("Instr::mvnShiftedReg\n");
  case Instr::eorShiftedReg:
    return printf("Instr::eorShiftedReg\n");
  case Instr::subShiftedReg:
    return printf("Instr::subShiftedReg\n");
  case Instr::rsbShiftedReg:
    return printf("Instr::rsbShiftedReg\n");
  case Instr::addShiftedReg:
    return printf("Instr::addShiftedReg\n");
  case Instr::adcShiftedReg:
    return printf("Instr::adcShiftedReg\n");
  case Instr::sbcShiftedReg:
    return printf("Instr::sbcShiftedReg\n");
  case Instr::rscShiftedReg:
    return printf("Instr::rscShiftedReg\n");
  case Instr::tstShiftedReg:
    return printf("Instr::tstShiftedReg\n");
  case Instr::teqShiftedReg:
    return printf("Instr::teqShiftedReg\n");
  case Instr::cmpShiftedReg:
    return printf("Instr::cmpShiftedReg\n");
  case Instr::cmnShiftedReg:
    return printf("Instr::cmnShiftedReg\n");
  case Instr::andShiftedReg:
    return printf("Instr::andShiftedReg\n");
  case Instr::asrImm:
    return printf("Instr::asrImm\n");
  case Instr::andReg:
    return printf("Instr::andReg\n");
  case Instr::eorReg:
    return printf("Instr::eorReg\n");
  case Instr::subReg:
    return printf("Instr::subReg\n");
  case Instr::rsbReg:
    return printf("Instr::rsbReg\n");
  case Instr::addReg:
    return printf("Instr::addReg\n");
  case Instr::adcReg:
    return printf("Instr::adcReg\n");
  case Instr::sbcReg:
    return printf("Instr::sbcReg\n");
  case Instr::rscReg:
    return printf("Instr::rscReg\n");
  case Instr::tstReg:
    return printf("Instr::tstReg\n");
  case Instr::teqReg:
    return printf("Instr::teqReg\n");
  case Instr::cmpReg:
    return printf("Instr::cmpReg\n");
  case Instr::cmnReg:
    return printf("Instr::cmnReg\n");
  case Instr::orrReg:
    return printf("Instr::orrReg\n");
  case Instr::movReg:
    return printf("Instr::movReg\n");
  case Instr::lslImm:
    return printf("Instr::lslImm\n");
  case Instr::lsrImm:
    return printf("Instr::lsrImm\n");
  case Instr::rrx:
    return printf("Instr::rrx\n");
  case Instr::rorImm:
    return printf("Instr::rorImm\n");
  case Instr::bicReg:
    return printf("Instr::bicReg\n");
  case Instr::mvnReg:
    return printf("Instr::mvnReg\n");
  case Instr::smlad:
    return printf("Instr::smlad\n");
  case Instr::smuad:
    return printf("Instr::smuad\n");
  case Instr::smlsd:
    return printf("Instr::smlsd\n");
  case Instr::smusd:
    return printf("Instr::smusd\n");
  case Instr::sdiv:
    return printf("Instr::sdiv\n");
  case Instr::smlald:
    return printf("Instr::smlald\n");
  case Instr::smlsld:
    return printf("Instr::smlsld\n");
  case Instr::smmla:
    return printf("Instr::smmla\n");
  case Instr::smmul:
    return printf("Instr::smmul\n");
  case Instr::smmls:
    return printf("Instr::smmls\n");
  case Instr::udiv:
    return printf("Instr::udiv\n");
  case Instr::revsh:
    return printf("Instr::revsh\n");
  case Instr::uxth:
    return printf("Instr::uxth\n");
  case Instr::uxtah:
    return printf("Instr::uxtah\n");
  case Instr::rbit:
    return printf("Instr::rbit\n");
  case Instr::uxtb:
    return printf("Instr::uxtb\n");
  case Instr::uxtab:
    return printf("Instr::uxtab\n");
  case Instr::usat16:
    return printf("Instr::usat16\n");
  case Instr::usat:
    return printf("Instr::usat\n");
  case Instr::uxtb16:
    return printf("Instr::uxtb16\n");
  case Instr::uxtab16:
    return printf("Instr::uxtab16\n");
  case Instr::rev16:
    return printf("Instr::rev16\n");
  case Instr::sxth:
    return printf("Instr::sxth\n");
  case Instr::sxtah:
    return printf("Instr::sxtah\n");
  case Instr::rev:
    return printf("Instr::rev\n");
  case Instr::sxtb:
    return printf("Instr::sxtb\n");
  case Instr::sxtab:
    return printf("Instr::sxtab\n");
  case Instr::ssat16:
    return printf("Instr::ssat16\n");
  case Instr::ssat:
    return printf("Instr::ssat\n");
  case Instr::sel:
    return printf("Instr::sel\n");
  case Instr::sxtb16:
    return printf("Instr::sxtb16\n");
  case Instr::sxtab16:
    return printf("Instr::sxtab16\n");
  case Instr::pkh:
    return printf("Instr::pkh\n");
  case Instr::qsub8:
    return printf("Instr::qsub8\n");
  case Instr::qadd8:
    return printf("Instr::qadd8\n");
  case Instr::qsub16:
    return printf("Instr::qsub16\n");
  case Instr::qsax:
    return printf("Instr::qsax\n");
  case Instr::qasx:
    return printf("Instr::qasx\n");
  case Instr::qadd16:
    return printf("Instr::qadd16\n");

  case Instr::shsub8:
    return printf("Instr::shsub8\n");
  case Instr::shadd8:
    return printf("Instr::shadd8\n");
  case Instr::shsub16:
    return printf("Instr::shsub16\n");
  case Instr::shsax:
    return printf("Instr::shsax\n");
  case Instr::shasx:
    return printf("Instr::shasx\n");
  case Instr::shadd16:
    return printf("Instr::shadd16\n");

  case Instr::ssub8:
    return printf("Instr::ssub8\n");
  case Instr::sadd8:
    return printf("Instr::sadd8\n");
  case Instr::ssub16:
    return printf("Instr::ssub16\n");
  case Instr::ssax:
    return printf("Instr::ssax\n");
  case Instr::sasx:
    return printf("Instr::sasx\n");
  case Instr::sadd16:
    return printf("Instr::sadd16\n");
  case Instr::ubfx:
    return printf("Instr::ubfx\n");
  case Instr::bfi:
    return printf("Instr::bfi\n");
  case Instr::bfc:
    return printf("instr::bfc\n");
  case Instr::sbfx:
    return printf("Instr::sbfx\n");
  case Instr::usad8:
    return printf("Instr::usad8\n");
  case Instr::usada8:
    return printf("Instr::usada8\n");
  case Instr::mcrr1:
    return printf("Instr::mcrr1\n");
  case Instr::mrrc1:
    return printf("Instr::mrrc1\n");
  case Instr::cdp1:
    return printf("Instr::cdp1\n");
  case Instr::mcr1:
    return printf("Instr::mcr1\n");
  case Instr::mrc1:
    return printf("Instr::mrc1\n");

  case Instr::ldc1Lit:
    return printf("Instr::ldc1Lit\n");
  case Instr::ldc1Imm:
    return printf("Instr::ldc1Imm\n");
  case Instr::stc1:
    return printf("Instr::stc1\n");
  case Instr::svc:
    return printf("Instr::svc\n");
  case Instr::b:
    return printf("Instr::b\n");
  case Instr::ldmExRet:
    return printf("Instr::ldmExRet\n");
  case Instr::stmUser:
    return printf("Instr::stmUser\n");
  case Instr::ldmUser:
    return printf("Instr::ldmUser\n");
  case Instr::ldmib:
    return printf("Instr::ldmib\n");
  case Instr::stmib:
    return printf("Instr::stmib\n");
  case Instr::ldmdb:
    return printf("Instr::ldmdb\n");
  case Instr::push:
    return printf("Instr::push\n");
  case Instr::stmdb:
    return printf("Instr::stmdb\n");
  case Instr::pop:
    return printf("Instr::pop\n");
  case Instr::ldm:
    return printf("Instr::ldm\n");
  case Instr::stm:
    return printf("Instr::stm\n");
  case Instr::ldmda:
    return printf("Instr::ldmda\n");
  case Instr::stmda:
    return printf("Instr::stmda\n");
  case Instr::srs:
    return printf("Instr::srs\n");
  case Instr::rfe:
    return printf("Instr::rfe\n");
  case Instr::bl:
    return printf("Instr::bl\n");
  case Instr::blx:
    return printf("Instr::blx\n");
  case Instr::stc2:
    return printf("Instr::stc2\n");
  case Instr::ldc2Imm:
    return printf("Instr::ldc2Imm\n");
  case Instr::ldc2Lit:
    return printf("Instr::ldcLit\n");
  case Instr::mcrr2:
    return printf("Instr::mcrr2\n");
  case Instr::mrrc2:
    return printf("Instr::mrrc2\n");
  case Instr::cdp2:
    return printf("Instr::cdp2\n");
  case Instr::mcr2:
    return printf("Instr::mcr2\n");
  case Instr::mrc2:
    return printf("Instr::mrc2\n");
  case Instr::cps:
    return printf("Instr::cps\n");
  case Instr::setend:
    return printf("Instr::setend\n");
  case Instr::unpredictable:
    return printf("Instr::unpredictable\n");
  case Instr::nop:
    return printf("Instr::nop\n");
  case Instr::pliImm:
    return printf("Instr::pliImm\n");
  case Instr::pldImm:
    return printf("Instr::pldImm\n");
  case Instr::pldLit:
    return printf("Instr::pldLit\n");
  case Instr::clrex:
    return printf("Instr::clrex\n");
  case Instr::dsb:
    return printf("Instr::dsb\n");
  case Instr::dmb:
    return printf("Instr::dmb\n");
  case Instr::isb:
    return printf("Instr::isb\n");
  case Instr::pliReg:
    return printf("Instr::pliReg\n");
  case Instr::pldReg:
    return printf("Instr::pldReg\n");
  case Instr::undefined:
    return printf("Instr::undefined\n");
  case Instr::strImm:
    return printf("Instr::strImm\n");
  case Instr::strReg:
    return printf("Instr::strReg\n");
  case Instr::strt1:
    return printf("Instr::strt1\n");
  case Instr::strt2:
    return printf("Instr::strt2\n");
  case Instr::ldrImm:
    return printf("Instr::ldrImm\n");
  case Instr::ldrLit:
    return printf("Instr::ldrLit\n");
  case Instr::ldrReg:
    return printf("Instr::ldrReg\n");
  case Instr::ldrt1:
    return printf("Instr::ldrt1\n");
  case Instr::ldrt2:
    return printf("Instr::ldrt2\n");
  case Instr::strbImm:
    return printf("Instr::strbImm\n");
  case Instr::strbReg:
    return printf("Instr::strbReg\n");
  case Instr::strbt1:
    return printf("Instr::strbt1\n");
  case Instr::strbt2:
    return printf("Instr::strbt2\n");
  case Instr::ldrbImm:
    return printf("Instr::ldrbImm\n");
  case Instr::ldrbLit:
    return printf("Instr::ldrbLit\n");
  case Instr::ldrbReg:
    return printf("Instr::ldrbReg\n");
  case Instr::ldrbt1:
    return printf("Instr::ldrbt1\n");
  case Instr::ldrbt2:
    return printf("Instr::ldrbt2\n");
  case Instr::uadd16:
    return printf("Instr::uadd16\n");
  case Instr::uasx:
    return printf("Instr::uasx\n");
  case Instr::usax:
    return printf("Instr::usax\n");
  case Instr::usub16:
    return printf("Instr::usub16\n");
  case Instr::uadd8:
    return printf("Instr::uadd8\n");
  case Instr::usub8:
    return printf("Instr::usub8\n");
  case Instr::uqadd16:
    return printf("Instr::uqadd16\n");
  case Instr::uqasx:
    return printf("Instr::uqasx\n");
  case Instr::uqsax:
    return printf("Instr::uqsax\n");
  case Instr::uqsub16:
    return printf("Instr::uqsub16\n");
  case Instr::uqadd8:
    return printf("Instr::uqadd8\n");
  case Instr::uqsub8:
    return printf("Instr::uqsub8\n");
  case Instr::uhadd16:
    return printf("Instr::uhadd16\n");
  case Instr::uhasx:
    return printf("Instr::uhasx\n");
  case Instr::uhsax:
    return printf("Instr::uhsax\n");
  case Instr::uhsub16:
    return printf("Instr::uhsub16\n");
  case Instr::uhadd8:
    return printf("Instr::uhadd8\n");
  case Instr::uhsub8:
    return printf("Instr::uhsub8\n");
  }

  return printf("Unknown instruction\n");
}

#ifdef TESTING
struct Pair {
  Instr instr;
  u32 mask;
};

constexpr u32 makeMask() { return 0; }

template <typename... Bits> constexpr u32 makeMask(int bit, Bits... bits) {
  return (1u << bit) | makeMask(bits...);
}

Pair pairs[] = {
    {Instr::adcImm, makeMask(21, 23, 25)},
    {Instr::adcReg, makeMask(21, 23)},
    {Instr::adcShiftedReg, makeMask(4, 21, 23)},
    {Instr::addImm, makeMask(23, 25)},
    {Instr::addReg, makeMask(23)},
    {Instr::addShiftedReg, makeMask(4, 23)},
    {Instr::adr, makeMask(25, 23, 19, 18, 17, 16)},
    {Instr::adr, makeMask(25, 22, 16, 17, 18, 19)},
    {Instr::andImm, makeMask(25)},
    {Instr::andReg, makeMask()},
    {Instr::andShiftedReg, makeMask(4)},
    {Instr::asrImm, makeMask(24, 23, 21, 6)},
    {Instr::asrReg, makeMask(24, 23, 21, 6, 4)},
    {Instr::b, makeMask(27, 25)},
    {Instr::bfc, makeMask(0, 1, 2, 3, 4, 22, 23, 24, 25, 26)},
    {Instr::bfi, makeMask(4, 22, 23, 24, 25, 26)},
    {Instr::bicImm, makeMask(22, 23, 24, 25)},
    {Instr::bicReg, makeMask(22, 23, 24)},
    {Instr::bicShiftedReg, makeMask(22, 23, 24, 4)},
    {Instr::bkpt, makeMask(4, 5, 6, 21, 24)},
    {Instr::bl, makeMask(24, 25, 27)},
    {Instr::blx, makeMask(25, 27, 28, 29, 30, 31)},
    {Instr::blxReg,
     makeMask(21, 24, 16, 17, 18, 19, 12, 13, 14, 15, 8, 9, 10, 11, 4, 5)},
    {Instr::bx,
     makeMask(4, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 24)},
    {Instr::bxj,
     makeMask(5, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 24)},
    {Instr::cdp1, makeMask(25, 26, 27)},
    {Instr::cdp2, makeMask(25, 26, 27, 28, 29, 30, 31)},
    {Instr::clrex, makeMask(1, 2, 3, 4, 0, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                            21, 22, 24, 26, 28, 29, 30, 31)},
    {Instr::clz, makeMask(4, 8, 9, 10, 11, 16, 17, 18, 19, 21, 22, 24)},
    {Instr::cmnImm, makeMask(20, 21, 22, 24, 25)},
    {Instr::cmnReg, makeMask(20, 21, 22, 24)},
    {Instr::cmnShiftedReg, makeMask(4, 20, 21, 22, 24)},
    {Instr::cmpImm, makeMask(20, 22, 24, 25)},
    {Instr::cmpReg, makeMask(20, 22, 24)},
    {Instr::cmpShiftedReg, makeMask(4, 20, 22, 24)},
    {Instr::cps, makeMask(7, 24, 28, 29, 30, 31)},
    {Instr::csdb, makeMask(4, 2, 12, 13, 14, 15, 21, 24, 25)},
    {Instr::dbg, makeMask(4, 5, 6, 7, 12, 13, 14, 15, 21, 24, 25)},
    {Instr::dmb, makeMask(4, 6, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24,
                          26, 28, 29, 30, 31)},
    {Instr::dsb, makeMask(6, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 26,
                          28, 29, 30, 31)},
    {Instr::eorImm, makeMask(21, 25)},
    {Instr::eorReg, makeMask(21)},
    {Instr::eorShiftedReg, makeMask(21, 4)},
    {Instr::eret, makeMask(5, 6, 1, 2, 3, 21, 22, 24)},
    {Instr::hvc, makeMask(4, 5, 6, 22, 24)},
    {Instr::isb, makeMask(5, 6, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24,
                          26, 28, 29, 30, 31)},
    {Instr::ldc1Imm, makeMask(20, 26, 27, 21, 22, 23, 24)},
    {Instr::ldc2Imm, makeMask(20, 26, 27, 28, 29, 30, 31, 21, 22, 23, 24)},
    {Instr::ldc2Lit,
     makeMask(16, 17, 18, 19, 20, 26, 27, 28, 29, 30, 31, 21, 22, 23, 24)},
    {Instr::ldc1Lit, makeMask(16, 17, 18, 19, 20, 26, 27, 21, 22, 23, 24)},
    {Instr::ldm, makeMask(20, 23, 27)},
    {Instr::ldmda, makeMask(20, 27)},
    {Instr::ldmdb, makeMask(20, 24, 27)},
    {Instr::ldmib, makeMask(20, 23, 24, 27)},
    {Instr::ldrImm, makeMask(20, 26)},
    {Instr::ldrLit, makeMask(16, 17, 18, 19, 20, 26)},
    {Instr::ldrReg, makeMask(20, 25, 26)},
    {Instr::ldrbImm, makeMask(20, 22, 26)},
    {Instr::ldrbLit, makeMask(16, 17, 18, 19, 20, 22, 26)},
    {Instr::ldrbReg, makeMask(20, 22, 25, 26)},
    {Instr::ldrbt1, makeMask(20, 21, 22, 26)},
    {Instr::ldrbt2, makeMask(20, 21, 22, 25, 26)},
    {Instr::ldrdImm, makeMask(4, 6, 7, 22)},
    {Instr::ldrdLit, makeMask(4, 6, 7, 16, 17, 18, 19, 22, 24)},
    {Instr::ldrdReg, makeMask(4, 6, 7)},
    {Instr::ldrex, makeMask(0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 20, 23, 24)},
    {Instr::ldrexb, makeMask(0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 20, 22, 23, 24)},
    {Instr::ldrexd, makeMask(0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 20, 21, 23, 24)},
    {Instr::ldrexh,
     makeMask(0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 20, 21, 22, 23, 24)},
    {Instr::ldrhImm, makeMask(4, 5, 7, 20, 22)},
    {Instr::ldrhLit, makeMask(4, 5, 7, 16, 17, 18, 19, 20, 22)},
    {Instr::ldrhReg, makeMask(4, 5, 7, 20)},
    {Instr::ldrht, makeMask(4, 5, 7, 20, 21, 22)},
    {Instr::ldrht, makeMask(4, 5, 7, 20, 21)},
    {Instr::ldrsbImm, makeMask(4, 6, 7, 20, 22)},
    {Instr::ldrsbLit, makeMask(4, 6, 7, 16, 17, 18, 19, 20, 22)},
    {Instr::ldrsbReg, makeMask(4, 6, 7, 20)},
    {Instr::ldrsbt, makeMask(4, 6, 7, 20, 21, 22)},
    {Instr::ldrsbt, makeMask(4, 6, 7, 20, 21)},
    {Instr::ldrshImm, makeMask(4, 5, 6, 7, 20, 22)},
    {Instr::ldrshLit, makeMask(4, 5, 6, 7, 16, 17, 18, 19, 20, 22)},
    {Instr::ldrsh, makeMask(4, 5, 6, 7, 20)},
    {Instr::ldrsht, makeMask(4, 5, 6, 7, 20, 21, 22)},
    {Instr::ldrsht, makeMask(4, 5, 6, 7, 20, 21)},
    {Instr::ldrt1, makeMask(20, 21, 26)},
    {Instr::ldrt2, makeMask(20, 21, 25, 26)},
    {Instr::lslImm, makeMask(21, 23, 24, 7)},
    {Instr::lslReg, makeMask(4, 21, 23, 24)},
    {Instr::lsrImm, makeMask(7, 5, 21, 23, 24)},
    {Instr::lsrReg, makeMask(4, 5, 21, 23, 24)},
    {Instr::mcr1, makeMask(4, 25, 26, 27)},
    {Instr::mcr2, makeMask(4, 25, 26, 27, 28, 29, 30, 31)},
    {Instr::mcrr1, makeMask(22, 26, 27)},
    {Instr::mcrr2, makeMask(22, 26, 27, 28, 29, 30, 31)},
    {Instr::mla, makeMask(4, 7, 21)},
    {Instr::mls, makeMask(4, 7, 21, 22)},
    {Instr::movImm, makeMask(21, 23, 24, 25)},
    {Instr::movImm16, makeMask(24, 25)},
    {Instr::movReg, makeMask(21, 23, 24)},
    {Instr::movt, makeMask(22, 24, 25)},
    {Instr::mrc1, makeMask(4, 20, 25, 26, 27)},
    {Instr::mrc2, makeMask(4, 20, 25, 26, 27, 28, 29, 30, 31)},
    {Instr::mrrc1, makeMask(20, 22, 26, 27)},
    {Instr::mrrc2, makeMask(20, 22, 26, 27, 28, 29, 30, 31)},
    {Instr::mrs, makeMask(16, 17, 18, 19, 24)},
    {Instr::mrsBanked, makeMask(9, 24)},
    {Instr::msrImmApp, makeMask(12, 13, 14, 15, 21, 24, 25, 19)},
    {Instr::msrImmSys, makeMask(12, 13, 14, 15, 21, 24, 25, 16)},
    {Instr::msrApp, makeMask(12, 13, 14, 15, 21, 24)},
    {Instr::msrSys, makeMask(12, 13, 14, 15, 21, 24, 17)},
    {Instr::msrBanked, makeMask(9, 12, 13, 14, 15, 21, 24)},
    {Instr::mul, makeMask(7, 4)},
    {Instr::mvnImm, makeMask(21, 22, 23, 24, 25)},
    {Instr::mvnReg, makeMask(21, 22, 23, 24)},
    {Instr::mvnShiftedReg, makeMask(4, 21, 22, 23, 24)},
    {Instr::nop, makeMask(12, 13, 14, 15, 21, 24, 25)},
    {Instr::orrImm, makeMask(23, 24, 25)},
    {Instr::orrReg, makeMask(23, 24)},
    {Instr::orrShiftedReg, makeMask(4, 23, 24)},
    {Instr::pkh, makeMask(23, 25, 26, 4)},
    {Instr::pldImm, makeMask(12, 13, 14, 15, 20, 24, 26, 28, 29, 30, 31)},
    {Instr::pldLit,
     makeMask(12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 24, 26, 28, 29, 30, 31)},
    {Instr::pldReg, makeMask(12, 13, 14, 15, 20, 25, 26, 24, 28, 29, 30, 31)},
    {Instr::pliImm, makeMask(12, 13, 14, 15, 20, 22, 26, 28, 29, 30, 31)},
    {Instr::pliReg, makeMask(12, 13, 14, 15, 22, 25, 26, 28, 29, 30, 31, 20)},
    {Instr::pop, makeMask(16, 18, 19, 20, 21, 23, 27)},
    // {Instr::pop, makeMask(16, 18, 19,20, 23,26, 2)},
    {Instr::push, makeMask(16, 18, 19, 21, 24, 27)},
    {Instr::qadd, makeMask(4, 6, 24)},
    {Instr::qadd16, makeMask(4, 8, 9, 10, 11, 21, 25, 26)},
    {Instr::qadd8, makeMask(4, 7, 8, 9, 10, 11, 21, 25, 26)},
    {Instr::qasx, makeMask(4, 5, 8, 9, 10, 11, 21, 25, 26)},
    {Instr::qdadd, makeMask(4, 6, 22, 24)},
    {Instr::qdsub, makeMask(4, 6, 21, 22, 24)},
    {Instr::qsax, makeMask(4, 6, 8, 9, 10, 11, 21, 25, 26)},
    {Instr::qsub, makeMask(4, 6, 21, 24)},
    {Instr::qsub16, makeMask(26, 25, 21, 11, 10, 9, 8, 6, 4, 5)},
    {Instr::qsub8, makeMask(4, 5, 6, 7, 8, 9, 10, 11, 21, 25, 26)},
    {Instr::rbit,
     makeMask(4, 5, 8, 9, 10, 11, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26)},
    {Instr::rev,
     makeMask(4, 5, 8, 9, 10, 11, 16, 17, 18, 19, 20, 21, 23, 25, 26)},
    {Instr::rev16,
     makeMask(4, 5, 7, 8, 9, 10, 11, 16, 17, 18, 19, 20, 21, 23, 25, 26)},
    {Instr::revsh,
     makeMask(4, 5, 7, 8, 9, 10, 11, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26)},
    {Instr::rfe, makeMask(9, 11, 20, 27, 28, 29, 30, 31)},
    {Instr::rorImm, makeMask(5, 6, 21, 23, 24, 7)},
    {Instr::rorReg, makeMask(4, 5, 6, 21, 23, 24)},
    {Instr::rrx, makeMask(5, 6, 21, 23, 24)},
    {Instr::rsbImm, makeMask(21, 22, 25)},
    {Instr::rsbReg, makeMask(21, 22)},
    {Instr::rsbShiftedReg, makeMask(21, 22, 4)},
    {Instr::rscImm, makeMask(21, 22, 23, 25)},
    {Instr::rscReg, makeMask(21, 22, 23)},
    {Instr::rscShiftedReg, makeMask(4, 21, 22, 23)},
    {Instr::sadd16, makeMask(4, 8, 9, 10, 11, 20, 25, 26)},
    {Instr::sadd8, makeMask(4, 7, 8, 9, 10, 11, 20, 25, 26)},
    {Instr::sasx, makeMask(4, 5, 8, 9, 10, 11, 20, 25, 26)},
    {Instr::sbcImm, makeMask(22, 23, 25)},
    {Instr::sbcReg, makeMask(22, 23)},
    {Instr::sbcShiftedReg, makeMask(4, 22, 23)},
    {Instr::sbfx, makeMask(4, 6, 21, 23, 24, 25, 26)},
    {Instr::sdiv, makeMask(4, 12, 13, 14, 15, 20, 24, 25, 26)},
    {Instr::sel, makeMask(4, 5, 7, 8, 9, 10, 11, 23, 25, 26)},
#define uncond 31, 30, 29, 28
    {Instr::setend, makeMask(uncond, 16, 24)},
    {Instr::sev, makeMask(2, 12, 13, 14, 15, 21, 24, 25)},
    {Instr::shadd16, makeMask(4, 8, 9, 10, 11, 20, 21, 25, 26)},
    {Instr::shadd8, makeMask(4, 7, 8, 9, 10, 11, 20, 21, 25, 26)},
    {Instr::shasx, makeMask(4, 5, 8, 9, 10, 11, 20, 21, 25, 26)},
    {Instr::shsax, makeMask(4, 6, 8, 9, 10, 11, 20, 21, 25, 26)},
    {Instr::shsub16, makeMask(4, 5, 6, 8, 9, 10, 11, 20, 21, 25, 26)},
    {Instr::shsub8, makeMask(4, 5, 6, 7, 8, 9, 10, 11, 20, 21, 25, 26)},
    {Instr::smc, makeMask(4, 5, 6, 21, 22, 24)},
    {Instr::smlabb, makeMask(7, 24)},
    {Instr::smlad, makeMask(4, 24, 25, 26)},
    {Instr::smlal, makeMask(4, 7, 21, 22, 23)},
    {Instr::smlalbb, makeMask(7, 22, 24)},
    {Instr::smlald, makeMask(4, 22, 24, 25, 26)},
    {Instr::smlawb, makeMask(7, 21, 24)},
    {Instr::smlsd, makeMask(4, 6, 24, 25, 26)},
    {Instr::smlsld, makeMask(4, 6, 22, 24, 25, 26)},
    {Instr::smmla, makeMask(4, 20, 22, 24, 25, 26)},
    {Instr::smmls, makeMask(4, 6, 7, 1, 20, 22, 24, 25, 26)},
    {Instr::smmul, makeMask(4, 12, 13, 14, 15, 20, 22, 24, 25, 26)},
    {Instr::smuad, makeMask(4, 12, 13, 14, 15, 24, 25, 26)},
    {Instr::smulbb, makeMask(7, 21, 22, 24)},
    {Instr::smull, makeMask(4, 7, 22, 23)},
    {Instr::smulwb, makeMask(5, 7, 21, 24)},
    {Instr::smusd, makeMask(4, 6, 12, 13, 14, 15, 24, 25, 26)},
    {Instr::srs, makeMask(uncond, 8, 10, 16, 18, 19, 22, 27)},
    {Instr::ssat, makeMask(4, 21, 23, 25, 26)},
    {Instr::ssat16, makeMask(4, 5, 8, 9, 10, 11, 21, 23, 25, 26)},
    {Instr::ssax, makeMask(4, 6, 8, 9, 10, 11, 20, 25, 26)},
    {Instr::ssub16, makeMask(4, 5, 6, 8, 9, 10, 11, 20, 25, 26)},
    {Instr::ssub8, makeMask(4, 5, 6, 7, 8, 9, 10, 11, 20, 25, 26)},
    {Instr::stc1, makeMask(26, 27, 21)},
    {Instr::stc2, makeMask(26, 27, 28, 29, 30, 31, 21)},
    {Instr::stm, makeMask(23, 27)},
    {Instr::stmda, makeMask(27)},
    {Instr::stmdb, makeMask(24, 27)},
    {Instr::stmib, makeMask(23, 24, 27)},
    {Instr::strImm, makeMask(26)},
    {Instr::strReg, makeMask(25, 26)},
    {Instr::strbImm, makeMask(22, 26)},
    {Instr::strbReg, makeMask(22, 25, 26)},
    {Instr::strbt1, makeMask(21, 22, 26)},
    {Instr::strbt2, makeMask(21, 22, 25, 26)},
    {Instr::strdImm, makeMask(4, 5, 6, 7, 22)},
    {Instr::strd, makeMask(4, 5, 6, 7)},
    {Instr::strex, makeMask(4, 7, 8, 9, 10, 11, 23, 24)},
    {Instr::strexb, makeMask(4, 7, 8, 9, 10, 11, 22, 23, 24)},
    {Instr::strexd, makeMask(4, 7, 8, 9, 10, 11, 21, 23, 24)},
    {Instr::strexh, makeMask(4, 7, 8, 9, 10, 11, 21, 22, 23, 24)},
    {Instr::strhImm, makeMask(4, 5, 7, 22)},
    {Instr::strhReg, makeMask(4, 5, 7)},
    {Instr::strht, makeMask(4, 5, 7, 21, 22)},
    {Instr::strht2, makeMask(4, 5, 7, 21)},
    {Instr::strt1, makeMask(21, 26)},
    {Instr::strt2, makeMask(21, 25, 26)},
    {Instr::subImm, makeMask(22, 25)},
    {Instr::subReg, makeMask(22)},
    {Instr::subShiftedReg, makeMask(4, 22)},
    {Instr::svc, makeMask(24, 25, 26, 27)},
    {Instr::swp, makeMask(4, 7, 24)},
    {Instr::sxtab, makeMask(4, 5, 6, 21, 23, 25, 26)},
    {Instr::sxtab16, makeMask(4, 5, 6, 23, 25, 26)},
    {Instr::sxtah, makeMask(4, 5, 6, 20, 21, 25, 26, 23)},
    {Instr::sxtb, makeMask(4, 5, 6, 16, 17, 18, 19, 21, 23, 25, 26)},
    {Instr::sxtb16, makeMask(4, 5, 6, 16, 17, 18, 19, 25, 26, 23)},
    {Instr::sxth, makeMask(4, 5, 6, 16, 17, 18, 19, 20, 21, 23, 25, 26)},
    {Instr::teqImm, makeMask(20, 21, 24, 25)},
    {Instr::teqReg, makeMask(20, 21, 24)},
    {Instr::teqShiftedReg, makeMask(4, 20, 21, 24)},
    {Instr::tstImm, makeMask(20, 24, 25)},
    {Instr::tstReg, makeMask(20, 24)},
    {Instr::tstShiftedReg, makeMask(4, 20, 24)},
    {Instr::uadd16, makeMask(4, 8, 9, 10, 11, 20, 22, 25, 26)},
    {Instr::uadd8, makeMask(4, 7, 8, 9, 10, 11, 20, 22, 25, 26)},
    {Instr::uasx, makeMask(4, 5, 8, 9, 10, 11, 20, 22, 25, 26)},
    {Instr::ubfx, makeMask(4, 6, 21, 22, 23, 24, 25, 26)},
    {Instr::undefined,
     makeMask(4, 5, 6, 7, 20, 21, 22, 23, 24, 25, 26, 29, 30, 31)},
    {Instr::udiv, makeMask(4, 12, 13, 12, 15, 20, 21, 24, 25, 26)},
    {Instr::uhadd16, makeMask(4, 8, 9, 10, 11, 20, 21, 22, 25, 26)},
    {Instr::uhadd8, makeMask(4, 7, 8, 9, 10, 11, 20, 21, 22, 25, 26)},
    {Instr::uhasx, makeMask(4, 5, 8, 9, 10, 11, 20, 21, 22, 25, 26)},
    {Instr::uhsax, makeMask(4, 6, 8, 9, 10, 11, 20, 21, 22, 25, 26)},
    {Instr::uhsub16, makeMask(4, 5, 6, 8, 9, 10, 11, 20, 21, 22, 25, 26)},
    {Instr::uhsub8, makeMask(4, 5, 6, 7, 8, 9, 10, 11, 20, 21, 22, 25, 26)},
    {Instr::umaal, makeMask(4, 7, 22)},
    {Instr::umlal, makeMask(4, 7, 21, 23)},
    {Instr::umull, makeMask(4, 7, 23)},
    {Instr::uqadd16, makeMask(4, 8, 9, 10, 11, 21, 22, 25, 26)},
    {Instr::uqadd8, makeMask(4, 7, 8, 9, 10, 11, 21, 22, 25, 26)},
    {Instr::uqasx, makeMask(4, 5, 8, 9, 10, 11, 21, 22, 25, 26)},
    {Instr::uqsax, makeMask(4, 6, 8, 9, 10, 11, 21, 22, 25, 26)},
    {Instr::uqsub16, makeMask(4, 5, 6, 8, 9, 10, 11, 21, 22, 25, 26)},
    {Instr::uqsub8, makeMask(4, 5, 6, 7, 8, 9, 10, 11, 21, 22, 25, 26)},
    {Instr::usad8, makeMask(4, 12, 13, 14, 15, 23, 24, 25, 26)},
    {Instr::usada8, makeMask(4, 23, 24, 25, 26)},
    {Instr::usat, makeMask(4, 21, 22, 23, 25, 26)},
    {Instr::usat16, makeMask(4, 5, 8, 9, 10, 11, 21, 22, 23, 25, 26)},
    {Instr::usax, makeMask(4, 6, 8, 9, 10, 11, 20, 22, 25, 26)},
    {Instr::usub16, makeMask(4, 5, 6, 8, 9, 10, 11, 20, 22, 25, 26)},
    {Instr::usub8, makeMask(4, 5, 6, 7, 8, 9, 10, 11, 20, 22, 25, 26)},
    {Instr::uxtab, makeMask(4, 5, 6, 21, 22, 23, 25, 26)},
    {Instr::uxtab16, makeMask(4, 5, 6, 22, 23, 25, 26)},
    {Instr::uxtah, makeMask(4, 5, 6, 20, 21, 22, 23, 25, 26)},
    {Instr::uxtb, makeMask(4, 5, 6, 16, 17, 18, 19, 21, 22, 23, 25, 26)},
    {Instr::uxtb16, makeMask(4, 5, 6, 16, 17, 18, 19, 22, 23, 25, 26)},
    {Instr::uxth, makeMask(4, 5, 6, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26)},
    {Instr::wfe, makeMask(12, 13, 14, 15, 24, 25, 1, 21)},
    {Instr::wfi, makeMask(0, 1, 12, 13, 14, 15, 21, 24, 25)},
    {Instr::yield, makeMask(0, 12, 13, 14, 15, 21, 24, 25)},
};

void test() {
  u32 tested = 0;
  for (size_t i = 0; i < (sizeof(pairs) / sizeof(Pair)); i++) {
    Instr actual = decodeA(pairs[i].mask);
    if (actual != pairs[i].instr) {
      printf("test [%ld] failed.\n expected: ", i);
      printInstr(pairs[i].instr);
      printf(" found: ");
      printInstr(actual);
      abort();
    }
    if (actual != Instr::andReg)
      tested += 1;
  }
  printf("tested %u/%u\n", tested, (u32)Instr::msrImmSys);
}

#endif

} // namespace Decoder
