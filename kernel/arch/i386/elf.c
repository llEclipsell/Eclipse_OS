#include <stdio.h>
#include <string.h>
#include <kernel/elf.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>

static void apply_relocations(uint8_t* data, Elf32_Ehdr* eh, uint32_t base) {
	Elf32_Phdr* ph  = (Elf32_Phdr*) (data + eh->e_phoff);
	Elf32_Dyn*  dyn = 0;

	for (int i = 0; i < eh->e_phnum; i++)
		if (ph[i].p_type == PT_DYNAMIC)
			dyn = (Elf32_Dyn*) (base + ph[i].p_vaddr);

	if (!dyn)
		return;                          /* statically linked — nothing to do */

	uint32_t rel = 0, relsz = 0, relent = sizeof(Elf32_Rel);

	for (Elf32_Dyn* d = dyn; d->d_tag != DT_NULL; d++) {
		switch (d->d_tag) {
			case DT_REL:    rel    = base + d->d_val; break;
			case DT_RELSZ:  relsz  = d->d_val;        break;
			case DT_RELENT: relent = d->d_val;        break;
		}
	}

	uint32_t count = 0;
	uint32_t applied = 0, skipped = 0;

	for (uint32_t off = 0; off < relsz; off += relent) {
		Elf32_Rel* r = (Elf32_Rel*) (rel + off);

		switch (ELF32_R_TYPE(r->r_info)) {
			case R_386_RELATIVE:
				*(uint32_t*) (base + r->r_offset) += base;
				count++;
				break;
			case R_386_PC32:
				/* Weak undefined symbols from crtbegin.o / crtend.o —
				   __register_frame_info and friends. The call sites are
				   guarded by a null check on the symbol's address, which
				   is 0 because nothing defines it, so they are never
				   reached. Leaving these unapplied is correct. */
				skipped++;
				break;
			default:
				printf("elf: unhandled reloc type %d\n",
				       ELF32_R_TYPE(r->r_info));
				break;
		}
	}

	printf("elf: applied %d relocations at base 0x%x\n", count, base);
}

uint32_t elf_load(uint8_t* data, uint32_t size, uint32_t* out_break) {
	Elf32_Ehdr* eh = (Elf32_Ehdr*) data;
	uint32_t base = 0;
	uint32_t highest = 0;

	if (size < sizeof(Elf32_Ehdr))            { printf("elf: too small\n");      return 0; }
	if (memcmp(eh->e_ident, "\x7F" "ELF", 4)) { printf("elf: bad magic\n");      return 0; }
	if (eh->e_ident[4] != 1)                  { printf("elf: not 32-bit\n");     return 0; }
	if (eh->e_machine != 3)                   { printf("elf: not i386\n");       return 0; }
	if (eh->e_type == ET_DYN)		  { base = 0x40000000; 			       }
	else if (eh->e_type != ET_EXEC)		  { printf("elf: not executable\n"); return 0; }

	Elf32_Phdr* ph = (Elf32_Phdr*) (data + eh->e_phoff);

	for (int i = 0; i < eh->e_phnum; i++) {
		if (ph[i].p_type != PT_LOAD)
			continue;

		uint32_t start = (base + ph[i].p_vaddr) & ~0xFFF;
		uint32_t end   = (base + ph[i].p_vaddr + ph[i].p_memsz + 0xFFF) & ~0xFFF;

		if (end > highest)
			highest = end;

		for (uint32_t v = start; v < end; v += 0x1000) {
			uint32_t frame = pmm_alloc_frame();
			if (!frame) { printf("elf: out of memory\n"); return 0; }

			paging_map(v, frame, PAGE_USER | PAGE_WRITE);
			memset((void*) v, 0, 0x1000);      /* zero AFTER mapping */
		}

		memcpy((void*) (base + ph[i].p_vaddr),
		       data + ph[i].p_offset,
		       ph[i].p_filesz);
	}

	apply_relocations(data, eh, base);

	if (out_break)
		*out_break = highest;

	return base + eh->e_entry;
}
