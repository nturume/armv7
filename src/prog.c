char stack[1024];

int array[10];

#define ARSENAL_UCL 0

int main() {
  long long *ptr = (long long *)0x40000000ul;
  long long a = *ptr;
  *ptr = 0;
  return 0;
}

__attribute__((naked))
void _start () {
  asm volatile("ldr sp, =stack+1024");
  asm volatile("bl main");
  asm volatile("wfe");
  asm volatile("b .");
}
