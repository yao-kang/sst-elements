// Copyright 2019 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2019, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _MIPS_4KC_H
#define _MIPS_4KC_H

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <vector>

#include <sst/core/event.h>
#include <sst/core/sst_types.h>
#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/core/output.h>
#include <sst/core/elementinfo.h>

#include <sst/core/interfaces/simpleMem.h>
#include <sst/elements/memHierarchy/memEvent.h>

#include "spim.h"
#include "inst.h"
#include "reg.h"
#include "mem.h"

#include "cl-mem.h"
#include "cl-cache.h"
#include "cl-cycle.h"
#include "cl-tlb.h"
#include "cl-except.h"

namespace SST {
namespace MIPS4KCComponent {

struct pipe_stage {
  instruction *inst;
  int stage;
  mem_addr pc;
  reg_word op1, op2, op3;
  reg_word value;
  reg_word value1;
  double fop1, fop2;
  double fval;
  reg_word addr_value;
  mem_addr paddr;
  unsigned int req_num;
  int dslot;
  int exception;
  int cyl_count;
  int count;
  struct pipe_stage *next;
};
typedef struct pipe_stage *PIPE_STAGE;

typedef struct lab_use
{
  instruction *inst;            /* NULL => Data, not code */
  mem_addr addr;
  struct lab_use *next;
} label_use;

typedef struct lab
{
  char *name;                   /* Name of label */
  long addr;                    /* Address of label or 0 if not yet defined */
  unsigned global_flag : 1;     /* Non-zero => declared global */
  unsigned gp_flag : 1;         /* Non-zero => referenced off gp */
  unsigned const_flag : 1;      /* Non-zero => constant value (in addr) */
  struct lab *next;             /* Hash table link */
  struct lab *next_local;       /* Link in list of local labels */
  label_use *uses;              /* List of instructions that reference */
} label;                        /* label that has not yet been defined */

class MIPS4KC : public SST::Component {
public:
/* Element Library Info */
    SST_ELI_REGISTER_COMPONENT(MIPS4KC, "MIPS4KC", "MIPS4KC", SST_ELI_ELEMENT_VERSION(1,0,0),
            "MIPS 4KC processor", COMPONENT_CATEGORY_PROCESSOR)

    SST_ELI_DOCUMENT_PARAMS(
            {"verbose",                 "(uint) Determine how verbose the output from the CPU is", "0"},
            {"clock",                   "(string) Clock frequency", "1GHz"}
                            )

    SST_ELI_DOCUMENT_PORTS( {"mem_link", "Connection to memory", { "memHierarchy.MemEventBase" } } )

    /* Begin class definiton */
    MIPS4KC(SST::ComponentId_t id, SST::Params& params);
    void finish() {
    }

protected:
    void cl_run_program (mem_addr addr, int steps, int display);
    void cl_initialize_world (int run);
    void cycle_init (void);
    void mdu_and_fp_init (void);
    void print_pipeline (void);
    void print_pipeline_internal (char *buf);
    
    /* Util functions */
    //void add_breakpoint (mem_addr addr);
    //void delete_breakpoint (mem_addr addr);
    void fatal_error (const char *fmt, ...);
    void initialize_registers (void);
    void initialize_run_stack (int argc, char **argv);
    void initialize_world (int load_trap_handler);
    //void list_breakpoints (void);
    inst_info *map_int_to_inst_info (inst_info tbl[], int tbl_len, int num);
    inst_info *map_string_to_inst_info (inst_info tbl[], int tbl_len, char *id);
    int read_assembly_file (char *name);
    int run_program (mem_addr pc, int steps, int display, int cont_bkpt);
    mem_addr starting_address (void);
    char *str_copy (char *str);
    void write_startup_message (void);
    void *xmalloc (int);
    void *zmalloc (int);

    mem_addr copy_int_to_stack (int n);
    mem_addr copy_str_to_stack (char *s);
    void delete_all_breakpoints (void);

    /* Data functions */
    void align_data (int alignment);
    mem_addr current_data_pc (void);
    void data_begins_at_point (mem_addr addr);
    void enable_data_alignment (void);
    void end_of_assembly_file (void);
    void extern_directive (char *name, int size);
    void increment_data_pc (int value);
    void k_data_begins_at_point (mem_addr addr);
    void lcomm_directive (char *name, int size);
    void set_data_alignment (int);
    void set_data_pc (mem_addr addr);
    void set_text_pc (mem_addr addr);
    void store_byte (int value);
    void store_double (double *value);
    void store_float (double *value);
    void store_half (int value);
    void store_string (char *string, int length, int null_terminate);
    void store_word (int value);
    void user_kernel_data_segment (int to_kernel); 

    /* Symbol Table */
#if 0
    mem_addr find_symbol_address (const char *symbol);
    void flush_local_labels (void);
    void initialize_symbol_table (void);
    label *label_is_defined (char *name);
    label *lookup_label (char *name);
    label *make_label_global (char *name);
    void print_symbols (void);
    label *record_label (char *name, mem_addr address);
    //void record_data_uses_symbol (mem_addr location, label *sym);
    //void record_inst_uses_symbol (instruction *inst, label *sym);
    void resolve_a_label (label *sym, instruction *inst);
    void get_hash (char *name, int *slot_no, label **entry);
    void resolve_label_uses (label *sym);
    void resolve_a_label_sub (label *sym, instruction *inst, mem_addr pc);
    /* Map from name of a label to a label structure. */
    static const int LABEL_HASH_TABLE_SIZE = 8191;
    label *label_hash_table [LABEL_HASH_TABLE_SIZE];
#endif


