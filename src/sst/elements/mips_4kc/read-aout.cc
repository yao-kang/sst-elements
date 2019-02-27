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
#include "mips_4kc.h"

using namespace SST;
using namespace SST::MIPS4KCComponent;

/*#include "spim.h"
#include "spim-utils.h"
#include "inst.h"
#include "mem.h"
#include "data.h"
#include "parser.h"
#include "read-aout.h"
#include "sym-tbl.h"*/


/* Exported Variables: */

/* Imported Variables: */



/* Read a MIPS executable file.  Return zero if successful and
   non-zero otherwise. */

int MIPS4KC::read_aout_file (const char *file_name)
{
    // load instructions, pre-decoded 
    text_dir = 1;
    //text_begins_at_point(0x1000); //(header.text_start);
    //store_instruction(inst_decode (getw (fd)));
    instruction *add_inst = inst_decode(0x0118820); // ADD r1, r2, r3
    store_instruction(add_inst);
    printf("Reading...\n");
    print_inst(TEXT_BOT);
    text_dir = 0;
    program_starting_address = TEXT_BOT; //header.entry;

    // load data
    //data_begins_at_point (header.data_start);
    //program_break = header.bss_start + header.bsize;
    
    
    return (0);
}
