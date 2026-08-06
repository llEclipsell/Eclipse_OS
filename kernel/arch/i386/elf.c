#include <stdio.h>
#include <string.h>
#include <kernel/elf.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>

uint32_t elf_load(uint8_t* data, uint32_t size) {
	Elf32_Ehdr* eh = (Elf32_Ehdr*) data;

	if (size < sizeof(Elf32_Ehdr))            { printf("elf: too small\n");      return 0; }
	if (memcmp(eh->e_ident, "\x7F" "ELF", 4)) { printf("elf: bad magic\n");      return 0; }
	if (eh->e_ident[4] != 1)                  { printf("elf: not 32-bit\n");     return 0; }
	if (eh->e_machine != 3)                   { printf("elf: not i386\n");       return 0; }
	if (eh->e_type != 2)                      { printf("elf: not executable\n"); return 0; }

	Elf32_Phdr* ph = (Elf32_Phdr*) (data + eh->e_phoff);

	for (int i = 0; i < eh->e_phnum; i++) {
		if (ph[i].p_type != PT_LOAD)
			continue;

		uint32_t start = ph[i].p_vaddr & ~0xFFF;
		uint32_t end   = (ph[i].p_vaddr + ph[i].p_memsz + 0xFFF) & ~0xFFF;

		for (uint32_t v = start; v < end; v += 0x1000) {
			uint32_t frame = pmm_alloc_frame();
			if (!frame) { printf("elf: out of memory\n"); return 0; }

			paging_map(v, frame, PAGE_USER | PAGE_WRITE);
			memset((void*) v, 0, 0x1000);      /* zero AFTER mapping */
		}

		memcpy((void*) ph[i].p_vaddr,
		       data + ph[i].p_offset,
		       ph[i].p_filesz);
	}

	return eh->e_entry;
}
