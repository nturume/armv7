#pragma once
#include "./elf.hpp"
#include "./stuff.hpp"
#include "mmu.hpp"
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <elf.h>
#include <exception>
#include <vector>

#define DEV_BASE 0x40000000

struct Region {
  u32 start;
  u32 len;
  bool isram = false;

  void *ctx = nullptr;

  u32 (*r)(u32 addr, u8 width, void *ptr) = nullptr;
  void (*w)(u32 addr, u32 value, u8 width, void *ptr) = nullptr;
  void *(*sysPtr)(u32 addr, void *ctx) = nullptr;

  bool has(u32 addr) { return addr >= start and addr < (start + len); }

  void *ptr(u32 addr, void *ctx) {
    if (r != nullptr) {
      return sysPtr(addr, ctx);
    }
    assert("region read() is arsenal UCL" == nullptr);
  }

  u32 read(u32 addr, u8 width, void *ctx) {
    if (r != nullptr) {
      return r(addr, width, ctx);
    }
    assert("region read() is arsenal UCL" == nullptr);
  }

  void write(u32 addr, u32 value, u8 width, void *ctx) {
    if (w != nullptr) {
      return w(addr, value, width, ctx);
    }
    assert("region write() is arsenal UCL" == nullptr);
  }
};

struct InvalidRegion : std::exception {
  u32 address;
  u32 byread;
  
  InvalidRegion(u32 addr, bool by) : address(addr), byread(by){};
};

struct Ram {
  u8 *buf;
  u32 len;
  u32 start;
  bool noalloc;

  Ram(u32 start, u32 len, bool nl = false) : len(len), start(start), noalloc(nl) { 
    buf = nl ? nullptr : new u8[len + 8]; 
  }

  u32 r32(u32 pos) {
    u32 v = *reinterpret_cast<u32 *>(&buf[pos]);
    return v;
  }

  void w32(u32 pos, u32 value) { *reinterpret_cast<u32 *>(&buf[pos]) = value; }

  u16 r16(u32 pos) { return *reinterpret_cast<u16 *>(&buf[pos]); }

  void w16(u32 pos, u16 value) { *reinterpret_cast<u16 *>(&buf[pos]) = value; }

  u8 r8(u32 pos) { return buf[pos]; }

  void w8(u32 pos, u8 value) { buf[pos] = value; }

  u32 offt(u32 vaddr) { return vaddr - start; }

  static u32 read(u32 addr, u8 width, Ram *ctx) {
    if(ctx->noalloc) {
      return 0xffffffff;
    }
    u32 offt = ctx->offt(addr);
    // if(addr>=0x400fef40 and addr<(0x400fef40+4)){
    //   printf("    ------> reading [offt %x] width: %d\n", offt, width);
    //    // assert(false);
    //    // fgetc(stdin);
    // }
    // if(addr>=0x400fef44 and addr<(0x400fef44+4)){
    //   printf("    ------> reading [offt %x] width: %d\n", offt, width);
    //    // assert(false);
    //    // fgetc(stdin);
    // }
    switch (width) {
    case 1:
      return ctx->r8(offt);
    case 2:
      return ctx->r16(offt);
    case 4:
      return ctx->r32(offt);
    }
    assert("bad read" == nullptr);
  }

  static void write(u32 addr, u32 value, u8 width, Ram *ctx) {
    assert(!ctx->noalloc);
    u32 offt = ctx->offt(addr);
    if (addr >= 0x400feea8 and addr < (0x400feea8 + 0xb)) {
      printf("    ------> writing %c [offt %x] width: %d\n", value, offt,
             width);
      // assert(false);
      fgetc(stdin);
    }
    // if(addr>=0x400fef44 and addr<(0x400fef44+4)){
    //   printf("    ------> writing %x [offt %x] width: %d\n", value,offt,
    //   width);
    //    // assert(false);
    //    // fgetc(stdin);
    // }
    switch (width) {
    case 1:
      return ctx->w8(offt, value);
    case 2:
      return ctx->w16(offt, value);
    case 4:
      return ctx->w32(offt, value);
    }
    assert("bad write" == nullptr);
  }

  static void *sysPtr(u32 addr, Ram *ctx) { return ctx->buf + ctx->offt(addr); }

  Region getRegion() {
    Region r;
    r.isram = true;
    r.ctx = this;
    r.len = len;
    r.r = (u32(*)(u32, u8, void *)) & read;
    r.w = (void (*)(u32, u32, u8, void *)) & write;
    r.start = start;
    r.sysPtr = (void *(*)(u32, void *)) & sysPtr;
    return r;
  }

