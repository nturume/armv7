#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <elf.h>
#include <cstring>

struct Elf {
  FILE *file;
  Elf32_Ehdr header;
  using Ph = Elf32_Phdr;

  struct ProgramHeaderIterator {
    FILE *file;
    Elf32_Ehdr header;
    std::size_t index;
    ProgramHeaderIterator(FILE*f,Elf32_Ehdr h) :file(f), header(h), index(0) {}
    Elf32_Phdr *next(Elf32_Phdr *ptr) {
      if(ptr == nullptr || file == nullptr) return nullptr;
      if(index >= header.e_phnum) {
        return nullptr;
      }
      const size_t offset =(size_t) header.e_phoff + (sizeof(Ph)*index);
      if(fseek(file, offset, SEEK_SET)){
        printf("failed to seek..\n");
        return nullptr;
      }
      const size_t n = fread(ptr, 1, sizeof(Ph), file);
      if(n!=sizeof(Elf32_Phdr)) {
        printf("failed to read program header");
        exit(1);
      }
      index += 1;
      return ptr;
    }
  };
  
  Elf(const char *filename) {
    FILE *f = std::fopen(filename, "r");

    if(f == nullptr) {
      std::printf("failed to open elf binary: %s\n", filename);
      std::exit(1);
    }
    file = f;
    size_t n = std::fread(&header, 1, sizeof(header), f);
    if(n != sizeof(header)) {
      printf("failed to read from elf binary: %s\n", filename);
      std::exit(1);
    }
    
    if(std::strncmp(((char *)header.e_ident),(const char[4]){0x7f,'E','L','F'},4)) {
      std::printf("invalid file: %s\n", filename);
      std::exit(1);
    }

    if(header.e_ident[EI_VERSION] != 1) {
      std::printf("invalid elf version: %s\n", filename);
      std::exit(1);
    }

    if(header.e_ident[EI_CLASS] != ELFCLASS32) {
      std::printf("not a 32 bit elf binary: %s\n", filename);
      std::exit(1);
    }

    if(header.e_ident[EI_DATA] != ELFDATA2LSB) {
      std::printf("not a little endian binary: %s\n", filename);
      std::exit(1);
    }

    if(header.e_machine != 40) {
      std::printf("not a arm binary: %s\n", filename);
      std::exit(1);
    }
  }

  ProgramHeaderIterator getPHIter() {
    return ProgramHeaderIterator(file, header);
  };

  ~Elf() {
    fclose(file);
  }
};
