char stack[1024];

#define UARTBASE 0x40000000

unsigned int *DATAREG = (unsigned int *)(UARTBASE + 0x0);
unsigned int *FLAGSREG = (unsigned int *)(UARTBASE + 0x18);
unsigned int *CTLREG = (unsigned int *)(UARTBASE + 0x30);

int main() {
  // enable uart + tx
  *CTLREG = (1u<<8) | 1u;

  char *m = "Hello UART!\n";

  while(*m) {
    // wait if busy
    while(((*FLAGSREG)>>3)&1);
    *DATAREG = *m;
    m += 1;
  }
  
  return 0;
}

__attribute__((section(".text.reset")))
__attribute__((naked)) void _start() {
  asm volatile("svc #0");
  asm volatile("ldr sp, =stack-1");
  asm volatile("bl main");
  asm volatile("b .");
}
