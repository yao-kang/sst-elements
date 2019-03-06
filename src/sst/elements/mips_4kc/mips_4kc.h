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
    void clear() {
        inst = 0;
        op1 = op2 = op3 = stage = 0;
        value = value1 = 0;
        fop1 = fop2 = 0;
        fval = 0;
        addr_value = paddr = 0;
        req_num = dslot = exception = cyl_count = 0;
        count = 0;
        next = 0;
    }
};
typedef struct pipe_stage *PIPE_STAGE;

#if 0
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
#endif

class MIPS4KC : public SST::Component {
public:
/* Element Library Info */
    SST_ELI_REGISTER_COMPONENT(MIPS4KC, "mips_4kc", "MIPS4KC", SST_ELI_ELEMENT_VERSION(1,0,0),
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
    int run_error (const char *fmt, ...);
    void error (const char *fmt, ...);
    void write_output (FILE*, const char *fmt, ...);
    void initialize_registers (void);
    void initialize_run_stack (int argc, char **argv);
    void initialize_world (int load_trap_handler);
    //void list_breakpoints (void);
    inst_info *map_int_to_inst_info (vector<inst_info> &tbl, int num);
    inst_info *map_string_to_inst_info (inst_info tbl[], int tbl_len, char *id);
    int read_assembly_file (char *name);
    int run_program (mem_addr pc, int steps, int display, int cont_bkpt);
    //mem_addr starting_address (void);
    char *str_copy (char *str);
    void write_startup_message (void);
    void *xmalloc (int);
    void *zmalloc (int);
    int read_aout_file (const char *file_name);

    mem_addr copy_int_to_stack (int n);
    mem_addr copy_str_to_stack (char *s);
    void delete_all_breakpoints (void);

    /* Data functions */
    void align_data (int alignment);
    mem_addr current_data_pc (void);
    void data_begins_at_point (mem_addr addr);
    void enable_data_alignment (void);
    void end_of_assembly_file (void);
    //void extern_directive (char *name, int size);
    void increment_data_pc (int value);
    void k_data_begins_at_point (mem_addr addr);
    //void lcomm_directive (char *name, int size);
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
    mem_addr next_data_pc;	/* Location for next datum in user process */
    mem_addr next_k_data_pc;	/* Location for next datum in kernel */    
    int in_kernel = 0;	/* Non-zero => data goes to kdata, not data */
    int data_dir = 0;
    int text_dir = 0;
    mem_addr next_text_pc; /* Locations for next instruction in user
                              and kernel text segments */
    mem_addr next_k_text_pc; /* Locations for next instruction in user
                                and kernel text segments */

    instruction *break_inst;
    int program_break;	/* Last address in data segment (edata) */
    /* tble mapping from opcode to instruction tye */
    int sorted_name_table = 0;	/* Non-zero => table sorted */
    /* Map between a SPIM instruction and the binary representation of
   the instruction. */
    int sorted_i_opcode_table = 0; /* Non-zero => table sorted */
    /* Maintain a table mapping from actual opcode to interal opcode.
       Table must be sorted before first use since its entries are
       alphabetical on name, not ordered by opcode. */
    int sorted_a_opcode_table = 0; /* Non-zero => table sorted */

    /* syscall stuff */
    int do_syscall (void);
    void handle_exception (void);
    void initialize_prog_fds (void);
    void kill_prog_fds (void);
    void print_syscall_usage (void);
 void do_sigreturn (mem_addr sigptr);
 int reverse_fds (int fd);
 void setup_signal_stack (void);
 int unixsyscall (void);
 int prog_sigmask = 0;	/* Copy of sigmask passed to system */
    //mem_addr exception_address[NSIG]; /* trampoline addresses for */
					 /* each signal handler */
 struct sigvec sighandler[NSIG]; /* Map to program handlers */
 int prog_fds[OPEN_MAX];	/* Map from program fds to simulator fds */
 int fds_initialized = 0;	/* FD map initialized? */

    /* Exceptions */
    void dosigreturn (mem_addr sigptr);
void initialize_catch_signals (void);
void initialize_sighandlers (void);
void initialize_excpt_counts (void);
int process_excpt (void);
void print_except_stats (void);
void print_signal_status (int sig);
 void intercept_signals (int sig, int code, struct sigcontext *scp);
 mem_addr compute_branch_target (instruction *inst);
 void psignal (int sig);
 int issig (void);
 int psig (void);
 void sendsig (void);

    /* configs */
    FILE* pipe_out;
    FILE* message_out;
    FILE* console_out;
    FILE* console_in;
    int quiet;		/* No warning messages */
    int mapped_io;		/* Non-zero => activate memory-mapped IO */

    /* signal / exception */
    static signal_desc siginfo[NSIG];
    static excpt_desc excpt_handler[];
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
    mem_addr gp_midpoint=0;	/* Middle of $gp area */
    /* The stack segment and boundaries. */
    mem_word *stack_seg=0;
    short *stack_seg_h=0;	/* Points to same vector as STACK_SEG */
    BYTE_TYPE *stack_seg_b=0;	/* Ditto */
    mem_addr stack_bot=0;
    /* The kernel text segment and boundaries. */
    instruction **k_text_seg=0;
    mem_addr k_text_top=0;
    /* Kernel data segment and boundaries. */
    mem_word *k_data_seg=0;
    short *k_data_seg_h=0;
    BYTE_TYPE *k_data_seg_b=0;
    mem_addr k_data_top=0;    

    mem_addr program_starting_address=0;   
    long initial_text_size=0;
    long initial_data_size=0;   
    long initial_data_limit=0;
    long initial_stack_size=0;
    long initial_stack_limit=0;
    long initial_k_text_size=0;
    long initial_k_data_size=0;
    long initial_k_data_limit=0;

    /* Instruction processing */
    //static int compare_pair_value (inst_info *p1, inst_info *p2);
    /*void i_type_inst_full_word (int opcode, int rt, int rs, imm_expr *expr,
      int value_known, long int value);*/
    void inst_cmp (instruction *inst1, instruction *inst2);
    instruction *make_r_type_inst (int opcode, int rd, int rs, int rt);
    instruction *mk_i_inst (unsigned long value, int opcode, int rs,
                            int rt, int offset);
    instruction *mk_j_inst (unsigned long value, int opcode, int target);
    instruction *mk_r_inst (unsigned long value, int opcode, int rs,
                            int rt, int rd, int shamt);
    int print_imm_expr (char *buf, imm_expr *expr, int base_reg);
    void sort_name_table (void);
    imm_expr *addr_expr_imm (addr_expr *expr);
    int addr_expr_reg (addr_expr *expr);
    //imm_expr *const_imm_expr (long int value);
    imm_expr *copy_imm_expr (imm_expr *old_expr);
    instruction *copy_inst (instruction *inst);
    mem_addr current_text_pc (void);
    //long eval_imm_expr (imm_expr *expr);
    void free_inst (instruction *inst);
    //void i_type_inst (int opcode, int rt, int rs, imm_expr *expr);
    //void i_type_inst_free (int opcode, int rt, int rs, imm_expr *expr);
    void increment_text_pc (int delta);
    //imm_expr *incr_expr_offset (imm_expr *expr, long int value);
    instruction *inst_decode (unsigned long int value);
    void sort_i_opcode_table (void);
    void sort_a_opcode_table (void);
    long inst_encode (instruction *inst);
    int inst_is_breakpoint (mem_addr addr);
    //void j_type_inst (int opcode, imm_expr *target);
    void k_text_begins_at_point ();
    void k_text_begins_at_point (mem_addr addr);
    //imm_expr *lower_bits_of_expr (imm_expr *old_expr);
    //addr_expr *make_addr_expr (long int offs, char *sym, int reg_no);
    //imm_expr *make_imm_expr (int offs, char *sym, int pc_rel);
    int opcode_is_branch (int opcode);
    int opcode_is_jump (int opcode);
    int opcode_is_load_store (int opcode);
    void print_inst (mem_addr addr);
    int print_inst_internal (char *buf, int len, instruction *inst, mem_addr addr);
    void r_cond_type_inst (int opcode, int rs, int rt);
    void r_sh_type_inst (int opcode, int rd, int rt, int shamt);
    void r_type_inst (int opcode, int rd, int rs, int rt);
    instruction *set_breakpoint (mem_addr addr);
    void store_instruction (instruction *inst);
    void text_begins_at_point (mem_addr addr);
    imm_expr *upper_bits_of_expr (imm_expr *old_expr);
    void user_kernel_text_segment (int to_kernel);
    int zero_imm (imm_expr *expr);

    /* local counters for collecting statistics */
    int mem_stall_count=0;
    int total_mem_ops=0;
    
    int bd_slot=0;
    mult_div_unit MDU;

    /* Exported Variables: */
    int cycle_level=0, cycle_running=0, cycle_steps=0, bare_machine=0;
    int EX_bp_reg=0, MEM_bp_reg=0, CP_bp_reg=0, CP_bp_cc=0, CP_bp_z=0;
    reg_word EX_bp_val=0, MEM_bp_val=0, CP_bp_val=0;
    int FP_add_cnt=0, FP_mul_cnt=0, FP_div_cnt=0;
    PIPE_STAGE alu[5]={0,0,0,0,0};
    PIPE_STAGE fpa[6]={0,0,0,0,0,0};

    /* registers */
    reg_word R[32] = {};
    reg_word HI=0, LO=0;
    int HI_present=0, LO_present=0;
    mem_addr PC=0, nPC=0;

    /* Floating Point Coprocessor (1) registers :*/
    double *FPR=0;		/* Dynamically allocate so overlay */
    float *FGR=0;		/* is possible */
    int *FWR=0;		/* is possible */
    
    int FP_reg_present=0;	/* Presence bits for FP registers */
    int FP_reg_poison=0;	/* Poison bits for FP registers */
    int FP_spec_load=0;	/* Is register waiting for a speculative load */
    
    /* Other Coprocessor Registers.  The floating point registers
       (coprocessor 1) are above.  */
    reg_word CpCond[4]={};
    reg_word CCR[4][32]={};
    reg_word CPR[4][32]={};
    
    /* Exeception Handling Registers (actually registers in Coprocoessor
       0's register file) */    
    int exception_occurred=0;

    /* memory variables */
    MEM_SYSTEM mem_system;
    int dcache_modified=0, icache_modified=0;
    int line_size=0;
    int dcache_on=0, icache_on=0, tlb_on=0;
    long data_size_limit=0, stack_size_limit=0, k_data_size_limit=0;
    /* Memory-mapped IO routines: */
    long recv_control=0, recv_buffer=0, recv_buffer_filled=0;
    long trans_control=0, trans_buffer=0, trans_buffer_filled=0;

    /* CL cache functions */
    void cache_wt_init (void);
void stat_init (void);
void stat_print (void);
unsigned int bus_service (MEM_SYSTEM mem_system);
MEM_SYSTEM mem_sys_init (void);
void print_mem_sys_status (int finished, MEM_SYSTEM mem_system);
void cache_init (MEM_SYSTEM mem_system, int type);
int cache_service (MEM_SYSTEM mem_system, mem_addr addr, int type, unsigned int
		   *req_num);
int cache_probe (CACHE cache, mem_addr addr, int type);
void print_cache_stats (char *buf, int type);
char *print_write_buffer (void);
void print_cache_data (char *buf, int type);
 int arbitrate (MEM_SYSTEM mem_system);
 int service_request(MEM_SYSTEM);
 void rb_insert (MEM_SYSTEM mem_system, mem_addr addr, unsigned int
		       *req_num);
 void ib_insert (MEM_SYSTEM mem_system, mem_addr addr, unsigned int
		       *req_num);
 int wb_insert (MEM_SYSTEM mem_system, mem_addr addr, unsigned int
		      *req_num);
 int wb_conflict (MEM_SYSTEM mem_system, mem_addr addr);
 unsigned int wb_promote (MEM_SYSTEM mem_system);
 void cache_update (CACHE cache, mem_addr addr, int type);
    int tlb_vat (mem_addr addr, unsigned int pid, int l_or_s, mem_addr *paddr) {
        *paddr = addr;
        return CACHEABLE;
    }
    

    /* memory functions */
    void free_instructions (register instruction **inst, int n);
    mem_word read_memory_mapped_IO (mem_addr addr);
    void write_memory_mapped_IO (mem_addr addr, mem_word value);
    mem_word bad_mem_read (mem_addr addr, int mask, mem_word *dest);
void bad_mem_write (mem_addr addr, mem_word value, int mask);
instruction *bad_text_read (mem_addr addr);
void bad_text_write (mem_addr addr, instruction *inst);
void check_memory_mapped_IO (void);
void expand_data (long int addl_bytes);
void expand_k_data (long int addl_bytes);
void expand_stack (long int addl_bytes);
void make_memory (long int text_size, long int data_size, long int data_limit,
	long int stack_size, long int stack_limit, long int k_text_size,
	long int k_data_size, long int k_data_limit);
void print_mem (mem_addr addr);
instruction *cl_bad_text_read (mem_addr addr, int *excpt);
void cl_bad_text_write (mem_addr addr, instruction *inst, int *excpt);
mem_word cl_bad_mem_read (mem_addr addr, int mask, mem_word *dest, int *excpt);
void cl_bad_mem_write (mem_addr addr, mem_word value, int mask, int *excpt);

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
