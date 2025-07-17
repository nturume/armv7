
#pragma once
#include "fifo.hpp"
#include "mem.hpp"
#include "stuff.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>

// #define mysd
// "\x1D\x53\x44\x30\x30\x31\x47\x80\x12\x34\x56\x78\x9A\xBC\xDE\x01"

struct PL180 {

  struct SD {

    union SCR {
      struct {
        u32 _1 : 32 = 0;
        u32 CMD_SUPPORT : 5 = 0x1f;
        u32 _2 : 1 = 0;
        u32 SD_SPECX : 4 = 0;
        // in 2

        u32 SD_SPEC4 : 1 = 0;
        u32 EX_SECURITY : 4 = 0;
        u32 SD_SPEC3 : 1 = 0;

        u32 SD_BUS_WIDTHS : 4 = 5;
        u32 SD_SECURITY : 3 = 0;
        u32 DATA_STAT_AFTER_ERASE : 1 = 0;

        u32 SD_SPEC : 4 = 0;
        u32 SCR_STRUCTURE : 4 = 0;
      } b = {};

      static void pack(u32 *buf) {
        SCR c;
        u8 *b8 = (u8 *)buf;

        b8[0] = c.b.SCR_STRUCTURE;
        b8[1] = c.b.SD_BUS_WIDTHS;
        b8[2] = 0;
        b8[3] = 0;
        b8[4] = 0;
        b8[5] = 0;
        b8[6] = 0;
        b8[7] = 0;

        buf[0] = swap32(buf[0]);
        buf[1] = swap32(buf[1]);
        buf[2] = swap32(buf[2]);
        buf[3] = swap32(buf[3]);
      }
    };

    struct CSD {

      static u32 csize(u32 cap) {
        assert(cap % (512 * 1024) == 0);
        return (cap / (512 * 1024)) - 1;
      }

      struct __attribute__((packed)) {
        u32 _1 : 1 = 1;
        u32 CRC : 7 = 0; // TODO
        u32 _2 : 1 = 0;
        u32 WP_UPC : 1 = 0;
        u32 FILE_FORMAT : 2 = 0;
        u32 TMP_WRITE_PROTECT : 1 = 0;
        u32 PERM_WRITE_PROTECT : 1 = 0;
        u32 COPY : 1 = 0;
        u32 FILE_FORMAT_GRP : 1 = 0;
        u32 _3 : 5 = 0;
        u32 WRITE_BL_PARTIAL : 1 = 0;

        // out 1
        u32 WRITE_BL_LEN : 4 = 9;
        u32 R2W_FACTOR : 3 = 0b0;
        u32 _4 : 2 = 0;
        // in 1

        // out 1
        u32 WP_GRP_ENABLE : 1 = 0;
        u32 WP_GRP_SIZE : 7 = 0;
        // in 1

        // out 1
        u32 SECTOR_SIZE : 7 = 0;
        u32 ERASE_BLK_EN : 1 = 1;
        //=====
        u32 C_SIZE_MULT : 3 = 3;
        u32 VDD_W_CURR_MAX : 3 = 0;
        u32 VDD_W_CURR_MIN : 3 = 0;

        u32 VDD_R_CURR_MAX : 3 = 0;
        u32 VDD_R_CURR_MIN : 3 = 0;

        u32 C_SIZE : 12 = 0xfff; // 2bits --->
        // 22

        u32 _5 : 2 = 0; // 6-2
        u32 DSR_IMP : 1 = 0;
        u32 READ_BLK_MISALIGN : 1 = 0;
        u32 WRITE_BLK_MISALIGN : 1 = 0;
        u32 READ_BL_PARTIAL : 1 = 1;

        u32 READ_BL_LEN : 4 = 9;      // 5
        u32 CCC : 12 = 0b10110110101; // 4
        //====
        u32 TRAN_SPEED : 8 = 0xff; // 3

        u32 NSAC : 8 = 0; // 2

        u32 TAAC : 8 = 0xe; // 1

        u32 CSD_STRUCTURE : 8 = 0; // 0
      } b = {};

