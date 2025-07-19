
#pragma once
#include "fifo.hpp"
#include "mem.hpp"
#include "stuff.hpp"
#include <cassert>

struct LAN9118 {
  //{
  struct MAC {
    struct PHY {
      u16 regs[32] = {
          [0] = 0,      // 5.5.1 BASIC CONTROL REGISTER
          [1] = 0x7809, // 5.5.2 BASIC STATUS REGISTER

      };

      void reset() { *this = {}; }

      bool isLoopBack() { return (regs[0] >> 14) & 1; }

      bool poweredDown() { return (regs[0] >> 11) & 1; }

      void linkUp() {
        regs[1] |= (1u << 2);
        // TODO
        regs[1] |= (1u << 5);
      }

      void ctl(u16 v) {
        if ((v >> 15) & 1) {
          // reset;
          return reset();
        }
        regs[0] = v;
        assert(!isLoopBack());
        if (!poweredDown()) {
          linkUp();
        }
      }

      u16 reg(u8 offt) {
        switch (offt) {
        case 1:
        case 0:
          return regs[offt];
        default:
          printf("unahndled phy reg %d\n", offt);
          assert(false);
        }
      }

      void reg(u8 offt, u16 v) {
        switch (offt) {
        case 0:
          return ctl(v);
        case 1:
          return; // RO
        default:
          printf("unahndled phy reg %d\n", offt);
          assert(false);
        }
      }
    };

    PHY phy;
    u32 regs[13] = {
        [0] = 0,          // not used
        [1] = 0x00040000, // MAC_CR MAC Control Register 00040000h
        [2] = 0x5254,     // ADDRH MAC Address High 0000FFFFh
        [3] = 0x12345678, // ADDRL MAC Address Low FFFFFFFFh
        [4] = 0,          // HASHH Multicast Hash Table High 00000000h
        [5] = 0,          // HASHL Multicast Hash Table Low 00000000h
        [6] = 0,          // MII_ACC MII Access 00000000h
        [7] = 0,          // MII_DATA MII Data 00000000h
        [8] = 0,          // FLOW Flow Control 00000000h
        [9] = 0,          // VLAN1 VLAN1 Tag 00000000h
        [10] = 0,         // VLAN2 VLAN2 Tag 00000000h
        [11] = 0,         // WUFF Wake-up Frame Filter 00000000h
        [12] = 0,         // WUCSR Wake-up Status and Control 00000000h
    };

    //{
    bool miiBusy() { return regs[6] & 1; }
    bool miiclrBusy() { return regs[6] &= 0xfffffffe; }
    bool miiWrite() { return (regs[6] >> 1) & 1; }
    bool phyAddr() { return (regs[6] >> 6) & 0x1f; }
    bool phyAwake() {
      return true; // TODO
    }
    void miiAcc(u32 v) {
      assert(!miiBusy());
      regs[6] = v;
      assert(miiBusy());
      if (miiWrite()) {
        printf("   phy write.. %d\n", phyAddr());
        phy.reg(phyAddr(), regs[7]);
      } else {
        printf("   phy read.. %d\n", phyAddr());
        regs[7] = phy.reg(phyAddr());
      }
      miiclrBusy();
    }
    //}

    void reset() {
      assert(phyAwake());
      *this = {};
    }

    u32 reg(u8 offt) {
      switch (offt) {
      case 7:
      case 3:
      case 2:
        return regs[offt]; // mac
      case 6:
        return regs[6];
      default:
        printf("unahndled mac reg %d\n", offt);
        assert(false);
      }
    }

    void ctl(u32 v) {
      regs[1] = v;
      // TODO
    }

    void reg(u8 offt, u32 v) {
      switch (offt) {
      case 6:
        return miiAcc(v);
      case 1:
        return ctl(v);
      case 2:
      case 3:
      case 7:
      case 8:
        regs[offt] = v;
        break; // idgaf
      default:
        printf("unahndled mac reg %d\n", offt);
        assert(false);
      }
    }
  };
  MAC mac;
  u32 MAC_CSR_CMD = 0;
  u32 MAC_CSR_DATA = 0;
  inline bool maccsrBusy() { return (MAC_CSR_CMD >> 31) & 1; }
  inline void macclrcsrBusy() { MAC_CSR_CMD &= (0x7fffffff); }
  inline bool maccsrRead() { return (MAC_CSR_CMD >> 30) & 1; }
  inline u8 maccsrAddress() { return MAC_CSR_CMD & 0xff; }
  inline u32 maccsrCMD() { return MAC_CSR_CMD; }
  inline u32 maccsrDATA() { return MAC_CSR_DATA; }
  inline void maccsrDATA(u32 v) { MAC_CSR_DATA = v; }
  inline void maccsrCMD(u32 v) {
    assert(!maccsrBusy());
    MAC_CSR_CMD = v;
    assert(maccsrBusy());
    if (maccsrRead()) {
      printf(" mac read %d\n", maccsrAddress());
      MAC_CSR_DATA = mac.reg(maccsrAddress());
    } else {
      printf(" mac write %d\n", maccsrAddress());
      mac.reg(maccsrAddress(), MAC_CSR_DATA);
    }
    macclrcsrBusy();
  }

