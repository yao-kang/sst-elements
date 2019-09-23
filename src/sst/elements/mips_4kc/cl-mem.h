
/* $Header: /var/home/larus/Software/larus/SPIM/RCS/cl-mem.h,v 1.2 1992/11/06 17:47:18 larus Exp $
*/

#ifndef _CL_MEM_H
#define _CL_MEM_H

/* New Memory Macros ... have an extra argument for signalling an exception */


#define CL_READ_MEM_INST(LOC, ADDR, PADDR, CMISS, EXPT, RNUM)	     \
{mem_addr _addr_ = (mem_addr) (ADDR);			             \
 unsigned int tmp;                                                           \
 tmp = tlb_vat(ADDR, 0, 1, &PADDR);					\
 if (tmp == CACHEABLE) {						\
   CMISS = cache_service (PADDR, ILOAD, &RNUM);			\
   if (_addr_ >= TEXT_BOT && _addr_ < text_top && !(_addr_ & 0x3))           \
     LOC = text_seg [(_addr_ - TEXT_BOT) >> 2];		                     \
   else if (_addr_ >= K_TEXT_BOT && _addr_ < k_text_top && !(_addr_ & 0x3))  \
     LOC = k_text_seg [(_addr_ - K_TEXT_BOT) >> 2];			     \
   else cl_bad_text_read (_addr_, &EXPT);				\
   }									\
 else CL_RAISE_EXCEPTION(tmp, 0, EXPT)     				\
}


#define CL_READ_MEM_BYTE(LOC, ADDR, PADDR, CMISS, EXPT, RNUM)            \
{mem_addr _addr_ = (mem_addr) (ADDR);			             \
 unsigned int tmp;                                                           \
 tmp = tlb_vat(ADDR, 0, 1, &PADDR);                                          \
 if (tmp == CACHEABLE) {                                                     \
   CMISS = cache_service(PADDR, LOAD, &RNUM);                            \
   if (_addr_ >= DATA_BOT && _addr_ < data_top)			             \
    LOC = data_seg_b [_addr_ - DATA_BOT];			             \
   else if (_addr_ >= stack_bot && _addr_ < STACK_TOP)		             \
    LOC = stack_seg_b [_addr_ - stack_bot];			             \
   else if (_addr_ >= K_DATA_BOT && _addr_ < k_data_top)	             \
    LOC = k_data_seg_b [_addr_ - K_DATA_BOT];			             \
   else cl_bad_mem_read (_addr_, 0, &LOC, &EXPT);}                           \
 else  CL_RAISE_EXCEPTION(tmp, 0, EXPT)                                      \
 }


#define CL_READ_MEM_HALF(LOC, ADDR, PADDR, CMISS, EXPT, RNUM)            \
{mem_addr _addr_ = (mem_addr) (ADDR);			             \
 unsigned int tmp;                                                           \
 tmp = tlb_vat(ADDR, 0, 1, &PADDR);                                          \
 if (tmp == CACHEABLE) {                                                     \
   CMISS = cache_service(PADDR, LOAD, &RNUM);                            \
   if (_addr_ >= DATA_BOT && _addr_ < data_top && !(_addr_ & 0x1))           \
     LOC = data_seg_h [(_addr_ - DATA_BOT) >> 1];		             \
   else if (_addr_ >= stack_bot && _addr_ < STACK_TOP && !(_addr_ & 0x1))    \
     LOC = stack_seg_h [(_addr_ - stack_bot) >> 1];		             \
   else if (_addr_ >= K_DATA_BOT && _addr_ < k_data_top && !(_addr_ & 0x1))  \
     LOC = k_data_seg_h [(_addr_ - K_DATA_BOT) >> 1];		             \
   else cl_bad_mem_read (_addr_, 0x1, &LOC, &EXPT);}                         \
 else   CL_RAISE_EXCEPTION(tmp, 0, EXPT)                                     \
}



#define CL_READ_MEM_WORD(LOC, ADDR, PADDR, CMISS, EXPT, RNUM)	      \
{mem_addr _addr_ = (mem_addr) (ADDR);			              \
 unsigned int tmp;                                                            \
 tmp = tlb_vat(ADDR, 0, 1, &PADDR);                                           \
 if (tmp == CACHEABLE) {                                                      \
   CMISS = cache_service(PADDR, LOAD, &RNUM);                             \
   if (_addr_ >= DATA_BOT && _addr_ < data_top && !(_addr_ & 0x3))            \
     LOC = data_seg [(_addr_ - DATA_BOT) >> 2];			              \
   else if (_addr_ >= stack_bot && _addr_ < STACK_TOP && !(_addr_ & 0x3))     \
     LOC = stack_seg [(_addr_ - stack_bot) >> 2];		              \
   else if (_addr_ >= K_DATA_BOT && _addr_ < k_data_top && !(_addr_ & 0x3))   \
     LOC = k_data_seg [(_addr_ - K_DATA_BOT) >> 2];		              \
   else cl_bad_mem_read (_addr_, 0x3, &LOC, &EXPT); }                         \
 else  CL_RAISE_EXCEPTION(tmp, 0, EXPT)                                       \
 }