  void loadElf(Elf::Ph *ph, FILE *bin) {
    assert(ph->p_vaddr >= start);
    u32 load_addr = offt(ph->p_vaddr);
    assert((load_addr + ph->p_memsz) <= len);
    FileReader f(bin);
    f.seekTo(ph->p_offset);
    printf("  loading ph at: %x to %x (%u)bytes\n", ph->p_vaddr,
           ph->p_vaddr + ph->p_filesz, ph->p_filesz);
    assert(f.read(buf + load_addr, ph->p_filesz) == ph->p_filesz);
  }

  void loadBin(u32 at, FILE *bin) {
    u32 loadaddr = offt(at);
    FileReader f(bin);
    u64 binlen = f.getFileSize();
    assert((binlen + loadaddr) <= len);
    f.seekTo(0);
    assert(f.read(buf + loadaddr, binlen) == binlen);
  }

  u32 loadArgs(const i8 *argv[], const i8 *envp[]) {
    std::vector<u32> argvptrs = {};
    std::vector<u32> envpptrs = {};
    i8 *load_addr = (i8 *)buf + len;
    u32 load_vaddr = start + len;
    for (u32 i = 0; argv[i]; i++) {
      u32 curlen = strlen(argv[i]);
      load_addr -= curlen + 1;
      load_vaddr -= curlen + 1;
      strncpy(load_addr, argv[i], curlen /*null*/ + 1);
      argvptrs.push_back(load_vaddr);
      printf(" ---> loaded string at %lx\n", (size_t)load_vaddr);
    }
    printf("loading envp...\n");
    for (u32 i = 0; envp[i]; i++) {
      u32 curlen = strlen(envp[i]);
      load_addr -= curlen + 1;
      load_vaddr -= curlen + 1;
      strncpy(load_addr, envp[i], curlen /*null*/ + 1);
      envpptrs.push_back(load_vaddr);
      printf(" ---> loaded string at %lx\n", (size_t)load_vaddr);
    }

    printf("   loaded strings: %s\n", load_addr + 6);

    load_addr = (i8 *)alignB((size_t)load_addr, 4);
    load_vaddr = alignB(load_vaddr, 4);

    u32 argc = argvptrs.size();

    { // 16 random bytes
      load_addr -= 16;
      load_vaddr -= 16;
    }

    u32 random_pointer = load_vaddr;

    { // auxv null term
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = 0;
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = AT_NULL;
      printf("    *****AUX NULL at: %x\n", load_vaddr);
    }

    { // auxv random
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = random_pointer;
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = AT_RANDOM;
      printf("    *****AUX RANDOM at: %x\n", load_vaddr);
    }

    { // envp null term
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = 0;
    }

    for (; envpptrs.size(); envpptrs.pop_back()) {
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = envpptrs.back();
    }
    { // argv null term
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = 0;
    }
    for (; argvptrs.size(); argvptrs.pop_back()) {
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = argvptrs.back();
    }
    {
      load_addr -= 4;
      load_vaddr -= 4;
      *reinterpret_cast<u32 *>(load_addr) = argc;
    }

    printf("  finished loading stuff... sp = %x argc = %d\n", load_vaddr, argc);
    return load_vaddr;
  }
};

struct Memory {
  std::vector<Ram *> rams = {};
  std::vector<Region> regions = {};
  std::bitset<1048576> bitset = {};
  u32 program_end = 0;

  void assertNotUsed(u32 start, u32 len) {
    assert(start % 4096 == 0);
    assert(len % 4096 == 0);
    u32 pages = len / 4096;
    u32 start_page = start / 4096;
    printf("  ..checking if used from %d + %d pages\n", start_page, pages);
    assert(start_page + pages < bitset.size());
    for (u32 i = 0; i < pages; i++) {
      if (bitset[start_page + i]) {
        printf("  [%d] Found set\n", start_page + i);
      }
      assert(!bitset[start_page + i]);
    }
  }

  void assertUsed(u32 start, u32 len) {
    assert(start % 4096 == 0);
    assert(len % 4096 == 0);
    u32 pages = len / 4096;
    u32 start_page = start / 4096;
    printf("  ..checking if used from %d + %d pages\n", start_page, pages);
    assert(start_page + pages < bitset.size());
    for (u32 i = 0; i < pages; i++) {
      assert(bitset[start_page + i]);
    }
  }

  void markUsed(u32 start, u32 len) {
    assertNotUsed(start, len);
    u32 pages = len / 4096;
    u32 start_page = start / 4096;
    printf("  ..marking as used from %d + %d pages\n", start_page, pages);
    assert(start_page + pages < bitset.size());
    for (u32 i = 0; i < pages; i++) {
      bitset[start_page + i] = 1;
    }
  }