      // capacity = ((C_SIZE+1) * (2.pow(C_SIZE_MULT+2)))
      // * (512)

      static u32 getmult(u32 imgsize) {
        u32 csize = 0xfff + 1;
        u32 blocklen = 512;

        // imgsize = csize * mult * blocklen;
        assert((imgsize % (csize * blocklen)) == 0);
        u32 mult = imgsize / (csize * blocklen);
        assert(bitcount32(mult) == 1);
        mult = clz32(mult);
        assert(mult > 1);
        mult -= 2;
        return mult;
      }

      static void pack(u32 *buf, u32 imgsize) {
        CSD c;
        c.b.C_SIZE_MULT = getmult(imgsize);
        u8 *b8 = (u8 *)buf;

        b8[0] = c.b.CSD_STRUCTURE;
        b8[1] = c.b.TAAC;
        b8[2] = c.b.NSAC;
        b8[3] = c.b.TRAN_SPEED;
        b8[4] = c.b.CCC >> 4;
        b8[5] = ((c.b.CCC & 0xf) << 4) | c.b.READ_BL_LEN;

        b8[6] = (0x80) | ((c.b.C_SIZE >> 10) & 0b11);
        b8[7] = ((c.b.C_SIZE >> 2) & 0xff);
        b8[8] = (u8(c.b.C_SIZE & 0b11) << 6);

        b8[9] = ((c.b.C_SIZE_MULT >> 1));

        b8[10] =
            (u8(c.b.C_SIZE_MULT & 1) << 7) | (u8(c.b.ERASE_BLK_EN) << 6) | (0);

        b8[11] = 0;
        b8[12] = (c.b.WRITE_BL_LEN >> 2);
        b8[13] = (u8(c.b.WRITE_BL_LEN & 0b11) << 6);
        b8[14] = 0;
        b8[15] = 0; // TODO crc

        buf[0] = swap32(buf[0]);
        buf[1] = swap32(buf[1]);
        buf[2] = swap32(buf[2]);
        buf[3] = swap32(buf[3]);
      }
    };

    static_assert(sizeof(CSD) == 16, "");

    struct CID {
      struct __attribute__((packed)) {
        u32 CRC : 8 = 0;
        // --->
        u32 MDT_M : 4 = 0x4;
        // --->
        u32 MDT_Y : 8 = 0x01;
        // --->
        u32 _1 : 4 = 0;
        // --->
        u32 PSN : 32 = 0xdeada55;
        // --->
        u32 PRV : 8 = 0xbe;
        // --->
        u8 PNM[5] = {'T', 'I', 'T', 'T', 'Y'};
        // --->
        u32 OID : 16 = 0x6967;
        // --->
        u32 MID : 8 = 0xba;
      } b = {};

      static void pack(u32 *buf) {
        CID c;
        u8 *b8 = (u8 *)buf;
        b8[0] = c.b.MID;
        b8[1] = c.b.OID >> 8;
        b8[2] = c.b.OID & 0xff;
        b8[3] = c.b.PNM[0];
        b8[4] = c.b.PNM[1];
        b8[5] = c.b.PNM[2];
        b8[6] = c.b.PNM[3];
        b8[7] = c.b.PNM[4];
        b8[8] = c.b.PRV;
        b8[9] = c.b.PSN >> 24;
        b8[10] = c.b.PSN >> 16;
        b8[11] = c.b.PSN >> 8;
        b8[12] = c.b.PSN;
        b8[13] = c.b.MDT_Y >> 8;
        b8[14] = (c.b.MDT_Y << 4) | c.b.MDT_M;
        b8[15] = 0; // TODO CRC

        buf[0] = swap32(buf[0]);
        buf[1] = swap32(buf[1]);
        buf[2] = swap32(buf[2]);
        buf[3] = swap32(buf[3]);
      }
    };

    static_assert(sizeof(CID) == 16, "");

