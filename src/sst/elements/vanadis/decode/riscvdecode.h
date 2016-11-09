
#ifndef _H_SST_VANADIS_RISCV_DECODE
#define _H_SST_VANADIS_RISCV_DECODE

#include <sst/core/output.h>
#include "icreader/icreader.h"
#include "utils/printutils.hpp"

namespace SST {
namespace Vanadis {

enum VanadisDecodeResponse {
	SUCCESS,
	UNKNOWN_REGISTER,
	ICACHE_FILL_FAILED,
	UNKNOWN_INSTRUCTION
};

//                                                 ***     *******
#define VANADIS_32B_INST_MASK   0b00000000000000000000000001111111
#define VANADIS_32BENCODE_MASK  0b00000000000000000000000000000011

//                                                 ***     *******
#define VANADIS_INST_IRSSB_TYPE 0b00000000000000000111000001111111

#define VANADIS_INST_LUI        0b00000000000000000000000000110111
#define VANADIS_INST_AUIPC      0b00000000000000000000000000010111
#define VANADIS_INST_JAL        0b00000000000000000000000001101111
#define VANADIS_INST_JALR       0b00000000000000000000000001100111

#define VANADIS_LOAD_FAMILY     0b00000000000000000000000000000011
#define VANADIS_STORE_FAMILY    0b00000000000000000000000000100011
#define VANADIS_IMM_MATH_FAMILY 0b00000000000000000000000000010011
#define VANADIS_FENCE_FAMILY    0b00000000000000000000000000001111
#define VANADIS_BRANCH_FAMILY   0b00000000000000000000000001100011

//                                                 ***     *******
#define VANADIS_INST_LB         0b00000000000000000000000000000011
#define VANADIS_INST_LH			0b00000000000000000001000000000011
#define VANADIS_INST_LW			0b00000000000000000010000000000011
#define VANADIS_INST_LBU		0b00000000000000000100000000000011
#define VANADIS_INST_LHU		0b00000000000000000101000000000011
#define VANADIS_INST_LD         0b00000000000000000011000000000011

//                                                 ***     *******
#define VANADIS_INST_SB         0b00000000000000000000000000100011
#define VANADIS_INST_SH			0b00000000000000000001000000100011
#define VANADIS_INST_SW			0b00000000000000000010000000100011

class VanadisRISCVDecoder {

public:
	VanadisRISCVDecoder(SST::Output* out, InstCacheReader* icache) {
			output = out;
			icacheReader = icache;
	}
	~VanadisRISCVDecoder() {}

	VanadisDecodeResponse decode(const uint64_t ip) {
			uint32_t nextInst = 0;

	    const bool fillSuccess = icacheReader->fill(ip, &nextInst, 4);

		if(fillSuccess) {
			output->verbose(CALL_INFO, 1, 0, "Instruction cache read was successful for decode\n");
			output->verbose(CALL_INFO, 1, 0, "Response: 0x%" PRIx32 "\n", nextInst);
				
			printInstruction(ip, nextInst);
			
			if( (nextInst & VANADIS_32BENCODE_MASK) == VANADIS_32BENCODE_MASK ) {
				output->verbose(CALL_INFO, 1, 0, "Decode Check - 32b Format Success\n");
				
				const uint32_t operation = nextInst & VANADIS_32B_INST_MASK;
				printInstruction(ip, operation);
				
				switch(operation) {
				
				case VANADIS_LOAD_FAMILY:
					return decodeLoadFamily(ip, nextInst);
				case VANADIS_STORE_FAMILY:
					return decodeStoreFamily(ip, nextInst);
				case VANADIS_IMM_MATH_FAMILY:
					break;
				case VANADIS_INST_LUI:
					break;
				case VANADIS_INST_AUIPC:
					break;
				case VANADIS_INST_JAL:
					break;
				case VANADIS_INST_JALR:
					break;
					
				}
			} else {
				output->verbose(CALL_INFO, 1, 0, "Decode Check - 32b Format Failed, Not Supported. Mark as UNKNOWN_INSTRUCTION.\n");
				return UNKNOWN_INSTRUCTION;
			}

			return SUCCESS;
		} else {
				output->verbose(CALL_INFO, 1, 0, "Instruction cache read could not be completed due to buffer fill failure.\n");

			return ICACHE_FILL_FAILED;
		}
	}

protected:
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
	}

	void printInstruction(const uint64_t ip, const uint32_t inst) {
		char* instString = (char*) malloc( sizeof(char) * 33 );
		instString[32] = '\0';
		
		binaryStringize32(inst, instString);
		
		output->verbose(CALL_INFO, 1, 0, "PRE-DECODE INST: ip=%15" PRIu64 " | 0x%010" PRIx64 " : 0x%010" PRIx32 " | %s\n", 
			ip, ip, inst, instString);
	}

	InstCacheReader* icacheReader;
	SST::Output* output;

};

}
}

#endif
