/* SPIM S20 MIPS simulator.
   Code to manipulate data segment directives.
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


/* $Header: /var/home/larus/Software/larus/SPIM/RCS/data.c,v 3.12 1994/01/18 03:21:45 larus Exp $
*/

#include <sst_config.h>
#include "mips_4kc.h"



using namespace SST;
using namespace SST::MIPS4KCComponent;


/* The first 64K of the data segment are dedicated to small data
   segment, which is pointed to by $gp. This register points to the
   middle of the segment, so we can use the full offset field in an
   instruction. */

#define DATA_PC (in_kernel ? next_k_data_pc : next_data_pc)

#define BUMP_DATA_PC(DELTA) {if (in_kernel) \
				next_k_data_pc += DELTA; \
				else {next_data_pc += DELTA; \
				      program_break = next_data_pc;}}

static int next_gp_offset;	/* Offset off $gp of next data item */

static int auto_alignment = 1;	/* Non-zero => align literal to natural bound*/



/* If TO_KERNEL is non-zero, subsequent data will be placed in the
   kernel data segment.  If it is zero, data will go to the user's data
   segment.*/

void MIPS4KC::user_kernel_data_segment (int to_kernel)
{
    in_kernel = to_kernel;
}


void MIPS4KC::end_of_assembly_file (void)
{
  in_kernel = 0;
  auto_alignment = 1;
}


/* Set the point at which the first datum is stored to be ADDRESS +
   64K.	 The 64K increment allocates an area pointed to by register
   $gp, which is initialized. */

void MIPS4KC::data_begins_at_point (mem_addr addr)
{
  if (bare_machine)
    next_data_pc = addr;
  else
    {
      next_gp_offset = addr;
      gp_midpoint = addr + 32*K;
      R[REG_GP] = gp_midpoint;
      next_data_pc = addr + 64 * K;
    }
}


/* Set the point at which the first datum is stored in the kernel's
   data segment. */

void MIPS4KC::k_data_begins_at_point (mem_addr addr)
{
    next_k_data_pc = addr;
}


/* Arrange that the next datum is stored on a memory boundary with its
   low ALIGNMENT bits equal to 0.  If argument is 0, disable automatic
   alignment.*/

void MIPS4KC::align_data (int alignment)
{
  if (alignment == 0)
    auto_alignment = 0;
  else if (in_kernel)
    {
      next_k_data_pc =
	(next_k_data_pc + (1 << alignment) - 1) & (-1 << alignment);
      fatal_error ("Kernel not supported\n");
      //fix_current_label_address (next_k_data_pc);
    }
  else
    {
      next_data_pc = (next_data_pc + (1 << alignment) - 1) & (-1 << alignment);
      fatal_error ("Kernel not supported\n");
      //fix_current_label_address (next_data_pc);
    }
}


void MIPS4KC::set_data_alignment (int alignment)
{
  if (auto_alignment)
    align_data (alignment);
}


void MIPS4KC::enable_data_alignment (void)
{
  auto_alignment = 1;
}


/* Set the location (in user or kernel data space) for the next datum. */

void MIPS4KC::set_data_pc (mem_addr addr)
{
  if (in_kernel)
    next_k_data_pc = addr;
  else
    next_data_pc = addr;
}


/* Return the address at which the next datum will be stored.  */

mem_addr MIPS4KC::current_data_pc (void)
{
  return (DATA_PC);
}


/* Bump the address at which the next data will be stored by VALUE
   bytes. */

void MIPS4KC::increment_data_pc (int value)
{
  BUMP_DATA_PC (value);
}


/* Process a .extern NAME SIZE directive. */

#if 0
void MIPS4KC::extern_directive (char *name, int size)
{
  label *sym = make_label_global (name);

  if (!bare_machine
      && size > 0 && size <= SMALL_DATA_SEG_MAX_SIZE
      && next_gp_offset + size < gp_midpoint + 32*K)
    {
      sym->gp_flag = 1;
      sym->addr = next_gp_offset;
      next_gp_offset += size;
    }
}


/* Process a .lcomm NAME SIZE directive. */

#ifdef __STDC__
void
lcomm_directive (char *name, int size)
#else
void
lcomm_directive (name, size)
     char *name;
     int size;
#endif
{
  label *sym = lookup_label (name);

  if (!bare_machine
      && size > 0 && size <= SMALL_DATA_SEG_MAX_SIZE
      && next_gp_offset + size < gp_midpoint + 32*K)
    {
      sym->gp_flag = 1;
      sym->addr = next_gp_offset;
      next_gp_offset += size;
    }
  /* Don't need to initialize since memory starts with 0's */
}
#endif


/* Process a .ascii STRING or .asciiz STRING directive. */

void MIPS4KC::store_string (char *string, int length, int null_terminate)
{
  for ( ; length > 0; string ++, length --) {
    SET_MEM_BYTE (DATA_PC, *string);
    BUMP_DATA_PC(1);
  }
  if (null_terminate)
    {
      SET_MEM_BYTE (DATA_PC, 0);
      BUMP_DATA_PC(1);
    }
}


/* Process a .byte EXPR directive. */

void MIPS4KC::store_byte (int value)
{
  SET_MEM_BYTE (DATA_PC, value);
  BUMP_DATA_PC (1);
}


/* Process a .half EXPR directive. */

void MIPS4KC::store_half (int value)
{
  if (DATA_PC & 0x1)
    {
#ifdef BIGENDIAN
      store_byte ((value >> 8) & 0xff);
      store_byte (value & 0xff);
#else
      store_byte (value & 0xff);
      store_byte ((value >> 8) & 0xff);
#endif
    }
  else
    {
      SET_MEM_HALF (DATA_PC, value);
      BUMP_DATA_PC (BYTES_PER_WORD / 2);
    }
}


/* Process a .word EXPR directive. */

void MIPS4KC::store_word (int value)
{
  if (DATA_PC & 0x3)
    {
#ifdef BIGENDIAN
      store_half ((value >> 16) & 0xffff);
      store_half (value & 0xffff);
#else
      store_half (value & 0xffff);
      store_half ((value >> 16) & 0xffff);
#endif
    }
  else
    {
      SET_MEM_WORD (DATA_PC, value);
      BUMP_DATA_PC (BYTES_PER_WORD);
    }
}


/* Process a .double EXPR directive. */

void MIPS4KC::store_double (double *value)
{
  if (DATA_PC & 0x7)
    {
      store_word (* ((long *) value));
      store_word (* (((long *) value) + 1));
    }
  else
    {
      SET_MEM_WORD (DATA_PC, * ((long *) value));
      BUMP_DATA_PC (BYTES_PER_WORD);
      SET_MEM_WORD (DATA_PC, * (((long *) value) + 1));
      BUMP_DATA_PC (BYTES_PER_WORD);
    }
}


/* Process a .float EXPR directive. */

void MIPS4KC::store_float (double *value)
{
  float val = *value;
  float *vp = &val;

  if (DATA_PC & 0x3)
    {
      store_half (*(long *) vp & 0xffff);
      store_half ((*(long *) vp >> 16) & 0xffff);
    }
  else
    {
      SET_MEM_WORD (DATA_PC, *((long *) vp));
      BUMP_DATA_PC (BYTES_PER_WORD);
    }
}