  void macscrReset() {
    mac.reset();
    MAC_CSR_CMD = 0;
    MAC_CSR_DATA = 0;
    // TODO
  }
  //}

  //{
  u32 PMT_CTRL = 0;
  u32 pmtctrl() { return PMT_CTRL | 1; }
  void pmtctrl(u32 v) { PMT_CTRL = v; } // idgaf
  //}

  //{
  u32 BYTE_TEST = 0x87654321;
  inline u32 byteTest() { return BYTE_TEST; }
  inline void byteTest(u32 v) {
    printf("BYTE TEST from 0x%x to 0x%x\n", BYTE_TEST, v);
  }
  //}

  //{
  u32 INT_EN = 0;
  u32 intEn() { return INT_EN; };
  void intEn(u32 v) { INT_EN = v; }
  //}

  /*{*/
  DYNFIFO<u32> txfifo;
  DYNFIFO<u32> rxfifo;
  u32 *txstatus = nullptr;
  u32 *rxstatus = nullptr;
  u8 __attribute__((aligned(alignof(u32)))) fifoback[16 * 1024];
  u32 HW_CFG = 00050000;
  u8 txfifsz() { return (HW_CFG >> 16) & 0xf; }
  bool is32bit() { return (HW_CFG >> 2) & 1; }
  void hwCFG(u32 v) {
    // assert((v>>20)&1);
    HW_CFG = v;
    if (HW_CFG & 1) {
      // soft reset
      printf("lan reset...\n");
      macscrReset();
      HW_CFG &= 0xfffffffe;
      return;
    }
    u8 txsz = txfifsz();
    assert(txsz > 0 and txsz <= 14);
    u8 rxsz = 16 - txsz;
    printf("****** Rx_SZ %d TX_SZ %d\n", rxsz, txsz);
    u32 txszbytes = txsz * 1024;
    u32 rxszbytes = rxsz * 1024;

    u32 txstatusbytes = 512;
    u32 rxstatusbytes = rxszbytes / 16;

    txfifo.N = (txszbytes - txstatusbytes) / 4;
    rxfifo.N = (rxszbytes - rxstatusbytes) / 4;

    txstatus = (u32 *)fifoback;
    rxstatus = (u32 *)fifoback + txszbytes;

    txfifo.buf = (u32 *)fifoback + 512;
    rxfifo.buf = (u32 *)(fifoback + txszbytes + rxstatusbytes);
  }
  /*}*/

  /*{*/
  u32 E2P_CMD = 0x0;
  inline bool e2pBusy() { return (E2P_CMD >> 31) & 1; }
  inline bool e2pclrBusy() { return E2P_CMD &= 0x7fffffff; }
  inline u8 e2pGetCmdIdx() { return (E2P_CMD >> 28) & 7; }
  void e2pCMD(u32 v) {
    assert(!e2pBusy());
    E2P_CMD = v;
    assert(e2pBusy());
    switch (e2pGetCmdIdx()) {
    case 0b111: {
      E2P_CMD |= (1u << 8);
      break;
    }
    default:
      printf("Unhandled e2p cmd: %d\n", e2pGetCmdIdx());
      abort();
    }
    e2pclrBusy();
  }
  u32 e2pCMD() { return E2P_CMD; }
  /*}*/

  /*{ honestly idgaf*/
  u32 AFC_CFG = 0;
  u32 afcCFG() { return AFC_CFG; };
  void afcCFG(u32 v) { AFC_CFG = v; }
  /*}*/

  /*{*/
  u32 GPIO_CFG = 0;
  u32 gpioCFG() { return GPIO_CFG; };
  void gpioCFG(u32 v) { GPIO_CFG = v; }
  /*}*/