#define CL_SET_MEM_INST(ADDR, INST, CMISS, EXPT)		             \
{mem_addr _addr_ = (mem_addr) (ADDR);			             \
 unsigned int tmp;                                                           \
 CMISS = CACHE_HIT;                                                          \
 text_modified = 1;						             \
 if (_addr_ >= TEXT_BOT && _addr_ < text_top && !(_addr_ & 0x3))             \
   text_seg [(_addr_ - TEXT_BOT) >> 2] = INST;		                     \
 else if (_addr_ >= K_TEXT_BOT && _addr_ < k_text_top && !(_addr_ & 0x3))    \
   k_text_seg [(_addr_ - K_TEXT_BOT) >> 2] = INST;		             \
 else cl_bad_text_write (_addr_, INST, &EXPT);}                              \
 }

#define CL_SET_MEM_BYTE(ADDR, PADDR, VALUE, CMISS, EXPT, RNUM)	\
{mem_addr _addr_ = (mem_addr) (ADDR);			        \
 unsigned int tmp;                                                      \
 tmp = tlb_vat(ADDR, 0, 0, &PADDR);                                     \
 if (tmp == CACHEABLE) {                                                \
   CMISS = cache_service(PADDR, STORE, &RNUM);                      \
   data_modified = 1;						        \
   if (_addr_ >= DATA_BOT && _addr_ < data_top)			        \
     data_seg_b [_addr_ - DATA_BOT] = (unsigned char) (VALUE);	        \
   else if (_addr_ >= stack_bot && _addr_ < STACK_TOP)		        \
     stack_seg_b [_addr_ - stack_bot] = (unsigned char) (VALUE);        \
   else if (_addr_ >= K_DATA_BOT && _addr_ < k_data_top)	        \
     k_data_seg_b [_addr_ - K_DATA_BOT] = (unsigned char) (VALUE);      \
   else cl_bad_mem_write (_addr_, VALUE, 0, &EXPT);}                    \
 else  CL_RAISE_EXCEPTION(tmp, 0, EXPT)                                 \
 }


#define CL_SET_MEM_HALF(ADDR, PADDR, VALUE, CMISS, EXPT, RNUM)           \
{mem_addr _addr_ = (mem_addr) (ADDR);			             \
 unsigned int tmp;                                                           \
 tmp = tlb_vat(ADDR, 0, 0, &PADDR);                                          \
 if (tmp == CACHEABLE) {                                                     \
     CMISS = cache_service(PADDR, STORE, &RNUM);                     \
   data_modified = 1;						             \
   if (_addr_ >= DATA_BOT && _addr_ < data_top && !(_addr_ & 0x1))           \
     data_seg_h [(_addr_ - DATA_BOT) >> 1] = (unsigned short) (VALUE);	     \
   else if (_addr_ >= stack_bot && _addr_ < STACK_TOP && !(_addr_ & 0x1))    \
     stack_seg_h [(_addr_ - stack_bot) >> 1] = (unsigned short) (VALUE);     \
   else if (_addr_ >= K_DATA_BOT && _addr_ < k_data_top && !(_addr_ & 0x1))  \
     k_data_seg_h [(_addr_ - K_DATA_BOT) >> 1] = (unsigned short) (VALUE);   \
   else cl_bad_mem_write (_addr_, VALUE, 0x1, &EXPT);}                       \
 else  CL_RAISE_EXCEPTION(tmp, 0, EXPT)                                      \
 }


#define CL_SET_MEM_WORD(ADDR, PADDR, VALUE, CMISS, EXPT, RNUM)           \
{mem_addr _addr_ = (mem_addr) (ADDR);			             \
 unsigned int tmp;                                                           \
 tmp = tlb_vat(ADDR, 0, 0, &PADDR);                                          \
 if (tmp == CACHEABLE) {                                                     \
   CMISS = cache_service(PADDR, STORE, &RNUM);                           \
   data_modified = 1;						             \
   if (_addr_ >= DATA_BOT && _addr_ < data_top && !(_addr_ & 0x3))           \
     data_seg [(_addr_ - DATA_BOT) >> 2] = (mem_word) (VALUE);	             \
   else if (_addr_ >= stack_bot && _addr_ < STACK_TOP && !(_addr_ & 0x3))    \
     stack_seg [(_addr_ - stack_bot) >> 2] = (mem_word) (VALUE);             \
   else if (_addr_ >= K_DATA_BOT && _addr_ < k_data_top && !(_addr_ & 0x3))  \
     k_data_seg [(_addr_ - K_DATA_BOT) >> 2] = (mem_word) (VALUE);           \
   else	 cl_bad_mem_write (_addr_, VALUE, 0x3, &EXPT);}                      \
 else  CL_RAISE_EXCEPTION(tmp, 0, EXPT)                                      \
 }





#endif // _CL_MEM_H