    enum class Cmd {
      GO_IDLE_STATE_0 = 0,
      SEND_IF_COND_8 = 8,
      APP_CMD_55 = 55,
      ALL_SEND_CID_2 = 2,
      SEND_RELATIVE_ADDR_3 = 3,
      SEND_CSD_9 = 9,
      SELECT_7 = 7,
      SWITCH_FUNC_8 = 6,
      SET_BLOCKLEN_16 = 16,
      READ_SINGLE_BLOCK_17 = 17,
      READ_MULTIPLE_BLOCK_18 = 18,
      STOP_TRANSMISSION_12 = 12,
      WRITE_BLOCK_24 = 24,
      SEND_STATUS_13 = 13,
    };

    enum class AppCmd {
      SD_SEND_OP_COND_41 = 41,
      SEND_SCR_51 = 51,
      SET_BUS_WIDTH_6 = 6,
      SD_STATUS_13 = 13,
    };

    enum class State {
      idle,
      ready,
      ident,
      stby,
      tran,
      data,
      rcv,
      prg,
      dis,
    };

    State _state;
    bool expectapp = false;
    bool selected = false;
    PL180 *controller;
    u32 blocklen = 512; // bytes
    i32 blockrem;
    u32 addr;
    bool readmultblocks;
    bool writemultblocks;
    FileReader imgreader;

    SD(const char *imgpath = "/home/m/Documents/vdisk.img")
        : imgreader(nullptr) {
      FILE *f = fopen(imgpath, "rb+");
      imgreader.f = f;

      u32 imgsize = imgreader.getFileSize();
      u32 mult = CSD::getmult(imgsize);
      printf("===> IMG SIZE = %d mult = %d\n", imgsize, mult);
    }

    bool select(u32 word) {
      if (!stdbyOrTran()) {
        return false;
      }

      if (word == 0x69680000) {
        state(State::tran);
      } else if (word == 0) {
        state(State::stby);
      } else {
        return false;
      }
      return true;
    }

    void state(State s) { _state = s; }
    State state() { return _state; };

    bool stdbyOrTran() {
      return _state == State::stby || _state == State::tran;
    }

    bool ready() { return _state != State::idle; }

    u32 getStatus() {
      u32 status = 0;
      status |= ((u8(state()) & 0xf) << 9);
      if (expectapp) {
        status |= (1u << 5);
      }
      if (ready()) {
        status |= (1u << 8);
      }
      return status;
    }

    void rcvStuff() {
      imgreader.seekTo(addr);
      while (!controller->_txfifo.empty() and blockrem > 0) {
        union {
          u32 back;
          u8 array[4];
        } tmp;
        tmp.back = controller->txfifo();
        u32 r = imgreader.write(tmp.array, 4);
        assert(r == 4);
        blockrem -= 4;
        addr += 4;
        if (blockrem == 0) {
          controller->dataBlockEnd();
          if (writemultblocks) {
            blockrem = blocklen;
          } else {
            state(State::tran);
            break;
          }
        }

        assert(blockrem >= 0);
      }
    }

    void poll() {
      switch (state()) {
      case State::data:
        break;
      case State::rcv:
        return rcvStuff();
      default:
        return;
      }

      imgreader.seekTo(addr);
      for (; blockrem > 0 and !controller->_rxfifo.full();) {
        union {
          u32 back;
          u8 array[4];
        } tmp;
        u32 r = imgreader.read(tmp.array, 4);
        // tmp = swap32(tmp);
        // printf("==============reading addr: %d (%ld) r=%d  [0x%x, 0x%x,
        // 0x%x,0x%x]\n", addr , imgreader.getPos(), r, tmp.array[0],
        // tmp.array[1], tmp.array[2], tmp.array[3]);
        controller->rxfifo(tmp.back);
        assert(r == 4);
        blockrem -= 4;
        addr += 4;

        if (blockrem == 0) {
          controller->cmdRespEnd();
          controller->dataBlockEnd();
          if (readmultblocks) {
            blockrem = blocklen;
          } else {
            state(State::tran);
            break;
          }
        }

        assert(blockrem >= 0);
      }
    }

