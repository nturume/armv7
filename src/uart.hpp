#pragma once
#include "fifo.hpp"
#include "mem.hpp"
#include "stuff.hpp"
#include <chrono>
#include <cstdio>
#include <thread>

struct PL011 {

  enum class OFFT : u32 {
    UARTDR = 0X0,     // RW
    UARTECR = 0X4,    // RW
    UARTFR = 0X18,    // RO
    UARTILPR = 0X20,  // RW
    UARTIBRD = 0X24,  // RW
    UARTFBRD = 0X28,  // RW
    UARTLCL_H = 0X2C, // RW
    UARTCR = 0X30,    // RW
    UARTIFLS = 0X34,  // RW
    UARTIMSC = 0X38,  // RW
    UARTRIS = 0X3C,   // RO
    UARTMIS = 0X40,   // RO
    UARTICR = 0X44,   // WO
    UARTDMACR = 0X48, // RW
    PID0 = 0XFE0,     // R0 ..
    PID1 = 0XFE4,
    PID2 = 0XFE8,
    PID3 = 0XFEC,
    CID0 = 0XFF0,
    CID1 = 0XFF4,
    CID2 = 0XFF8,
    CID3 = 0XFFC
  };

  union UARTFR {
    u32 back;
    struct {
      u32 cts : 1 = 0;
      u32 dsr : 1 = 0;
      u32 dcd : 1 = 0;
      u32 busy : 1 = 0;
      u32 rxfe : 1 = 0;
      u32 txff : 1 = 0;
      u32 rxff : 1 = 0;
      u32 txfe : 1 = 0;
      u32 ri : 1 = 0;
    } f;
  };

  bool uart_en = true;

  bool enable_fifos = false;
  bool tx_fen = false;
  // bool tx_full = false;
  // bool rx_full = false;
  // bool tx_empty = true;
  bool tx_enabled = true;
  bool rx_enabled = false;
  bool busy = false;
  // u8 tx_push_idx = 31;
  // u8 tx_pop_idx = 31;

  u8 fract_baud;
  u16 int_baud;

  // u8 tx_fifo[32];

  FIFO<u8, 32> tx;

  SharedFIFO<u8, 32> rx;

  void reset() {
    uart_en = true;
    enable_fifos = false;
    tx_fen = false;
    tx_enabled = true;
    rx_enabled = false;
    busy = false;
    rx.drain();
    tx.drain();
  }

  u32 getUARTFR() {
    return UARTFR{
        .f =
            {
                .busy = busy,
                .rxfe = rx.empty(),
                .txff = tx.full(),
                .rxff = rx.full(),
                .txfe = tx.empty(),
            },
    }
        .back;
  }

  u32 getUARTLCR_H() {
    if (enable_fifos) {
      return u32(1) << 4;
    }
    return 0;
  }

  u32 getUARTCR() {
    u32 res = 0;
    if (rx_enabled) {
      res |= (1 << 9);
    }
    if (tx_enabled) {
      res |= (1 << 8);
    }
    if (uart_en) {
      res |= 1;
    }
    return res;
  }

  void lineCtl(u32 lcr) { enable_fifos = (lcr >> 4) & 1; }

  inline void enable() { uart_en = true; }

  inline void disable() { uart_en = false; }

  // inline u8 decIdx(u8 idx) {
  //   if (idx == 0)
  //     return 31;
  //   return idx - 1;
  // }

  inline void enableTxfifo() { tx_fen = true; }

  inline void disableTxfifo() { tx_fen = false; }

  void txPush(u8 data) {
    if (tx.full()) {
      return;
    }
    tx.write(data);
  }

  void rxPush(u8 data) {
    rx.write(data);
  }

  u8 txPop() {
    if (tx.empty()) {
      return 0;
    }
    u8 data = tx.read();
    return data;
  }

  u8 rxPop() {
    return rx.read();
  }

  void txDrain() {
    busy = true;
    while (!tx.empty()) {
      printf("%c", txPop());
      fflush(stdout);
    }
    busy = false;
  }

  inline void writeDR(u32 dr) {
    if (tx_enabled and uart_en) {
      // printf("%c", dr);
      txPush(dr);
      txDrain();
    }
  }
  
  inline u32 readDR() {
    // TODO fill other fields
    if (rx_enabled and uart_en) {
      return rxPop();
    }
    return 0xffffffff;
  }

  void control(u32 cr) {
    uart_en = cr & 1;
    if (uart_en) {
      reset();
    }
    rx_enabled = (cr >> 9) & 1;
    tx_enabled = (cr >> 8) & 1;
  }

  static void rxgetc(PL011 *p) {
    for (;;) {
      u8 ch = fgetc(stdin);
      // printf("stdin: %c\n", ch);
      p->rxPush(ch);
    }
  }

  void printTxState() {
    // printf("full: %d empty: %d pop:%d push: %d\n", tx_full, tx_empty,
    //        tx_pop_idx, tx_push_idx);
    // printf("====================\n");
    // for (u8 i = 0; i < 32; i++) {
    //   printf("[%d] %d\n", i, tx_fifo[i]);
    // }
  }

  static u32 read(u32 addr, u8 width, void *ctx) {
    PL011 *uart = (PL011 *)ctx;
    u32 offset = addr & 0xfff;
    // static u32 nnn = 0;
    // printf("UART was read!!!! width: %d %d %u\n", width, offset, nnn++);
    switch (PL011::OFFT(offset)) {
    case PL011::OFFT::UARTDR:
      return uart->readDR();
    case PL011::OFFT::UARTECR:
      return 0;
    case PL011::OFFT::UARTFR:
      return uart->getUARTFR();
    case PL011::OFFT::UARTIBRD:
      return uart->int_baud;
    case PL011::OFFT::UARTFBRD:
      return uart->fract_baud;
    case PL011::OFFT::UARTLCL_H:
      return uart->getUARTLCR_H();
    case PL011::OFFT::UARTCR:
      return uart->getUARTCR();
    default:
      printf("Unhandled uart read offset: %d\n", offset);
      exit(1);
    }
    return 0;
  }

  static void write(u32 addr, u32 value, u8 width, void *ctx) {
    // printf("UART was written!!! width: %d\n", width);
    PL011 *uart = (PL011 *)ctx;
    u32 offset = addr & 0xfff;

    switch (PL011::OFFT(offset)) {
    case PL011::OFFT::UARTDR:
      return uart->writeDR(value);
    case PL011::OFFT::UARTECR:
      break;
    case PL011::OFFT::UARTIBRD:
      uart->int_baud = value;
      break;
    case PL011::OFFT::UARTFBRD:
      uart->fract_baud = value;
      break;
    case PL011::OFFT::UARTLCL_H:
      return uart->lineCtl(value);
    case PL011::OFFT::UARTCR:
      return uart->control(value);
    default:
      printf("Unhandled uart write offset: %d\n", offset);
      exit(1);
    }
  }

  Region getRegion(u32 start) {
    return {
        .start = start,
        .len = 0x1000,
        .ctx = this,
        .r = (u32(*)(u32, u8, void *)) & read,
        .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
