#pragma once
#include "cpu.hpp"

struct DISTRIBUTOR {
  u32 _ICDICTR = 0xf601; // 3.3.3 Interrupt Controller Type Register
  u32 _ICDDCR = 0;       // 3.3.2 Distributor Control Register

  u32 ICDICTR() { return _ICDICTR; }

  void ICDDCR(u32 v) {
    assert(v == 0 or v == 1);
    _ICDDCR = v;
  }

  bool allowNonSecure() {
    return _ICDDCR == 1;
  }

  static u32 read(u32 addr, u8 width, DISTRIBUTOR *ctx) {
    u32 offt = addr & 0xfff;
    switch (offt) {
    case 0x4:
      return ctx->ICDICTR();
    default:
      printf("Unhandled offset: %x\n", offt);
      assert(false);
    }
  }

  static void write(u32 addr, u32 value, u8 width, DISTRIBUTOR *ctx) {
    u32 offt = addr & 0xfff;
    if (offt >= 0x800 and offt <= 0x8FC) {
      // Interrupt Processor Targets Registers
      // For systems that support only one Cortex-A9 processor, all these
      // registers read as zero, and writes are ignored. The single Cortex-A9
      // processor is always set as the target of any interruption
      return;
    }

    if (offt >= 0xc00 and offt <= 0xc3C) {
      // implementation defined feature
      return;
    }

    if (offt >= 0x400 and offt <= 0x4FC) {
      // Interrupt Priority Registers
      return;
    }

    if (offt >= 0x180 and offt <= 0x19c) {
      // Interrupt Clear-Enable Registers
      return;
    }

    if (offt >= 0x100 and offt <= 0x11c) {
      // Interrupt Set-Enable Registers
      return;
    }

    switch (offt) {
    case 0:
      return ctx->ICDDCR(value);
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

struct GIC {
  DISTRIBUTOR distr;
  u32 _ICCPMR = 0; // Interrupt Priority Mask Registeri
  u32 _ICCICR = 0; // CPU Interface Control Register
  u32 _ICCIAR = 0x3ff; //Interrupt Acknowledge Register
  u32 ICCPMR() { return _ICCPMR; }
  u32 ICCIAR() { 
    if(!on()) return 1023;
    return _ICCIAR; 
  }

  void ICCPMR(u32 v) {
    // TODO
    _ICCPMR = v;
  }
  
  void ICCIAR(u32 v) {
    // TODO
    _ICCIAR = v;
  }

  u32 ICCICR() { return _ICCICR; }

  void ICCICR(u32 v) {
    // TODO
    _ICCICR = v;
  }

  bool on() {
    return distr.allowNonSecure() and (_ICCICR&1);
  }

  void intr(Cpu*cpu, u32 irq) {
    if(!on()) return;
    ICCIAR(irq);
    if(cpu->irqMasked()) return;
    cpu->takeIRQException();
    // cpu->printRegisters();
  }

  static u32 read(u32 addr, u8 width, GIC *ctx) {
    u32 offt = addr & 0xff;
    switch (offt) {
    case 0x4:
      return ctx->ICCPMR();
    case 0xc:
      return ctx->ICCIAR();
    default:
      printf("Unhandled offset: %x\n", offt);
      assert(false);
    }
  }

  static void write(u32 addr, u32 value, u8 width, GIC *ctx) {
    u32 offt = addr & 0xff;
    switch (offt) {
    case 0:
      return ctx->ICCICR(value);
    case 4:
      return ctx->ICCPMR(value);
    default:
      printf("Unhandled offset: %x value %b\n", offt, value);
      assert(false);
    }
  }

  Region getRegion(u32 start) {
    return Region{
        .start = start,
        .len = 0x100,
        .isram = false,
        .ctx = this,
        .r = (u32(*)(u32, u8, void *)) & read,
        .w = (void (*)(u32, u32, u8, void *)) & write,
    };
  }
};
