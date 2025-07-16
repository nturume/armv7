#include "./bin.hpp"
#include "./decoder.hpp"
#include "./elf.hpp"
#include "./mem.hpp"
#include "./stuff.hpp"
#include "arith.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include "uart.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "sp810.hpp"
#include "sp804.hpp"
#include "flash.hpp"
#include "mmc.hpp"
#include "lan9118.hpp"
#include <thread>


#define UBOOT_BASE 0x60800000
#define HIGMEM_BASE 0x60000000
// #define HIGMEM2_BASE 0x70000000
#define FLASH1_BASE 0x40000000
#define FLASH2_BASE 0x44000000

int main(int argc, const char *argv[], const char *envp[]) {
  // Cpu::test();
  Rubber<u8> rubber;
  // return 0;
  Cpu c;
  c.mem.bitset.reset();
  assert(!c.mem.bitset.any());

  Flash flash1(64*1024*1024);
  Flash flash2(64*1024*1024);

  c.mem.addRegion(flash1.getRegion(FLASH1_BASE));
  c.mem.addRegion(flash2.getRegion(FLASH2_BASE));
  
  c.mem.newRam(HIGMEM_BASE, 1024*1024*1024); //16M ram
  // c.mem.newRam(HIGMEM2_BASE, 256*1024*1024); //16M ram

  SP810 sp810;
  PL011 uart0;
  SP804 timer01;
  PL180 pl180;
  LAN9118 eth;
  
  c.mem.addRegion(sp810.getRegion(0x10001000));
  c.mem.addRegion(uart0.getRegion(0x10009000));
  c.mem.addRegion(timer01.getRegion(0x10011000));
  c.mem.addRegion(pl180.getRegion(0x10005000));
  c.mem.addRegion(eth.getRegion(0x4e000000));
  

  c.mem.loadBin(UBOOT_BASE,"/home/m/Desktop/u-boot-2025.07/u-boot.bin");
  
  // Elf elf("/home/m/Desktop/u-boot-2025.07/u-boot");

  // Elf::ProgramHeaderIterator iter(elf.file, elf.header);

  // c.mem.loadElf(iter);
  // Ram *stack = c.mem.newRam(0x40000000, 1024 * 1024);

  // u32 sp = stack->loadArgs(argv, envp+10);

  // c.apsr.b.m = u8(Cpu::Mode::user);

  // c.mem.regions.push_back(UART::getRegion());

  std::thread t(PL011::rxgetc, &uart0);

  c.reset();

  c.pcReal(UBOOT_BASE);

  // printf("ENTRY %x\n", elf.header.e_entry);
  // printf("HEAP BEGIN %x\n", c.mem.program_end);

  // Ram *heap = c.mem.newRam(c.mem.program_end, 1024 * 1024);

  c.sp(0);
  c.r(0, 0);
  c.r(1, 0);
  c.r(2, 0);
  c.r(3, 0);
  c.r(4, 0);
  c.r(5, 0);
  c.r(6, 0);
  c.r(7, 0);
  c.r(8, 0);
  c.r(9, 0);
  c.r(10, 0);
  c.r(11, 0);
  c.r(12, 0);
  c.r(14, 0);

  // u32 count = 0;
  u32 prev;
  // r10 = 0x93000 r0 = 0x170

  // u32 bug = c.mem.a32u(0x93000+0x170);

  // printf("BUGGG: %x\n", bug);

  // fgetc(stdin);

  // {
  //     u8 *ptr = (u8*)c.mem.sysPtr(0x4080eeb8);
  //     for(u32 i = 0; i < 0xcc; i++) {
  //       *ptr=0xff;
  //       ptr += 1;
  //     }

  // }

  // c.printRegisters();
  bool step_mode = false;

  u32 count = 0;

  while (true) {
    // printf("pc: %x sp: %x, r0: %x\n", c.pcReal(), c.r(13), c.r(0));
    //  c.printMode(c.M());
    //  printf("ttbcr: %d\n", c.mem.mmu.ttbr0.back);
    try {
      // c.printRegisters();
      // if(count==10)break;
      u32 word = c.mem.a32u(c.pcReal());
      // if(word==prev) count+=1; else count=0;
      // printf("                          %x\n", c.pcReal());
      if(c.disasmode) c.disasm(c.pcReal(), word);
      // Decoder::printInstr(Decoder::decode(word, c.currInstrSet()));
      u32 pc = c.exec(word);
      // prev = word;
      c.pcReal(pc);
    } catch (InvalidRegion &in) {
      printf("===invalid %s region=== %x\n",in.byread?"read":"write", in.address);
      c.printRegisters();
      // c.pcReal(c.takeDataAbortException());
      abort();
    } catch (Memory::MMU::PageFault &pf) {
      printf("===page fault=== %x\n", pf.vaddr);
      abort();
    }

    // if(c.pcReal()==0x184c8) {
    //   u8 *ptr2 = (u8*)c.mem.sysPtr(0x4080ef80);
    //   u8 *ptr = (u8*)c.mem.sysPtr(0x4080ef78);
    //   u8 *ptr3 = (u8*)c.mem.sysPtr(0x4080ef78);
    //   for(u32 i = 0; ptr != ptr2; i++) {
    //     printf("checking....%d %ld\n", i, (u64)ptr2-(u64)ptr3);
    //     assert(*ptr==0);
    //     ptr += 1;
    //   }
    // }

    // if (c.pcReal()==0x7ffc27e8 or step_mode) {
    //   step_mode = true;
    //   c.disasmode = true;
    //   c.printRegisters();
    //   c.dbgStack(0,10);
    //   fgetc(stdin);
    // }

    // if(c.disasmode) {
      // c.printRegisters();
    // }

    // 400210
    // 400feea8

    count += 1;

    if((count&0xfff)) {
      bool t1 = timer01.tick1();
      bool t2 = timer01.tick2();

      assert(!t1 and !t2);
    }
  }
}