  /*{*/
  u32 GPT_CFG = 0xffff;
  u32 gptCFG() { return GPT_CFG; };
  inline bool gptEnabled(u32 v) { return (v >> 29) & 1; }
  void gptCFG(u32 v) {
    if (gptEnabled(GPT_CFG) and !gptEnabled(v)) {
      GPT_CFG |= 0xffff;
    }
    GPT_CFG = v;
    assert(!gptEnabled(GPIO_CFG));
  }
  /*{*/

  /*{*/
  u32 TX_CFG = 0;
  u32 txCFG() { return TX_CFG; };
  void txCFG(u32 v) {
    TX_CFG = v;
    if (txSTOP())
      txSTOP(true);
    if (txdDump()) {
      // TODO
    }
    if (txsDump()) {
      // TODO
    }
  }
  bool txsDump() { return (TX_CFG >> 15) & 1; }
  bool txdDump() { return (TX_CFG >> 14) & 1; }
  bool txAO() { return (TX_CFG >> 2) & 1; }
  bool txON() { return (TX_CFG >> 1) & 1; }
  bool txSTOP(bool stop = false) {
    if (stop)
      TX_CFG &= 0xfffe;
    return (TX_CFG) & 1;
  }
  /*}*/

  /*{*/
  u32 RX_CFG = 0;
  u32 rxCFG() {
    return RX_CFG;
    // TODO
  };
  void rxCFG(u32 v) { RX_CFG = v; }
  u16 rxDmaCnt() { return (RX_CFG >> 12) & 0xfff; }
  bool rxdDump(bool dump) {
    if (dump)
      RX_CFG &= (1u << 15);
    return (RX_CFG >> 15) & 1;
  }
  u8 rxdoff() { return (RX_CFG >> 8) & 0x1f; }
  u8 rxEndAlignmnt() {
    switch ((RX_CFG) >> 30) {
    case 0:
      return 4;
    case 1:
      return 16;
    case 2:
      return 32;
    }
    unrch();
  }
  /*}*/

  inline u32 IDREV() { return 0x1180001; }

  u32 rx() {
    // TODO
    unrch();
  }

  void tx(u32 v) {
    // TODO
    unrch();
  }

  static u32 read(u32 addr, u8 width, LAN9118 *ctx) {
    assert(width == 4);
    // assert(false);
    u32 offt = addr & 0xffffff;
    printf("eth read offt 0x%x (%u) %d\n", offt, offt, width);
    if (offt >= 0 and offt <= 0x1c) {
      // rx
      return ctx->rx();
    }
    switch (offt) {
    case 0x50:
      return ctx->IDREV();
    case 0x64:
      return ctx->byteTest();
    case 0xa4:
      return ctx->maccsrCMD();
    case 0xa8:
      return ctx->maccsrDATA();
    case 0x84:
      return ctx->pmtctrl();
    case 0xb0:
      return ctx->e2pCMD();
    default:
      printf("Unhandled offt 0x%x (%u)\n", offt, offt);
      assert(false);
    }
  }
  static void write(u32 addr, u32 value, u8 width, LAN9118 *ctx) {
    // assert(false);
    assert(width == 4);
    u32 offt = addr & 0xffffff;
    printf("eth write offt 0x%x (%u) %d\n", offt, offt, width);
    if (offt >= 020 and offt <= 0x3c) {
      return ctx->tx(value);
    }
    switch (offt) {
    case 0x64:
      return ctx->byteTest(value);
    case 0xa4:
      return ctx->maccsrCMD(value);
    case 0x84:
      return ctx->pmtctrl(value);
    case 0x5c:
      return ctx->intEn(value);
    case 0x74:
      return ctx->hwCFG(value);
    case 0xb0:
      return ctx->e2pCMD(value);
    case 0xa8:
      return ctx->maccsrDATA(value);
    case 0xac:
      return ctx->afcCFG(value);
    case 0x88:
      return ctx->gpioCFG(value);
    case 0x8c:
      return ctx->gpioCFG(value);
    case 0x70:
      return ctx->txCFG(value);
    case 0x6c:
      return ctx->rxCFG(value);
    default:
      printf("Unhandled offt 0x%x (%u)\n", offt, offt);
      assert(false);
    }
  }

  Region getRegion(u32 start) {
    return Region{
        .start = start,
        .len = 0x1000000,
        .isram = false,
        .ctx = this,
        .r = (u32(*)(u32, u8, void *)) & read,
        .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