    /* configs */
    FILE* pipe_out;
    FILE* message_out;
    FILE* console_out;
    FILE* console_in;
    int quiet;		/* No warning messages */
    int mapped_io;		/* Non-zero => activate memory-mapped IO */

    /* signal / exception */
    signal_desc siginfo[NSIG];
    excpt_desc *excpt_handler;
    mem_addr breakpoint_reinsert; /* !0 -> reinsert break at this
                                      address in IF because we've just
                                      processed it */
    spim_proc proc;			/* spim's signal tracking structure */

    /* The text segment and boundaries. */
    instruction **text_seg;
    int text_modified;	/* Non-zero means text segment was written */
    mem_addr text_top;
    mem_word *data_seg;
    int data_modified;	/* Non-zero means a data segment was written */
    short *data_seg_h;	/* Points to same vector as DATA_SEG */
    BYTE_TYPE *data_seg_b;	/* Ditto */
    mem_addr data_top;
    mem_addr gp_midpoint;	/* Middle of $gp area */
    /* The stack segment and boundaries. */
    mem_word *stack_seg;
    short *stack_seg_h;	/* Points to same vector as STACK_SEG */
    BYTE_TYPE *stack_seg_b;	/* Ditto */
    mem_addr stack_bot;
    /* The kernel text segment and boundaries. */
    instruction **k_text_seg;
    mem_addr k_text_top;
    /* Kernel data segment and boundaries. */
    mem_word *k_data_seg;
    short *k_data_seg_h;
    BYTE_TYPE *k_data_seg_b;
    mem_addr k_data_top;    

    mem_addr program_starting_address;   
    long initial_text_size;
    long initial_data_size;   
    long initial_data_limit;
    long initial_stack_size;
    long initial_stack_limit;
    long initial_k_text_size;
    long initial_k_data_size;
    long initial_k_data_limit;

    /* local counters for collecting statistics */
    int mem_stall_count;
    int total_mem_ops;
    
    int bd_slot;
    mult_div_unit MDU;

    /* Exported Variables: */
    int cycle_level, cycle_running, cycle_steps;
    int EX_bp_reg, MEM_bp_reg, CP_bp_reg, CP_bp_cc, CP_bp_z;
    reg_word EX_bp_val, MEM_bp_val, CP_bp_val;
    int FP_add_cnt, FP_mul_cnt, FP_div_cnt;
    PIPE_STAGE alu[5], fpa[6];

    /* registers */
    reg_word R[32];
    reg_word HI, LO;
    int HI_present, LO_present;
    mem_addr PC, nPC;

    /* Floating Point Coprocessor (1) registers :*/
    double *FPR;		/* Dynamically allocate so overlay */
    float *FGR;		/* is possible */
    int *FWR;		/* is possible */
    
    int FP_reg_present;	/* Presence bits for FP registers */
    int FP_reg_poison;	/* Poison bits for FP registers */
    int FP_spec_load;	/* Is register waiting for a speculative load */
    
    /* Other Coprocessor Registers.  The floating point registers
       (coprocessor 1) are above.  */
    reg_word CpCond[4], CCR[4][32], CPR[4][32];
    
    /* Exeception Handling Registers (actually registers in Coprocoessor
       0's register file) */    
    int exception_occurred;

    /* memory variables */
    MEM_SYSTEM mem_system;
    int dcache_modified, icache_modified;
    int line_size;
    int dcache_on, icache_on, tlb_on;

    /* Local functions: */
    int cycle_spim (int *steps, int display);
    int can_issue (short int oc);
    int process_ID (PIPE_STAGE ps, int *stall, int mult_div_busy);
    void process_EX (PIPE_STAGE ps, struct mult_div_unit *pMDU);
    int process_MEM (PIPE_STAGE ps, MEM_SYSTEM mem_sys);
    int process_WB (PIPE_STAGE ps);
    int process_f_ex1 (PIPE_STAGE ps);
    void process_f_ex2 (PIPE_STAGE ps);
    void process_f_fwb (PIPE_STAGE ps);
    void init_stage_pool (void);
    PIPE_STAGE stage_alloc (void);
    void stage_dealloc (PIPE_STAGE ps);
    void pipe_dealloc (int stage, PIPE_STAGE alu[], PIPE_STAGE fpa[]);
    void long_multiply (reg_word v1, reg_word v2, reg_word *hi, reg_word
                        *lo);


private:
    MIPS4KC();  // for serialization only
    MIPS4KC(const MIPS4KC&); // do not implement
    void operator=(const MIPS4KC&); // do not implement
    void init(unsigned int phase);
    
    void handleEvent( SST::Interfaces::SimpleMem::Request * req );
    virtual bool clockTic( SST::Cycle_t );

    Output out;
    Interfaces::SimpleMem * memory;
    std::map<uint64_t, void*> requests; //memory requests

    TimeConverter *clockTC;
    Clock::HandlerBase *clockHandler;

};

}
}
#endif /* _MIPS_4KC_H */
