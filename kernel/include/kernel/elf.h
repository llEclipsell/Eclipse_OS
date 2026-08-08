#ifndef _KERNEL_ELF_H
#define _KERNEL_ELF_H

#include <stdint.h>

typedef struct {
	uint8_t  e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

typedef struct {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} __attribute__((packed)) Elf32_Phdr;

#define PT_LOAD 1

uint32_t elf_load(uint8_t* data, uint32_t size, uint32_t* out_break);

#define PT_DYNAMIC 2

#define ET_EXEC 2
#define ET_DYN  3

#define DT_NULL   0
#define DT_REL    17
#define DT_RELSZ  18
#define DT_RELENT 19

#define R_386_RELATIVE 8
#define R_386_32       1
#define R_386_PC32     2
#define R_386_RELATIVE 8

#define ELF32_R_TYPE(i) ((i) & 0xFF)
#define ELF32_R_SYM(i)  ((i) >> 8)

typedef struct {
	int32_t  d_tag;
	uint32_t d_val;
} __attribute__((packed)) Elf32_Dyn;

typedef struct {
	uint32_t r_offset;
	uint32_t r_info;
} __attribute__((packed)) Elf32_Rel;

#endif