    bool doAppCommand(u8 command, u32 arg, u32 *buf) {
      // printf("==========> app command: %d\n", command);
      switch (AppCmd(command)) {
      case AppCmd::SD_SEND_OP_COND_41: {
        state(State::ready);
        *buf = 0x80FFFF00;
        return true;
      }
      case AppCmd::SEND_SCR_51: {
        assert(controller->_rxfifo.empty());
        blockrem = 0;
        readmultblocks = false;
        SCR::pack(buf);
        controller->rxfifo(swap32(buf[0]));
        controller->rxfifo(swap32(buf[1]));
        controller->cmdRespEnd();
        controller->dataBlockEnd();
        *buf = getStatus();
        return true;
      }
      case AppCmd::SET_BUS_WIDTH_6: {
        *buf = getStatus();
        return true;
      }
      case AppCmd::SD_STATUS_13: {
        *buf = getStatus();
        for (u32 i = 0; i < 16; i++) {
          controller->rxfifo(0);
        }
        controller->cmdRespEnd();
        controller->dataBlockEnd();
        return true;
      }
      default:
        printf("Unhandled app command: %d\n", command);
        assert(false);
      }
    }

    bool doCommand(u8 command, u32 arg, u32 *buf) {
      buf[0] = 0;
      buf[1] = 0;
      buf[2] = 0;
      buf[3] = 0;
      if (expectapp) {
        expectapp = false;
        return doAppCommand(command, arg, buf);
      }
      // printf("==========> command: %d\n", command);
      switch (Cmd(command)) {
      case Cmd::GO_IDLE_STATE_0: {
        state(State::idle);
        return true;
      }
      case Cmd::SEND_IF_COND_8: {
        *buf = 1u << 8;
        *buf |= arg & 0xff;
        return true;
      }
      case Cmd::APP_CMD_55: {
        expectapp = true;
        *buf = getStatus();
        return true;
      }
      case Cmd::SEND_CSD_9: {
        // printf("RCA: %x\n", arg);
        assert(arg == 0x69680000);
        state(State::stby);
        CSD::pack(buf, imgreader.getFileSize());
        return true;
      }
      case Cmd::ALL_SEND_CID_2: {
        state(State::ident);
        CID::pack(buf);
        return true;
      }
      case Cmd::SEND_RELATIVE_ADDR_3: {
        assert(state() == State::ident);
        state(State::stby);
        *buf = 0x69680000;
        return true;
      }
      case Cmd::SELECT_7: {
        bool done = select(arg);
        *buf = getStatus();
        return done;
      }
      case Cmd::SET_BLOCKLEN_16: {
        blocklen = arg;
        assert(blocklen == 512);
        // printf("NEW BLOCK LEN: %d\n", blocklen);
        *buf = getStatus();
        return true;
      }
      case Cmd::READ_SINGLE_BLOCK_17: {
        assert(state() == State::tran);
        assert(controller->_rxfifo.empty());
        readmultblocks = false;
        *buf = getStatus();
        state(State::data);
        addr = arg;
        blockrem = i32(blocklen);
        // printf("READ SINGLE: add = %d rem = %d\n", addr, blockrem);
        // poll();
        return true;
      }
      case Cmd::WRITE_BLOCK_24: {
        assert(state() == State::tran);
        assert(controller->_txfifo.empty());
        readmultblocks = false;
        *buf = getStatus();
        state(State::rcv);
        addr = arg;
        blockrem = i32(blocklen);
        // printf("WRITE SINGLE: add = %d rem = %d\n", addr, blockrem);
        // poll();
        return true;
      }
      case Cmd::READ_MULTIPLE_BLOCK_18: {
        assert(state() == State::tran);
        assert(controller->_rxfifo.empty());
        readmultblocks = true;
        *buf = getStatus();
        state(State::data);
        addr = arg;
        blockrem = i32(blocklen);
        // printf("READ MULT: add = %d rem = %d\n", addr, blockrem);
        // poll();
        return true;
      }
      case Cmd::STOP_TRANSMISSION_12: {
        // printf("        STOP TRANSMISSION: %d fifoempty? %d\n", u8(state()),
        // controller->_rxfifo.empty()?1:0);
        assert(state() == State::data);
        readmultblocks = false;
        *buf = getStatus();
        state(State::tran);
        return true;
      }
      case Cmd::SEND_STATUS_13: {
          *buf = getStatus();
          return true;
        }
      default:
        printf("Unhandled command: %d\n", command);
        assert(false);
      }
    }
  };