  void markFree(u32 start, u32 len) {
    assertUsed(start, len);
    u32 pages = len / 4096;
    u32 start_page = start / 4096;
    printf("  ..marking as used from %d + %d pages\n", start_page, pages);
    assert(start_page + pages < bitset.size());
    for (u32 i = 0; i < pages; i++) {
      bitset[start_page + i] = 0;
    }
  }

  Ram *newRam(u32 start, u32 len) {
    printf("  new region start: 0x%x len: 0x%x\n", start, len);
    u32 end = start + len;
    start = alignB(start, 4096);
    len = alignF(end, 4096) - start;
    printf("  new region start: 0x%x len: 0x%x\n", start, len);
    Ram *r = new Ram(start, len);
    rams.push_back(r);
    Region region = r->getRegion();
    regions.push_back(region);
    markUsed(start, len);
    // printf("NEW REGION ALOCATED start: 0x%x len: 0x%x\n", start, len);
    return r;
  }

  Ram *allocRandom(u32 len) {
    u32 pages = (len / 4096) + 1;
    u32 gap = 0;
    u32 s = 1024;
    for (u32 i = 1024; i < bitset.size() and gap < pages; i++) {
      if (!bitset[i]) {
        gap += 1;
      } else {
        s = i + 1;
        gap = 0;
      }
    }
    if (gap == pages) {
      return newRam(s * 4096, len);
    }
    return nullptr;
  }

  void *sysPtr(u32 addr) {
    for (u32 i = 0; i < regions.size(); i++) {
      Region *region = regions.data() + i;
      if (region->has(addr)) {
        return region->ptr(addr, region->ctx);
      }
    }
    throw InvalidRegion(addr, true);
  }

  void writeRegion(u32 addr, u32 value, u8 width) {
    for (u32 i = 0; i < regions.size(); i++) {
      Region *region = regions.data() + i;
      if (region->has(addr)) {
        return region->write(addr, value, width, region->ctx);
      }
    }
    throw InvalidRegion(addr, false);
  }

  u32 readRegion(u32 addr, u8 width) {
    for (u32 i = 0; i < regions.size(); i++) {
      Region *region = regions.data() + i;
      if (region->has(addr)) {
        return region->read(addr, width, region->ctx);
      }
    }
    throw InvalidRegion(addr, true);
  }

  Region *findRegion(u32 addr, u32 width) {
    for (u32 i = 0; i < regions.size(); i++) {
      Region *region = regions.data() + i;
      if (region->has(addr) and region->has(addr + width)) {
        return region;
      }
    }
    return nullptr;
  }

  void loadElf(Elf::ProgramHeaderIterator it) {
    Elf::Ph ph = {};
    while (it.next(&ph)) {
      if (ph.p_type != PT_LOAD)
        continue;
      if ((ph.p_vaddr + ph.p_memsz) > program_end) {
        program_end = (ph.p_vaddr + ph.p_memsz);
        program_end = alignF(program_end, 4096) + 4096;
      }
      Region *region = findRegion(ph.p_vaddr, ph.p_memsz);
      Ram *ram;
      if (region == nullptr) {
        ram = newRam(ph.p_vaddr, ph.p_memsz);
      } else {
        assert(region->isram);
        Ram *ram = (Ram *)region->ctx;
      }
      ram->loadElf(&ph, it.file);
    }
  }

  void loadBin(u32 at, const i8 *path) {
    FILE *bin = fopen(path, "r");
    FileReader f(bin);
    Region *region = findRegion(at, f.getFileSize());
    assert(region);
    Ram *ram = (Ram *)region->ctx;
    ram->loadBin(at, bin);
    f.close();
  }

  struct MMU {
    bool on = false;

    struct PageFault : std::exception {
      u32 vaddr;
      PageFault(u32 vaddr) : vaddr(vaddr) {}
    };

    union TTBCR {
      struct {
        u32 n : 3 = 0;
        u32 _1 : 29 = 0;
      } b;
      u32 back;
      u8 n() { return b.n; }
    };

    union TTBR0 {
      struct {
        u32 _1 : 32 = 0;
      } b;
      u32 back;
    };

    union TTBR1 {
      struct {
        u32 _1 : 32 = 0;
      } b;
      u32 back;
    };

