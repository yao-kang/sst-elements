
#ifndef _H_SST_VANADIS_RISCV_DECODE
#define _H_SST_VANADIS_RISCV_DECODE

#include <sst/core/output.h>

#include "riscv/decodeload.h"
#include "riscv/decodestore.h"
#include "riscv/decodemath32.h"
#include "riscv/decodemathi.h"
#include "riscv/decodepc.h"
#include "riscv/decodebranch.h"
#include "riscv/decodemathiw.h"
#include "riscv/decodemath64.h"
#include "riscv/decodefmath64.h"
#include "riscv/decodefload.h"
#include "riscv/decodefstore.h"
#include "riscv/decodefmath64fma.h"

#include "icreader/icreader.h"
#include "utils/printutils.hpp"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_32BENCODE_MASK    0b00000000000000000000000000000011

class VanadisRISCVDecoder {

public:
	VanadisRISCVDecoder(SST::Output* out, InstCacheReader* icache) {
			output = out;
			icacheReader = icache;

			decodeLoad.setOutput(out);
			decodeStore.setOutput(out);
			decodeMath32.setOutput(out);
			decodeMathI.setOutput(out);
			decodeMathIW.setOutput(out);
			decodePCHandler.setOutput(out);
			decodeBranch.setOutput(out);
			decodeMath64.setOutput(out);
			decodeMathF64.setOutput(out);
			decodeFPLoad.setOutput(out);
			decodeFPStore.setOutput(out);
			decodeFP64FMA.setOutput(out);

	}
	~VanadisRISCVDecoder() {}

	VanadisDecodeResponse decode(const uint64_t ip) {
			uint32_t nextInst = 0;

	    const bool fillSuccess = icacheReader->fill(ip, &nextInst, 4);

		if(fillSuccess) {
			output->verbose(CALL_INFO, 4, 0, "Instruction cache read was successful for decode\n");
			output->verbose(CALL_INFO, 4, 0, "Response: 0x%" PRIx32 "\n", nextInst);
				
			printInstruction(ip, nextInst);
			
			if( (nextInst & VANADIS_32BENCODE_MASK) == VANADIS_32BENCODE_MASK ) {
				output->verbose(CALL_INFO, 4, 0, "Decode Check - 32b Format Success\n");
				
				const uint32_t operation = nextInst & VANADIS_32B_INST_MASK;
				printInstruction(ip, operation);
				
				VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

				switch(operation) {
				
				case VANADIS_LOAD_FAMILY:
					decodeResp = decodeLoad.decode(ip, nextInst);
					break;

				case VANADIS_STORE_FAMILY:
					decodeResp = decodeStore.decode(ip, nextInst);
					break;

				case VANADIS_MATH_FAMILY:
					decodeResp = decodeMath32.decode(ip, nextInst);
					break;

				case VANADIS_MATH64_FAMILY:
					decodeResp = decodeMath64.decode(ip, nextInst);
					break;

				case VANADIS_IMM_MATH_FAMILY:
					decodeResp = decodeMathI.decode(ip, nextInst);
					break;

				case VANADIS_BRANCH_FAMILY:
					decodeResp = decodeBranch.decode(ip, nextInst);
					break;

				case VANADIS_INST_ADDIW:
					decodeResp = decodeMathIW.decode(ip, nextInst);
					break;

				case VANADIS_F64MATH_FAMILY:
					decodeResp = decodeMathF64.decode(ip, nextInst);
					break;

				case VANADIS_FPLOAD_FAMILY:
					decodeResp = decodeFPLoad.decode(ip, nextInst);
					break;

				case VANADIS_FPSTORE_FAMILY:
					decodeResp = decodeFPStore.decode(ip, nextInst);
					break;

				case VANADIS_FP64FMA_FAMILY:
					decodeResp = decodeFP64FMA.decode(ip, nextInst);
					break;

				default:
					decodeResp = decodePCHandler.decode(ip, nextInst);
					break;

				}

				if( UNKNOWN_INSTRUCTION == decodeResp ) {
					// Do we want to add custom instructions here?
					// TODO: custom instructions can go here
					output->fatal(CALL_INFO, -1, "Error: Instruction code failed: 0x%" PRIx64 "\n", ip);
				}
			} else {
				output->verbose(CALL_INFO, 2, 0, "Decode Check - 32b Format Failed, Not Supported. Mark as UNKNOWN_INSTRUCTION (IP=0x%" PRIx64 ").\n", ip);
				return UNKNOWN_INSTRUCTION;
			}

			return SUCCESS;
		} else {
				output->verbose(CALL_INFO, 4, 0, "Instruction cache read could not be completed due to buffer fill failure.\n");

			return ICACHE_FILL_FAILED;
		}
	}
	
protected:

