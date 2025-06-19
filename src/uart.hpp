#pragma once
#include "mem.hpp"
#include <cstdio>

namespace UART {

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
      u32 cts : 1;
      u32 dsr : 1;
      u32 dcd : 1;
      u32 busy : 1;
      u32 rxfe : 1;
      u32 txff : 1;
      u32 rxff : 1;
      u32 txfe : 1;
      u32 ri : 1;
    } f;
  };

  bool uart_en = true;

  bool enable_fifos = false;
  bool tx_fen = false;
  bool tx_full = false;
  bool rx_full = false;
  bool tx_empty = true;
  bool tx_enabled = false;
  bool rx_enabled = false;
  bool busy = false;
  u8 tx_push_idx = 31;
  u8 tx_pop_idx = 31;

  u8 fract_baud;
  u16 int_baud;
  
  u8 tx_fifo[32];

  void reset() {
    *this = {};
  }

  u32 getUARTFR() {
    return UARTFR{
        .f =
            {
                .busy = busy,
                .rxfe = rx_enabled,
                .txff = tx_full,
                .rxff = rx_full,
                .txfe = tx_enabled,
            },
    }
        .back;
  }

  u32 getUARTLCR_H() {
    if(enable_fifos) {
      return u32(1)<<4;
    }
    return 0;
  }

  u32 getUARTCR() {
    u32 res = 0;
    if(rx_enabled) {
      res |= (1<<9);
    }
    if(tx_enabled) {
      res |= (1<<8);
    }
    if(uart_en) {
      res |= 1;
    }
    return res;
  }

  void lineCtl(u32 lcr) {
    enable_fifos = (lcr>>4)&1;
  }

  inline void enable() { uart_en = true; }

  inline void disable() { uart_en = false; }

  inline u8 decIdx(u8 idx) {
    if (idx == 0)
      return 31;
    return idx - 1;
  }

  inline void enableTxfifo() { tx_fen = true; }

  inline void disableTxfifo() { tx_fen = false; }

  void txPush(u8 data) {
    if (tx_full) {
      return;
    }
    tx_fifo[tx_push_idx] = data;
    tx_push_idx = decIdx(tx_push_idx);
    tx_empty = false;
    if (tx_push_idx == tx_pop_idx or !tx_fen) {
      tx_full = true;
    }
  }

  u8 txPop() {
    if (tx_empty) {
      return 0;
    }
    u8 data = tx_fifo[tx_pop_idx];
    tx_pop_idx = decIdx(tx_pop_idx);
    tx_full = false;
    if (tx_pop_idx == tx_push_idx or !tx_fen) {
      tx_empty = true;
    }
    return data;
  }

  void tx() {
    busy = true;
    while (!tx_empty) {
      printf("%c", txPop());
      fflush(stdout);
    }
    busy = false;
  }

  inline void writeDR(u32 dr) {
    if (tx_enabled and uart_en) {
      txPush(dr);
      tx();
    }
  }

  
  void control(u32 cr) {
    uart_en = cr&1;
    if(uart_en) {
      reset();
    }
    rx_enabled = (cr>>9)&1;
    tx_enabled = (cr>>8)&1;
  }

  void printTxState() {
    printf("full: %d empty: %d pop:%d push: %d\n", tx_full, tx_empty,
           tx_pop_idx, tx_push_idx);
    printf("====================\n");
    for (u8 i = 0; i < 32; i++) {
      printf("[%d] %d\n", i, tx_fifo[i]);
    }
  }
};

u32 read(u32 addr, u8 width);

void write(u32 addr, u32 value, u8 width);

static Region getRegion() {
  return {
      .start = 0x40000000,
      .end = 0x40000100,
      .r = &read,
      .w = &write,
  };
}
}; // namespace UART