  enum class PowerState {
    off,
    reserved,
    up,
    on,
  };

  enum class Reply {
    arsenalucl,
    small,
    big,
  };

  PowerState ps = PowerState::off;
  u32 powerctl = 0;
  u32 _mask0 = 0;
  u32 _mask1 = 0;
  u32 clock = 0;
  u32 dctl = 0;

  SD sd;

  u32 _status = 0;

  u32 _arg;

  u32 cmdbuf[4];

  u32 _datalen;
  u32 _datacnt;

  FIFO<u32, 16> _txfifo;
  FIFO<u32, 16> _rxfifo;

  PL180() {
    // printf("C_SIZE ==>>>> 0x%x\n", SD::CSD{}.b.C_SIZE);
    sd.controller = this;
  }

  void dataCntDown(u32 amount = 4) {
    if (_datacnt == 0)
      return;
    u32 tmp = _datacnt - amount;
    assert(tmp < _datacnt);
    _datacnt = tmp;
    if (_datacnt == 0) {
      _status |= (1u << 8);
    } else {
      _status &= ~(1u << 8);
    }
  }

  void txfifo(u32 v) {
    if (_txfifo.full()) {
      return;
    }
    _txfifo.write(v);
  }

  u32 txfifo() {
    assert(!_txfifo.empty());
    if (!tranFromCard())
      dataCntDown();
    u32 data = _txfifo.read();
    if (_datacnt == 0) {
      _txfifo.drain();
    }
    return data;
  }

  void rxfifo(u32 v) {
    if (_rxfifo.full()) {
      _status |= (1u << 5);
      return;
    }
    _rxfifo.write(v);
    // _datacnt -= 1;
  }

  u32 rxfifo() {
    if (_rxfifo.empty()) {
      return 0;
    }
    if (tranFromCard())
      dataCntDown();
    u32 data = _rxfifo.read();
    if (_datacnt == 0) {
      _rxfifo.drain();
    }
    return data;
  }

  void datalen(u32 v) {
    _datalen = v;
    _datacnt = v;
  }

  void datactl(u32 v) { dctl = v; }

  bool tranFromCard() { return (dctl >> 1) & 1; }

  bool streamTran() { return (dctl >> 2) & 1; }

  bool dmaTran() { return (dctl >> 3) & 1; }

  bool tranBlocksz() {
    u32 exp = (dctl >> 4) & 0xf;
    assert(exp < 12);
    if (exp == 0)
      return 1;
    return 2 << (exp - 1);
  }

  bool dataTranEnabled() { return (dctl) & 1; }

  u32 status() {
    if (_datacnt > 0)
      sd.poll();

    if (_datacnt > 0) {
      if (tranFromCard()) {
        _status |= (1u << 12);
        _status &= ~(1u << 13);
      } else {
        _status |= (1u << 13);
        _status &= ~(1u << 12);
      }
    } else {
      _status &= ~(1u << 13);
      _status &= ~(1u << 12);
    }

    if (_txfifo.empty()) {
      _status |= (1u << 18);
      _status &= ~(1u << 20);
    } else {
      _status &= ~(1u << 18);
      _status |= (1u << 20);
    }

    if (_txfifo.full()) {
      _status |= (1u << 16);
    } else {
      _status &= ~(1u << 16);
    }

    // 1100 0000 0000 0000 0000

    if (_rxfifo.empty()) {
      _status |= (1u << 19);
      _status &= ~(1u << 21);
    } else {
      _status &= ~(1u << 19);
      _status |= (1u << 21);
    }
    
    if (!_rxfifo.halfEmpty()) {
      _status |= (1u << 15);
    } else {
      _status &= ~(1u << 15);
    }
    
    if (_txfifo.halfEmpty()) {
      _status |= (1u << 14);
    } else {
      _status &= ~(1u << 14);
    }

    if (_rxfifo.full()) {
      _status |= (1u << 17);
    } else {
      _status &= ~(1u << 17);
    }

    return _status;
  }