		void printInstruction(const uint64_t ip, const uint32_t inst) {
			char* instString = (char*) malloc( sizeof(char) * 33 );
			instString[32] = '\0';

			binaryStringize32(inst, instString);

			output->verbose(CALL_INFO, 2, 0, "PRE-DECODE INST: ip=%15" PRIu64 " | 0x%010" PRIx64 " : 0x%010" PRIx32 " | %s\n",
				ip, ip, inst, instString);
		}

		InstCacheReader* icacheReader;
		SST::Output* output;

		VanadisDecodeLoad    decodeLoad;
		VanadisDecodeStore   decodeStore;
		VanadisDecodeMath32  decodeMath32;
		VanadisDecodeMathI   decodeMathI;
		VanadisDecodePCHandler decodePCHandler;
		VanadisDecodeBranch  decodeBranch;
		VanadisDecodeMathIW decodeMathIW;
		VanadisDecodeMath64 decodeMath64;
		VanadisDecodeFPMath64 decodeMathF64;
		VanadisFPDecodeLoad decodeFPLoad;
		VanadisFPDecodeStore decodeFPStore;
		VanadisFP64FMADecodeLoad decodeFP64FMA;

	/*
	VanadisDecodeResponse decodeComplexInst(const uint64_t& ip, const uint64_t& inst) {
        uint32_t rd;
        uint32_t rs1;
        uint64_t imm;
        
        switch(inst & VANADIS_32B_INST_MASK) {
            case VANADIS_INST_LUI:
                decodeUType(inst, rd, imm);
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " LUI   rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            case VANADIS_INST_AUIPC:
                decodeUType(inst, rd, imm);
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " AUIPC rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            case VANADIS_INST_JAL:
                decodeUJType(inst, rd, imm);
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " JAL   rd=%" PRIu32 ",                , imm=%" PRIu64 "\n", ip, rd, imm);
                break;
            case VANADIS_INST_JALR:
                decodeIType(inst, rd, rs1, imm);
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " JALR  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            default:
                output->fatal(CALL_INFO, -1, "Unknown instruction: IP=%" PRIx64 ", Inst=%" PRIu32 "\n", ip, inst);
                return UNKNOWN_INSTRUCTION;
        }
        
        return SUCCESS;
    }
    
    VanadisDecodeResponse decodeImmMathFamily(const uint64_t& ip, const uint64_t& inst) {
        const uint32_t opType = inst & VANADIS_INST_IRSSB_TYPE;
        
        output->verbose(CALL_INFO, 1, 0, "Decode: opType: %" PRIu32 "\n", opType);
        
        uint32_t rd  = 0;
        uint32_t rs1 = 0;
        
        uint64_t imm = 0;
        
        decodeIType(inst, rd, rs1, imm);
        
        switch(opType) {
            case VANADIS_INST_ADDI:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " ADDI  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            case VANADIS_INST_SLTI:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLTI  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            case VANADIS_INST_SLTIU:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLTIU rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            case VANADIS_INST_XORI:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " XORI  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            case VANADIS_INST_ORI:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " ORI   rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            case VANADIS_INST_ANDI:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " ANDI  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
                break;
            default:
                const uint32_t shiftType = inst & VANADIS_IMM_SHIFT_MASK;
                
                char* instString = (char*) malloc( sizeof(char) * 33 );
                instString[32] = '\0';
                
                binaryStringize32(inst, instString);
                
                char* shiftString = (char*) malloc( sizeof(char) * 33 );
                shiftString[32] = '\0';
                
                binaryStringize32(shiftType, shiftString);
                
                output->verbose(CALL_INFO, 1, 0, "Decode-Additional: IP=0x%" PRIx64 " shift-class=0x%" PRIx32 " | %s | %s\n", ip, shiftType, instString, shiftString);
                
                uint32_t shamt = 0;
                decodeRType(inst, rd, rs1, shamt);
                
                switch(shiftType) {
                    case VANADIS_INST_SLLI:
                        output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLLI  rd=%" PRIu32 ", rs1=%" PRIu32 ", shamt=%" PRIu32 "\n",
                                        ip, rd, rs1, shamt);
                        break;
                    case VANADIS_INST_SRLI:
                        output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLRI  rd=%" PRIu32 ", rs1=%" PRIu32 ", shamt=%" PRIu32 "\n",
                                        ip, rd, rs1, shamt);
                        break;
                    case VANADIS_INST_SRAI:
                        output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLAI  rd=%" PRIu32 ", rs1=%" PRIu32 ", shamt=%" PRIu32 "\n",
                                        ip, rd, rs1, shamt);
                        break;
                    default:
                        return UNKNOWN_INSTRUCTION;
                }
        }
        
        return SUCCESS;
    }
    
    VanadisDecodeResponse decodeMath64Family(const uint64_t& ip, const uint64_t& inst) {
        const uint32_t opType = inst & VANADIS_INST_MATH_TYPE;
        
        output->verbose(CALL_INFO, 1, 0, "Decode: opType: %" PRIu32 "\n", opType);
        
        uint32_t rs1 = 0;
        uint32_t rs2 = 0;
        uint32_t rd  = 0;
        
        decodeRType(inst, rd, rs1, rs2);
        
        switch(opType) {
            case VANADIS_INST_ADDW:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " ADDW  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
                break;
            case VANADIS_INST_SUBW:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SUBW  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
                break;
            case VANADIS_INST_SLLW:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLLW  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
                break;
            case VANADIS_INST_SRLW:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SRLW  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
                break;
            case VANADIS_INST_SRAW:
                output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SRAW  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
                break;
            default:
                return UNKNOWN_INSTRUCTION;
        }
        
        return SUCCESS;
    }
    
	VanadisDecodeResponse decodeMathFamily(const uint64_t& ip, const uint64_t& inst) {
		const uint32_t opType = inst & VANADIS_INST_MATH_TYPE;
		
		output->verbose(CALL_INFO, 1, 0, "Decode: opType: %" PRIu32 "\n", opType);
		
		uint32_t rs1 = 0;
		uint32_t rs2 = 0;
		uint32_t rd  = 0;
		
		decodeRType(inst, rd, rs1, rs2);
		
		switch(opType) {
		case VANADIS_INST_ADD:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " ADD  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_SUB:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SUB  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_SLL:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLL  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_SLT:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLT  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_SLTU:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SLTU rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_XOR:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " XOR  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_SRL:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SRL  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_SRA:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SRA  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_OR:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " OR   rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		case VANADIS_INST_AND:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " AND  rd=%" PRIu32 ", rs1=%" PRIu32 ", rs2=%" PRIu32 "\n", ip, rd, rs1, rs2);
			break;
		default:
            return UNKNOWN_INSTRUCTION;
		}
		
        return SUCCESS;
	}
	
	VanadisDecodeResponse decodeBranchFamily(const uint64_t& ip, const uint64_t& inst) {
		const uint32_t branchType = inst & VANADIS_INST_IRSSB_TYPE;
		
		output->verbose(CALL_INFO, 1, 0, "Decode: branchType: %" PRIu32 "\n", branchType);
		
		uint32_t rs1 = 0;
		uint32_t rs2 = 0;

		uint64_t imm = 0;
		
		decodeSBType(inst, rs1, rs2, imm);
		
		switch(branchType) {
		case VANADIS_INST_BEQ:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " BEQ  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_BNE:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " BNE  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_BLT:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " BLT  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_BGE:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " BGE  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_BLTU:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " BLTU rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_BGEU:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " BGEU rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		default:
			return UNKNOWN_INSTRUCTION;
		}
		
		return SUCCESS;
	}

	VanadisDecodeResponse decodeLoadFamily(const uint64_t& ip, const uint64_t& inst) {
		const uint32_t loadType = inst & VANADIS_INST_IRSSB_TYPE;
		
		output->verbose(CALL_INFO, 1, 0, "Decode: loadType: %" PRIu32 "\n", loadType);
		
		uint32_t rd  = 0;
		uint32_t rs1 = 0;

		uint64_t imm = 0;
		
		decodeIType(inst, rd, rs1, imm);
		
		switch(loadType) {
		case VANADIS_INST_LB:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " LB  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
			break;
		case VANADIS_INST_LH:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " LH  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
			break;
		case VANADIS_INST_LW:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " LW  rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
			break;
		case VANADIS_INST_LBU:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " LBU rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
			break;
		case VANADIS_INST_LHU:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " LHU rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
			break;
		case VANADIS_INST_LD:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " LD rd=%" PRIu32 ", rs1=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rd, rs1, imm);
			break;
		default:
			return UNKNOWN_INSTRUCTION;
		}
		
		return SUCCESS;
	}
	
	VanadisDecodeResponse decodeStoreFamily(const uint64_t& ip, const uint64_t& inst) {
		const uint32_t loadType = inst & VANADIS_INST_IRSSB_TYPE;
		
		output->verbose(CALL_INFO, 1, 0, "Decode: loadType: %" PRIu32 "\n", loadType);
		
		uint32_t rs1 = 0;
		uint32_t rs2 = 0;
		uint64_t imm = 0;
		
		decodeSType(inst,rs1, rs2, imm);
		
		switch(loadType) {
		case VANADIS_INST_SB:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SB  rs1=%" PRIu32 ", rs2=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_SH:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SH  rs1=%" PRIu32 ", rs2=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_SW:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SW  rs1=%" PRIu32 ", rs2=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		case VANADIS_INST_SD:
			output->verbose(CALL_INFO, 1, 0, "Decode: IP=0x%" PRIx64 " SD  rs1=%" PRIu32 ", rs2=%" PRIu32 ", imm=%" PRIu64 "\n", ip, rs1, rs2, imm);
			break;
		default:
			return UNKNOWN_INSTRUCTION;
		}
		
		return SUCCESS;
	}
	
	void decodeUJType(const uint32_t& inst, uint32_t& rd, uint64_t& imm) {
		//                                                  #####
		const uint32_t rd_mask       = 0b00000000000000000000111110000000;
		//
		const uint32_t immMSB_mask   = 0b10000000000000000000000000000000;
		//
		const uint32_t bit10_1_mask  = 0b01111111111000000000000000000000;
		//
		const uint32_t bit11_mask    = 0b00000000000100000000000000000000;
		//
		const uint32_t bit19_12_mask = 0b00000000000011111111000000000000;
		
		const uint64_t fullBits      = 0b1111111111111111111111111111111111111111111000000000000000000000;
		
		rd = (inst & rd_mask) >> 7;
		
		const uint32_t immTemp = ( (inst & bit10_1_mask)  >> 21 ) +
								 ( (inst & bit11_mask)    >> 9  ) +
								 ( (inst & bit19_12_mask)       ) +
								 ( (inst & immMSB_mask)   >> 11 );
								 
		const uint32_t immMSBTemp = inst & immMSB_mask;
		
		imm = static_cast<uint64_t>(immTemp);
		
		if(immMSBTemp) {
			imm += fullBits;
		}
	}
	
	void decodeUType(const uint32_t& inst, uint32_t& rd, uint64_t& imm) {
		//                                                 #####
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		const uint32_t immMSB_mask = 0b10000000000000000000000000000000;
		//                                                 #####*******
		const uint32_t imm_mask    = 0b00000000000000000000111111111111;
		const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111100000000000000000000;
		
		rd = (inst & rd_mask) >> 7;
		
		const uint32_t immTemp = (inst & imm_mask) >> 12;
		const uint32_t immMSBTemp = (inst & immMSB_mask);
		
		if(immMSBTemp) {
			imm += fullBits;
		}
	}
	
	void decodeSType(const uint32_t& inst, uint32_t& rs1, uint32_t& rs2, uint64_t& imm) {
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                                    *****#####***#####*******
		const uint32_t rs2_mask    = 0b00000001111100000000000000000000;
		//                                    *****#####***#####*******
		const uint32_t immL_mask   = 0b00000000000000000000111110000000;
		//                                    *****#####***#####*******
		const uint32_t immU_mask   = 0b11111110000000000000000000000000;
		const uint32_t immMSB_mask = 0b10000000000000000000000000000000;
		const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111111111111000000000000;

		rs1 = (inst & rs1_mask) >> 15;
		rs2 = (inst & rs2_mask) >> 20;
		
		const uint32_t immTemp = ((inst & immL_mask) >> 7) + ((inst & immU_mask) >> 18);
		
		imm = static_cast<uint64_t>(immTemp);
		
		if(immTemp) {
			imm += fullBits;
		}
	}
    
    void decodeSBType(const uint32_t& inst, uint32_t& rs1, uint32_t& rs2, uint64_t& imm) {
        //                                         #####***#####*******
        const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
        //                                    *****#####***#####*******
        const uint32_t rs2_mask    = 0b00000001111100000000000000000000;
        //                                    *****#####***#####*******
        const uint32_t imm11_mask  = 0b00000000000000000000000010000000;
        //                                    *****#####***#####*******
        const uint32_t imm4_mask   = 0b00000000000000000000111100000000;
        //                                    *****#####***#####*******
        const uint32_t imm12_mask  = 0b10000000000000000000000000000000;
        //                                    *****#####***#####*******
        const uint32_t imm10_mask  = 0b01111110000000000000000000000000;

        
        const uint32_t immMSB_mask =                                 0b10000000000000000010000000000000;
        const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111111111110000000000000;
        
        rs1 = (inst & rs1_mask) >> 15;
        rs2 = (inst & rs2_mask) >> 20;
        
        const uint32_t immTemp = ((inst & imm4_mask) >> 6) + ((inst & imm10_mask) >> 19) +
            ((inst & imm11_mask) << 4) + ((inst & imm12_mask) >> 18);
        
        imm = static_cast<uint64_t>(immTemp);
        
        if(immTemp & immMSB_mask) {
            imm += fullBits;
        }
    }
	
	void decodeRType(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint32_t& rs2) {
		//                                              ***#####*******
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                                    *****#####***#####*******
		const uint32_t rs2_mask    = 0b00000001111100000000000000000000;

		
		rd  = (inst & rd_mask)  >> 7;
		rs1 = (inst & rs1_mask) >> 15;
		rs2 = (inst & rs2_mask) >> 20;
	}
	
	void decodeIType(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint64_t& imm) {
		//                                              ***#####*******
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                             ????????????#####***#####*******
		const uint32_t imm_mask    = 0b11111111111100000000000000000000;
		const uint32_t immMSB_mask = 0b10000000000000000000000000000000;
		//                                                 ????????????
		const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111111111111000000000000;
		
		rd  = (inst & rd_mask)  >> 7;
		rs1 = (inst & rs1_mask) >> 15;
		
		const uint32_t immTemp    = (inst & imm_mask);
		const uint32_t immMSBTemp = (inst & immMSB_mask);

		imm = static_cast<uint64_t>(immTemp);

		// Do we need to sign extend the immediate or not?
		if(immMSBTemp) {
			imm += fullBits;
		}
	}*/



};

}
}

#endif
