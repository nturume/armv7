
#pragma once
#include "fifo.hpp"
#include "mem.hpp"
#include "stuff.hpp"
#include <cassert>
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
        u32 SD_SPEC4 : 1 = 0;
        u32 EX_SECURITY : 4 = 0;
        u32 SD_SPEC3 : 1 = 0;
        u32 SD_BUS_WIDTHS : 4 = 5;
        u32 SD_SECURITY : 3 = 0;
        u32 DATA_STAT_AFTER_ERASE : 1 = 0;
        u32 SD_SPEC : 4 = 0;
        u32 SCR_STRUCTURE : 4 = 0;
      } b = {};
      u32 back[2];
    };

    union CSD {

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
        u32 WRITE_BL_LEN : 4 = 9;
        u32 R2W_FACTOR : 3 = 0b10;
        u32 _4 : 2 = 0;
        u32 WP_GRP_ENABLE : 1 = 0;
        u32 WP_GRP_SIZE : 7 = 0x7f;
        u32 SECTOR_SIZE : 7 = 0x7f;
        u32 ERASE_BLK_EN : 1 = 1;
        //=====
        u32 C_SIZE_MULT : 3 = 3;
        u32 VDD_W_CURR_MAX : 3 = 0;
        u32 VDD_W_CURR_MIN : 3 = 0;
        u32 VDD_R_CURR_MAX : 3 = 0;
        u32 VDD_R_CURR_MIN : 3 = 0;
        u32 C_SIZE : 12 = 2000;
        // 22
        u32 _5 : 2 = 0;
        u32 DSR_IMP : 1 = 0;
        u32 READ_BLK_MISALIGN : 1 = 0;
        u32 WRITE_BLK_MISALIGN : 1 = 0;
        u32 READ_BL_PARTIAL : 1 = 1;
        u32 READ_BL_LEN : 4 = 9;
        u32 CCC : 12 = 0b10110110101;
        //====
        u32 TRAN_SPEED : 8 = 0xff;
        u32 NSAC : 8 = 0;
        u32 TAAC : 8 = 0xe;
        u32 CSD_STRUCTURE : 8 = 0;
      } b = {};

      u32 back[4];

      // CSD() { b.C_SIZE = 0; }
    };

    static_assert(sizeof(CSD) == 16, "");

    enum class Cmd {
      GO_IDLE_STATE = 0,
      SEND_IF_COND = 8,
      APP_CMD = 55,
      ALL_SEND_CID = 2,
      SEND_RELATIVE_ADDR = 3,
      SEND_CSD = 9,
      SELECT = 7,
      SWITCH_FUNC = 6,
      SET_BLOCKLEN = 16,
      READ_SINGLE_BLOCK = 17,
      READ_MULTIPLE_BLOCK = 18,
      STOP_TRANSMISSION = 12,
    };

    enum class AppCmd {
      SD_SEND_OP_COND = 41,
      SEND_SCR = 51,
      SET_BUS_WIDTH = 6,
      SD_STATUS = 13,
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

    bool select(u32 word) {
      u16 rca = ((word >> 16) & 0xff);

      if (!stdbyOrTran()) {
        return false;
      }

      if (rca == 1) {
        state(State::tran);
      } else if (rca == 0) {
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

    bool ready() { return _state == State::ready || _state == State::tran; }

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

    void poll() {
      if (blockrem == 0 and !readmultblocks)
        return;
      
      if (blockrem == 0 and readmultblocks)
        blockrem = i32(blocklen);

      for (; blockrem > 0 and !controller->_rxfifo.full();) {
        controller->rxfifo(0);
        blockrem -= 4;
      }

      if (blockrem == 0) {
        controller->cmdRespEnd();
        controller->dataBlockEnd();
      }

      assert(blockrem >= 0);
    }

    bool doAppCommand(u8 command, u32 arg, u32 *buf) {
      // printf("==========> app command: %d\n", command);
      switch (AppCmd(command)) {
      case AppCmd::SD_SEND_OP_COND: {
        state(State::ready);
        *buf = 0x81000000;
        return true;
      }
      case AppCmd::SEND_SCR: {
        controller->rxfifo(swap32(SCR{}.back[1]));
        controller->rxfifo(swap32(SCR{}.back[0]));
        controller->cmdRespEnd();
        controller->dataBlockEnd();
        *buf = getStatus();
        return true;
      }
      case AppCmd::SET_BUS_WIDTH: {
        *buf = getStatus();
        return true;
      }
      case AppCmd::SD_STATUS: {
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
      if (expectapp) {
        expectapp = false;
        return doAppCommand(command, arg, buf);
      }
      // printf("==========> command: %d\n", command);
      switch (Cmd(command)) {
      case Cmd::GO_IDLE_STATE: {
        state(State::idle);
        return true;
      }
      case Cmd::SEND_IF_COND: {
        *buf = 1u << 8;
        *buf |= arg & 0xff;
        return true;
      }
      case Cmd::APP_CMD: {
        *buf = getStatus();
        expectapp = true;
        return true;
      }
      case Cmd::SEND_CSD: {
        state(State::stby);
        CSD csd;
        buf[3] = csd.back[0];
        buf[2] = csd.back[1];
        buf[1] = csd.back[2];
        buf[0] = csd.back[3];
        return true;
      }
      case Cmd::ALL_SEND_CID: {
        state(State::ident);
        CSD csd;
        // TODO
        return true;
      }
      case Cmd::SEND_RELATIVE_ADDR: {
        *buf = 0x100;
        return true;
      }
      case Cmd::SELECT: {
        bool done = select(arg);
        *buf = getStatus();
        return done;
      }
      case Cmd::SET_BLOCKLEN: {
        blocklen = arg;
        // printf("NEW BLOCK LEN: %d\n", blocklen);
        *buf = getStatus();
        return true;
      }
      case Cmd::READ_SINGLE_BLOCK: {
        readmultblocks = false;
        *buf = getStatus();
        addr = arg;
        blockrem = i32(blocklen);
        poll();
        return true;
      }
      case Cmd::READ_MULTIPLE_BLOCK: {
        readmultblocks = true;
        *buf = getStatus();
        addr = arg;
        blockrem = i32(blocklen);
        poll();
        return true;
      }
      case Cmd::STOP_TRANSMISSION: {
          readmultblocks = false;
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
    _datacnt -= amount;
    if (_datacnt == 0) {
      _status |= (1u << 8);
    } else {
      _status &= ~(1u << 8);
    }
    // TODO check overflow
  }

  void txfifo(u32 v) {
    if (_txfifo.full()) {
      return;
    }
    _txfifo.write(v);
    // _datacnt -= 1;
  }

  u32 txfifo() {
    if (_txfifo.empty()) {
      _status |= (1u << 4);
      return 0;
    }
    if (!tranFromCard())
      dataCntDown();
    return _txfifo.read();
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
    return _rxfifo.read();
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
    sd.poll();

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
    // printf("..mmc r %x status %b\n", offt, ctx->_status);
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
