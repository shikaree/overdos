/* -------------------------------------------------------------------------- */
/* - ELF Loader for ÖverDOS Emulator                                        - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"
#include "mmu.h"
#include "isa.h"

extern int emu_verbose;                 /* set from argv in main.c */

static void debug_Elf32_Ehdr (Elf32_Ehdr Ehdr);
static void debug_Elf32_Phdr (Elf32_Phdr Phdr);

int load_elf(MMU *mmu, const char *filename)
{
 FILE *file;
 Elf32_Ehdr Ehdr;
 Elf32_Phdr Phdr;
 MMU_NODE *node;
 long pos;
 int i;

 mmu->next = NULL;
 mmu->end = NULL;

 /* NOTE: must be binary ("rb") - on Windows text mode mangles the image. */
 file = fopen(filename,"rb");
 if (!file) {
  printf("ELF: Error. Unable to open '%s'.\n", filename);
  return 1;
 }

 /* Read the ELF header. */
 if (fread((void *) &Ehdr, sizeof(Ehdr), 1, file) != 1) {
  printf("ELF: Error. '%s' is too short to be an ELF file.\n", filename);
  fclose(file);
  return 1;
 }
 if (emu_verbose) {
  debug_Elf32_Ehdr(Ehdr);
 }

 /* Check ELF header. */
 if (isa_reorder_short(Ehdr.e_machine) != EM_MIPS) {
  printf("ELF: Error. Not MIPS R3000 big-endian format.\n");
  fclose(file);
  return 1;
 }

 mmu->entry = isa_reorder(Ehdr.e_entry);

 /* ----- Read all Phdrs (program headers). ----- */
 fseek(file, isa_reorder(Ehdr.e_phoff), SEEK_SET);
 for (i=0; i<isa_reorder_short(Ehdr.e_phnum); i++) {
  if (fread((void *) &Phdr, isa_reorder_short(Ehdr.e_phentsize), 1, file) != 1) {
   printf("ELF: Error. Truncated program header table.\n");
   fclose(file);
   return 1;
  }

  if (emu_verbose) {
   debug_Elf32_Phdr(Phdr);
  }

  if (Phdr.p_memsz) {
   size_t got;
   pos = ftell(file);

   node = mmu_node_new(mmu);
   if (!node) {
    printf("ELF: Error. Out of memory when allocating memory for MMU.\n");
    fclose(file);
    return 1;
   }

   /* Allocate the segment's memory. */
   node->data = (char *)malloc (isa_reorder(Phdr.p_memsz));
   if (!node->data) {
    printf("ELF: Error. Unable to allocate %i bytes at 0x%08x.\n",
           isa_reorder(Phdr.p_memsz), isa_reorder(Phdr.p_vaddr));
    fclose(file);
    return 1;
   }

   /* Zero the whole segment first so that .bss (memsz > filesz) is properly
      zero-initialised; stale garbage in kernel globals causes random crashes.
      Then read the on-disk part (filesz) over the top. */
   memset(node->data, 0, isa_reorder(Phdr.p_memsz));
   fseek(file, isa_reorder(Phdr.p_offset), SEEK_SET);
   got = fread(node->data, 1, isa_reorder(Phdr.p_filesz), file);
   (void) got;
   fseek(file, pos, SEEK_SET);

   node->addr =  isa_reorder(Phdr.p_vaddr);
   node->size =  isa_reorder(Phdr.p_memsz);
   node->callback = NULL;
  }
 }

 fclose(file);
 return 0;
}


static void debug_Elf32_Ehdr (Elf32_Ehdr Ehdr)
{
 printf("         e_ident: %s\n", Ehdr.e_ident);
 printf("          e_type:     0x%04x\n", isa_reorder_short(Ehdr.e_type));
 printf("       e_machine:     0x%04x\n", isa_reorder_short(Ehdr.e_machine));
 printf("         e_entry: 0x%08x\n", isa_reorder(Ehdr.e_entry));
 printf("         e_phoff: 0x%08x\n", isa_reorder(Ehdr.e_phoff));
 printf("         e_shoff: 0x%08x\n", isa_reorder(Ehdr.e_shoff));
 printf("         e_flags: 0x%08x\n", isa_reorder(Ehdr.e_flags));
 printf("        e_ehsize:     0x%04x\n", isa_reorder_short(Ehdr.e_ehsize));
 printf("     e_phentsize:     0x%04x\n", isa_reorder_short(Ehdr.e_phentsize));
 printf("         e_phnum:     0x%04x\n", isa_reorder_short(Ehdr.e_phnum));
 printf("     e_shentsize:     0x%04x\n", isa_reorder_short(Ehdr.e_shentsize));
 printf("         e_shnum:     0x%04x\n", isa_reorder_short(Ehdr.e_shnum));
 printf("      e_shstrndx:     0x%04x\n\n", isa_reorder_short(Ehdr.e_shstrndx));
}


static void debug_Elf32_Phdr (Elf32_Phdr Phdr)
{
 printf("          p_type: 0x%08x\n", isa_reorder(Phdr.p_type));
 printf("        p_offset: 0x%08x\n", isa_reorder(Phdr.p_offset));
 printf("         p_vaddr: 0x%08x\n", isa_reorder(Phdr.p_vaddr));
 printf("         p_paddr: 0x%08x\n", isa_reorder(Phdr.p_paddr));
 printf("        p_filesz: 0x%08x\n", isa_reorder(Phdr.p_filesz));
 printf("         p_memsz: 0x%08x\n", isa_reorder(Phdr.p_memsz));
 printf("         p_flags: 0x%08x\n", isa_reorder(Phdr.p_flags));
 printf("         p_align: 0x%08x\n\n", isa_reorder(Phdr.p_align));
}
