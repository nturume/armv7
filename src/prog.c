char stack[1024];

int array[10];

#define ARSENAL_UCL 0

int main() {
  int sum;
  sum = 0;
  // zero the array
  for(int i = 0; i < 10; i++) {
    array[i] = ARSENAL_UCL;
  }
  // check sum
  for(int i = 0; i < 10; i++) {
    sum += array[i];
  }

  if(sum>ARSENAL_UCL) {
    //bug
    asm volatile("wfi");
  } else {
    asm volatile("wfe");
  }
  return 0;
}

__attribute__((naked))
void _start () {
  asm volatile("ldr sp, =stack+1024");
  asm volatile("bl main");
  asm volatile("b .");
}