    union SCTLR {
      struct {
        u32 m : 1 = 0;
        u32 a : 1;
        u32 c : 1;
        u32 _1 : 2 = 0b11;
        u32 cp15ben : 1;
        u32 _9 : 1 = 1;
        u32 b : 1;
        u32 _2 : 2 = 0;
        u32 sw : 1;
        u32 z : 1;
        u32 i : 1;
        u32 v : 1 = 0;
        u32 rr : 1;
        u32 _3 : 1 = 0;
        u32 _4 : 1 = 1;
        u32 ha : 1;
        u32 _5 : 1 = 1;
        u32 wxn : 1;
        u32 uwxn : 1;
        u32 fi : 1;
        u32 u : 1 = 1;
        u32 _6 : 1 = 1;
        u32 ve : 1;
        u32 ee : 1;
        u32 _7 : 1 = 0;
        u32 nmfi : 1;
        u32 tre : 1;
        u32 afe : 1;
        u32 te : 1;
        u32 _8 : 1 = 0;
      } b;
      u32 back;
    };

    SCTLR sctrl = {};
    TTBCR ttbcr = {};
    TTBR0 ttbr0 = {};
    TTBR1 ttbr1 = {};

    struct L1 {

      enum class Type { invalid, page_table, section, super_section, _ };

      u32 back;

      Type t() {
        if ((back & 0b11) == 0) {
          return Type::invalid;
        }

        if ((back & 0b11) == 1) {
          return Type::page_table;
        }

        if ((back & 0b11) == 0b10 and !((back >> 18) & 1)) {
          return Type::section;
        }

        if ((back & 0b11) == 0b10 and ((back >> 18) & 1)) {
          return Type::super_section;
        }
        return Type::_;
      }

      u32 base(u32 vaddr) {
        switch (t()) {
        case Type::page_table:
          return back & 0xfffffc00;
        case Type::section:
          return back & 0xfff00000;
        case Type::super_section:
          return back & 0xff000000;
        default:
          throw PageFault(vaddr);
        }
      }
    };

    struct L2 {
      enum class Type {
        invalid,
        large_page,
        small_page,
      };
      u32 back;

      Type t() {
        if ((back & 0b11) == 0)
          return Type::invalid;
        if ((back & 0b11) == 1)
          return Type::large_page;
        return Type::small_page;
      }

      u32 base(u32 vaddr) {
        switch (t()) {
        case Type::large_page:
          return back & 0xffff0000;
        case Type::small_page:
          return back & 0xfffff000;
        default:
          throw PageFault(vaddr);
        }
      }
    };

    union VA {
      u32 back;
      struct {
        u32 offt : 12;
        u32 l2 : 8;
        u32 l1 : 12;
      } b;

      u32 l1() { return b.l1; }

      u32 l2() { return b.l2; }

      u32 offt() { return b.offt; }

      u32 lOfft() { return back & 0xffff; }

      u32 secOfft() { return back & 0xfffff; }

      u32 supOfft() { return back & 0xffffff; }
    };

    u8 x() { return 14 - ttbcr.n(); }

    u32 ttbr0Addr() {
      u8 n = x();
      return (ttbr0.back >> n) << n;
    }

    void setsctrl(u32 value) {
      sctrl.back = value;
      on = sctrl.b.m;
    }

    u32 ttbr1Addr() { return (ttbr1.back >> 14) << 14; }

    inline u32 fstTTBR1Addr() {
      assert(ttbcr.n() != 0);
      return (0x80000000) >> (ttbcr.n() - 1);
    }

    u32 walk(u32 vaddr, Memory *mem) {
      if (!on)
        return vaddr;
      VA va = VA{.back = vaddr};
      if (ttbcr.n() > 0 and vaddr >= fstTTBR1Addr()) {
        printf("TODO ttbr1\n");
        assert(false);
      } else {
        u32 ttbr0_addr = ttbr0Addr();
        // printf("==================: %d %d\n", ttbr0_addr, ttbr0.back);
        L1 l1 = {
            .back = mem->a32a(ttbr0_addr + 4 * va.l1()),
        };
        // printf("li.base %x type %d back %d l1: %d l2: %d\n", l1.base(vaddr),
        // (u32)l1.t(), l1.back, va.l1(), va.l2());
        switch (l1.t()) {
        case L1::Type::page_table: {
          // assert(false);
          L2 l2 = {
              .back = mem->a32a(l1.base(vaddr) + 4 * va.l2()),
          };
          // printf("l1 base = %x l2 base = %x l2 back = %x\n",l1.base(vaddr),
          // l2.base(vaddr), l2.back); assert(false);
          switch (l2.t()) {
          case L2::Type::small_page:
            return l2.base(vaddr) + va.offt();
          case L2::Type::large_page:
            return l2.base(vaddr) + va.lOfft();
          default:
            throw PageFault(vaddr);
          }
        }
        case L1::Type::section:
          return l1.base(vaddr) + va.secOfft();
        case L1::Type::super_section:
          return l1.base(vaddr) + va.supOfft();
        default:
          throw PageFault(vaddr);
        }
      }
    }
  };

