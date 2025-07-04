#include "uart.hpp"

namespace UART {
PL011 uart0 = {};

static PL011 *getUart(u32 addr) { return &uart0; }

u32 read(u32 addr, u8 width, void *ctx) {
  (void)ctx;
  PL011 *uart = getUart(addr);
  u32 offset = addr & 0xfff;
  //printf("UART was read!!!! width: %d %d\n", width, offset);
  switch (PL011::OFFT(offset)) {
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

void write(u32 addr, u32 value, u8 width, void *ctx) {
  (void)ctx;
  //printf("UART was written!!! width: %d\n", width);
  PL011 *uart = getUart(addr);
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

Region getRegion() {
  return {
      .start = 0x80009000,
      .len = 0x1000,
      .r = &read,
      .w = &write,
  };
}

} // namespace UART
