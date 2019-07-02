/* SPIM S20 MIPS simulator.
   Code to read a MIPS a.out file.
   Copyright (C) 1990-1994 by James Larus (larus@cs.wisc.edu).
   ALL RIGHTS RESERVED.

   SPIM is distributed under the following conditions:

     You may make copies of SPIM for your own use and modify those copies.

     All copies of SPIM must retain my name and copyright notice.

     You may not sell SPIM or distributed SPIM in conjunction with a
     commerical product or service without the expressed written consent of
     James Larus.

   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE. */


/* $Header: /var/home/larus/Software/larus/SPIM/RCS/read-aout.c,v 3.18 1994/02/01 20:41:25 larus Exp $
*/


#include <sst_config.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <libelf/gelf.h>
#include "mips_4kc.h"

using namespace SST;
using namespace SST::MIPS4KCComponent;

/*#include "spim.h"
#include "spim-utils.h"
#include "inst.h"
#include "mem.h"
#include "data.h"
#include "read-aout.h"
#include "sym-tbl.h"*/


/* Exported Variables: */

/* Imported Variables: */



/* Read a MIPS executable file.  Return zero if successful and
   non-zero otherwise. */

int MIPS4KC::read_aout_file (const char *file_name)
{
    // load instructions, pre-decoded 
    
    if (elf_version(EV_CURRENT) == EV_NONE) {
        out.fatal(CALL_INFO,-1, "ELF library initialization failed: %s\n", 
                  elf_errmsg(-1));
    }

    int fd;
    if ((fd = open("test/a.out", O_RDONLY, 0)) < 0) 
        out.fatal(CALL_INFO,-1, "ELF file open failed\n");

    Elf *e;
    if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL) 
        out.fatal(CALL_INFO,-1,"elf_begin() failed: %s.",elf_errmsg(-1));

    if (elf_kind(e) != ELF_K_ELF)
        out.fatal(CALL_INFO,-1, "file is not an ELF object.");

    Elf32_Ehdr *ehdr;
    if ((ehdr = elf32_getehdr(e)) == NULL) {
        out.fatal(CALL_INFO,-1, "getehdr() failed: %s.",elf_errmsg(-1));
        if (ehdr->e_machine != EM_MIPS) {
            out.fatal(CALL_INFO,-1, "Executable is not MIPS\n");
        }
        if (ehdr->e_type != ET_EXEC) {
            out.fatal(CALL_INFO,-1, "File is not Executable\n");
        }
        if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
            out.fatal(CALL_INFO,-1, "Exdecutable is not 32-bit\n");
        }
        if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
            out.fatal(CALL_INFO,-1, "Exdecutable is LSB Endianness\n");
        }
    }

    size_t shstrndx;
    if (elf_getshdrstrndx(e, &shstrndx) != 0) 
        out.fatal(CALL_INFO,-1, "elf getshdrstrndx() failed:␣%s.", elf_errmsg(-1));

    Elf_Scn *scn=NULL;
    GElf_Shdr shdr;
    while ((scn = elf_nextscn(e, scn)) != NULL) {
        if (gelf_getshdr(scn, &shdr) != &shdr) 
            out.fatal(CALL_INFO,-1 , "getshdr() failed:␣%s.", elf_errmsg(-1));

        char *name;
        if ((name = elf_strptr(e, shstrndx , shdr.sh_name)) == NULL)
            out.fatal(CALL_INFO,-1 , "elf_strptr() failed:␣%s.", 
                      elf_errmsg(-1));
        
        printf("Section %4lu %s Size=%lu Flags= %s%s%s Addr=%lx\n", 
               elf_ndxscn(scn), name, shdr.sh_size,
               (shdr.sh_flags & SHF_WRITE) ? "Write " : "",
               (shdr.sh_flags & SHF_ALLOC) ? "Alloc " : "",
               (shdr.sh_flags & SHF_EXECINSTR) ? "Exec " : "",
               shdr.sh_addr);

                    
        // if .init section, set the program_starting_address
        if (strncmp(name,".init",5) == 0) {
            printf("Detected INIT section %lx\n", shdr.sh_addr);
            program_starting_address = shdr.sh_addr;
        }
        
        // load executable sections
        if (shdr.sh_flags & SHF_EXECINSTR) {
            mem_addr addr = shdr.sh_addr;
            Elf_Data *data = NULL;
            while((data = elf_getdata(scn, data)) != NULL) {
                size_t n=0;
                printf(" Data: align %lu, off %llu, size %lu\n", 
                       data->d_align, data->d_off, data->d_size);
                while (n < shdr.sh_size) {
                    //pointer to instruction
                    uint32_t* wp = (uint32_t*) data->d_buf + (n>>2);
                    // make instruction
                    instruction *add_inst = inst_decode(*wp); 
                    store_instruction(add_inst, addr); // store
                    addr += 4; // advance
                    n += 4;
                }
            }
        }
        // SET R[REG_GP] = ;
    }

    elf_end(e);
    close(fd);

#if 0
    //text_begins_at_point(0x1000); //(header.text_start);
    //store_instruction(inst_decode (getw (fd)));
    instruction *add_inst = inst_decode(0x0221820); // ADD r3, r1, r2
    store_instruction(add_inst);

     add_inst = inst_decode(0x0221820); // ADD r3, r1, r2
    store_instruction(add_inst);

     add_inst = inst_decode(0x0221820); // ADD r3, r1, r2
    store_instruction(add_inst);

     add_inst = inst_decode(0x0221820); // ADD r3, r1, r2
    store_instruction(add_inst);

     add_inst = inst_decode(0x0221820); // ADD r3, r1, r2
    store_instruction(add_inst);

    add_inst = inst_decode(0x0221820); // ADD r3, r1, r2
    store_instruction(add_inst);

    printf("Reading...\n");
    print_inst(TEXT_BOT);
    program_starting_address = TEXT_BOT; //header.entry;
#endif

    // TODO load data
    //data_begins_at_point (header.data_start);
    //program_break = header.bss_start + header.bsize;
    
    
    return (0);
}