// 4080f09c

// 0x0006a480

// #define RBASE 0x60000000
// #define ABASE 0x60000100
// #define KBASE 0x60010000
// #define FBASE 0x60800000

// int main() {
//   // Cpu::test();
//   // return 0;
//   Cpu c;
//   // Elf elf("/build/c.elf");

//   // c.mem.rambase = 0x1000000;
//   c.mem.rambase = RBASE;
//   // Elf::ProgramHeaderIterator iter(elf.file, elf.header);

//   // c.mem.loadElf(iter, c.mem.rambase);
//   c.mem.loadBinary("/home/m/Desktop/a32/atags",ABASE);
//   c.mem.loadBinary("/home/m/Desktop/a32/zImage",KBASE);
//   c.mem.loadBinary("/home/m/Desktop/a32/initramfs",FBASE);

//   c.mem.regions.push_back(UART::getRegion());
//   c.reset();

//   c.r(0, 0);
//   c.r(1, 2272);
//   c.r(2, ABASE);

//   c.pcReal(KBASE);
//   // printf("ENTRY %x\n", elf.header.e_entry);

//   u32 count = 0;
//   u32 prev;

//   u32 break_point = 0x60010104;

//   bool stepi = false;

//   while (true) {
//     // printf("pc: %u sp: %u, r0: %x\n", c.pcReal(), c.r(13), c.r(0));
//     // c.printMode(c.M());
//     // printf("ttbcr: %d\n", c.mem.mmu.ttbr0.back);
//     try {
//       //if(count==10)break;
//       u32 word = c.mem.a32u(c.pcReal());
//       //if(word==prev) count+=1; else count=0;
//       // printf("%x %x RL: %x R6 %x ",c.pcReal(), word, c.r(14), c.r(6));

//       printf("%x %x ",c.pcReal(), c.r(14));
//       Decoder::printInstr(Decoder::decodeA(word));
//       u32 pc = c.exec(word);

//       if(pc==break_point or stepi) {
//         c.printRegisters();
//         stepi = true;
//       }
//      // prev = word;
//       c.pcReal(pc);
//     } catch (InvalidRegion &in) {
//       printf("===invalid region=== %x\n", in.address);
//       // c.pcReal(c.takeDataAbortException());
//       abort();
//     } catch (Memory::MMU::PageFault &pf) {
//       printf("===page fault=== %x\n", pf.vaddr);
//       abort();
//     }
//     if(stepi){
//       if(fgetc(stdin)=='c') stepi = false;
//     }
//   }
// }
