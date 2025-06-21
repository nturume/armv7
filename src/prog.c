char stack[1024];
char stack1[1024];
char stack2[1024];
char stack3[1024];
char stack4[1024];

#define UARTBASE 0x40000000

unsigned int *DATAREG = (unsigned int *)(UARTBASE + 0x0);
unsigned int *FLAGSREG = (unsigned int *)(UARTBASE + 0x18);
unsigned int *CTLREG = (unsigned int *)(UARTBASE + 0x30);

void print(char *m) {
  while(*m) {
    // wait if busy
    //while(((*FLAGSREG)>>3)&1);
    *DATAREG = *m;
    m += 1;
  }
}

int main() {
  print("Reset!!!!\n");
  //print("UDF returned!!\n");
  return 0;
}

int undef() {
  print("undefined!!!!\n");
  return 0;
}

int svc() {
  print("SVC!!!!\n");
  return 0;
}

int pabt() {
  print("Pabt!!!!\n");
  return 0;
}

int dabt() {
  print("dabt!!!!\n");
  return 0;
}

__attribute__((naked))
void _main(){
  asm volatile("ldr sp, =stack+1024");
  asm volatile("cps #0b10111");
  asm volatile("ldr sp, =stack1+1024");
  asm volatile("cps #0b11011");
  asm volatile("ldr sp, =stack2+1024");
  asm volatile("cps #0b10011");
  
  asm volatile("bl main");
  asm volatile("b .");
}

__attribute__((naked))
void _undef(){
  asm volatile("ldr sp, =stack1+1024");
  asm volatile("sub r14, r14, #0");
  asm volatile("srsdb sp!, #0b10111");
  asm volatile("bl undef");
  asm volatile("rfeia sp!");
  asm volatile("b .");
}

__attribute__((naked))
void _svc(){
  asm volatile("ldr sp, =stack2+1024");
  asm volatile("bl svc");
  asm volatile("b .");
}

__attribute__((naked))
void _pabt(){
  asm volatile("bl pabt");
  asm volatile("b .");
}

__attribute__((naked))
void _dabt(){
  asm volatile("bl dabt");
  asm volatile("b .");
}

__attribute__((section(".text.reset")))
__attribute__((naked)) void _start() {
  asm volatile("ldr pc, = _main");
  asm volatile("ldr pc, = _undef");
  asm volatile("ldr pc, = _svc");
  asm volatile("ldr pc, = _pabt");
  asm volatile("ldr pc, = _dabt");
}