  MMU mmu;
  u32 a32u(u64 pos) {
    pos = mmu.walk(pos, this);
    return readRegion(pos, 4);
  }

  void a32u(u64 pos, u32 value) {
    pos = mmu.walk(pos, this);
    return writeRegion(pos, value, 4);
  }

  u16 a16u(u64 pos) {
    pos = mmu.walk(pos, this);
    return readRegion(pos, 2);
  }

  void a16u(u64 pos, u16 value) {
    pos = mmu.walk(pos, this);
    return writeRegion(pos, value, 2);
  }

  u8 a8u(u64 pos) {
    pos = mmu.walk(pos, this);
    return readRegion(pos, 1);
  }

  void a8u(u64 pos, u8 value) {
    pos = mmu.walk(pos, this);
    return writeRegion(pos, value, 1);
  }

  u32 a32a(u64 pos) {
    pos = mmu.walk(pos, this);
    return readRegion(pos, 4);
  }

  void a32a(u64 pos, u32 value) {
    // TODO
    pos = mmu.walk(pos, this);
    return writeRegion(pos, value, 4);
  }

  u16 a16a(u64 pos) {
    // TODO
    pos = mmu.walk(pos, this);
    return readRegion(pos, 2);
  }

  void a16a(u64 pos, u16 value) {
    // TODO
    pos = mmu.walk(pos, this);
    return writeRegion(pos, value, 2);
  }

  u8 a8a(u64 pos) {
    // TODO
    pos = mmu.walk(pos, this);
    return readRegion(pos, 1);
  }

  void a8a(u64 pos, u8 value) {
    // TODO
    pos = mmu.walk(pos, this);
    return writeRegion(pos, value, 1);
  }

  void loadBinary(const char *path, u32 pos) {
    // u32 real_pos = ;
    // printf("binary loaded at %x (%x) rambase = %x\n", real_pos, pos,
    // rambase); FILE *binary = fopen(path, "r"); if (binary == nullptr) {
    //   printf("failed to open binary: %s\n", path);
    //   exit(1);
    // }
    // FileReader fr(binary);
    // u64 file_size = fr.getFileSize();
    // // printf("file size: %ld\n", file_size);
    // u32 written = fr.read(buf + real_pos, file_size);
    // assert(written == file_size);
  }
  // template <typename T> T readLE(u64 pos) {
  //   T tmp;
  //   pos = pos % size;
  //   const u64 r = size - pos;
  //   memcpy(&tmp, buf + pos, sizeof(T) < r ? sizeof(T) : r);
  //   u8 *b = (u8 *)&tmp;
  //   int half = sizeof(T) / 2;
  //   for (int j = 0; j < half; j++) {
  //     char c = b[j];
  //     b[j] = b[sizeof(T) - j - 1];
  //     b[sizeof(T) - j - 1] = c;
  //   }

  //   return tmp;
  // }

  // template <typename T> void writeLE(u64 pos, T value) {
  //   pos %= size;

  //   u8 *b = (u8 *)&value;
  //   int half = sizeof(T) / 2;
  //   for (int j = 0; j < half; j++) {
  //     char c = b[j];
  //     b[j] = b[sizeof(T) - j - 1];
  //     b[sizeof(T) - j - 1] = c;
  //   }

  //   const u64 r = size - pos;
  //   memcpy(buf + pos, &value, sizeof(T) < r ? sizeof(T) : r);
  // }

  static void test() {
    // Memory<64> m = {};
    // m.writeBE<u8>(0, 0xff);
    // assert(m.readBE<u8>(0) == 0xff);
    // m.writeBE<u16>(0, 0xff01);
    // assert(m.readBE<u16>(0) == 0xff01);
    // m.writeBE<u32>(0, 0xff0000ff);
    // assert(m.readBE<u32>(0) == 0xff0000ff);
    // m.writeBE<u64>(0, 0xff00000000000000);
    // assert(m.readBE<u64>(0) == 0xff00000000000000);

    // m.writeLE<u8>(0, 0xff);
    // assert(m.readLE<u8>(0) == 0xff);
    // m.writeLE<u16>(0, 0xff01);
    // assert(m.readLE<u16>(0) == 0xff01);
    // m.writeLE<u32>(0, 0xff0000ff);
    // assert(m.readLE<u32>(0) == 0xff0000ff);
    // m.writeLE<u64>(0, 0xff00000000000000);
    // assert(m.readLE<u64>(0) == 0xff00000000000000);

    // printf("Memory::test() passed...\n");
  }
};