  void arg(u32 value) { _arg = value; }
  u32 arg() { return _arg; }

  void clk(u32 value) { clock = value; }
  u32 clk() { return clock; }

  void mask0(u32 value) { _mask0 = value; }
  u32 mask0() { return _mask0; }

  void mask1(u32 value) { _mask1 = value; }
  u32 mask1() { return _mask1; }

  bool on() { return ps == PowerState::on; }

  void powerstuff(u32 word) {
    ps = PowerState(word & 0b11);
    powerctl = word;
  }

  u32 powerstuff() { return powerctl; }

  void cmdSent() { _status |= (u32(1) << 7); }

  void cmdRespEnd() { _status |= (u32(1) << 6); }

  void dataEnd() { _status |= (u32(1) << 8); }

  void dataBlockEnd() { _status |= (u32(1) << 10); }

  void rxAvail() { _status |= (1u << 21); }

  void clearStatus(u32 v) { _status &= ~(v); }

  void doCommand(u32 cmd) {
    assert(((cmd >> 8) & 0x3) == 0);
    u8 idx = cmd & 0x3f;
    Reply rep = Reply::arsenalucl;

    if ((cmd >> 5) & 1)
      rep = Reply::small;
    if ((cmd >> 6) & 1)
      rep = Reply::big;

    bool suc = sd.doCommand(idx, arg(), (u32 *)&cmdbuf);

    if (suc) {
      rep == Reply::arsenalucl ? cmdSent() : cmdRespEnd();
    } else {
      assert(false);
    }
  }

  u32 response(u32 n) { return cmdbuf[n]; }

  static u32 read(u32 addr, u8 width, PL180 *ctx) {
    // assert(false);
    u32 offt = addr & 0xfff;
    // printf("..mmc r %x status %b datacount: %d\n", offt, ctx->_status,
           // ctx->_datacnt);
    // fgetc(stdin);
    if (offt >= 0x80 and offt <= 0xbc) {
      // printf("reading... datacnt: %d width: %d\n", ctx->_datacnt, width);
      return ctx->rxfifo();
    }
    switch (offt) {
    case 0:
      return ctx->powerstuff();
    case 0x4:
      return ctx->clk();
    case 0x3c:
      return ctx->mask0();
    case 0x14:
      return ctx->response(0);
    case 0x18:
      return ctx->response(1);
    case 0x1c:
      return ctx->response(2);
    case 0x20:
      return ctx->response(3);
    case 0x34:
      return ctx->status();
    case 0x40:
      return ctx->mask1();
    case 0xfe0:
      return 0x81;
    case 0xfe4:
      return 0x11;
    case 0xfe8:
      return 0x4;
    case 0xfec:
      return 0x0;
    default:
      printf("Unhandled offset: %x\n", offt);
      assert(false);
    }
  }
  static void write(u32 addr, u32 value, u8 width, PL180 *ctx) {
    // assert(false);
    // printf("..mmc w\n");

    u32 offt = addr & 0xfff;
    if (offt >= 0x80 and offt <= 0xbc) {
      return ctx->txfifo(value);
    }
    switch (offt) {
    case 0:
      return ctx->powerstuff(value);
    case 4:
      return ctx->clk(value);
    case 8:
      return ctx->arg(value);
    case 0xc:
      return ctx->doCommand(value);
    case 0x24:
      return;
    case 0x28:
      return ctx->datalen(value);
    case 0x2c:
      return ctx->datactl(value);
    case 0x38:
      return ctx->clearStatus(value);
    case 0x3c:
      return ctx->mask0(value);
    case 0x40:
      return ctx->mask1(value);
    default:
      printf("Unhandled offset: %x value %b\n", offt, value);
      assert(false);
    }
  }

  Region getRegion(u32 start) {
    return Region{
        .start = start,
        .len = 0x1000,
        .isram = false,
        .ctx = this,
        .r = (u32(*)(u32, u8, void *)) & read,
        .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
