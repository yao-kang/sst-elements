

#ifndef _H_SST_VANADIS_DECODE_BLOCK_MATH_I
#define _H_SST_VANADIS_DECODE_BLOCK_MATH_I

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// LOAD MASKS                                        ***     *******
#define VANADIS_IMM_MATH_FAMILY   0b00000000000000000000000000010011

// MATH MASKS                       *******          ***     *******
#define VANADIS_INST_MATHI_MASK   0b00000000000000000111000001111111

// MATH-IMM MASKS                                    ***     *******
#define VANADIS_INST_ADDI	      0b00000000000000000000000000010011
#define VANADIS_INST_SLTI	      0b00000000000000000010000000010011
#define VANADIS_INST_SLTIU        0b00000000000000000011000000010011
#define VANADIS_INST_XORI         0b00000000000000000100000000010011
#define VANADIS_INST_ORI          0b00000000000000000110000000010011
#define VANADIS_INST_ANDI         0b00000000000000000111000000010011

// MATH-IMM MASKS                   ******           ***     *******
#define VANADIS_IMM_SHIFT_MASK    0b11111100000000000111000001111111

#define VANADIS_INST_SLLI         0b00000000000000000001000000010011
#define VANADIS_INST_SRLI         0b00000000000000000101000000010011
#define VANADIS_INST_SRAI         0b01000000000000000001000000010011


class VanadisDecodeMathI : public VanadisDecodeBlock {

public:
	VanadisDecodeMathI() {

	}

	~VanadisDecodeMathI() {}

	std::string getBlockName() {
		return "MathIDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_IMM_MATH_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rd  = 0;
		uint32_t rs1 = 0;

		uint64_t imm = 0;

		decodeIType(inst, rd, rs1, imm);

		switch(VANADIS_INST_MATHI_MASK & inst) {

		case VANADIS_INST_ADDI:
			output->verbose(CALL_INFO, 1, 0, "Decode:  ADDI  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SLTI:
			output->verbose(CALL_INFO, 1, 0, "Decode:  SLTI  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SLTIU:
			output->verbose(CALL_INFO, 1, 0, "Decode:  SLTIU %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_XORI:
			output->verbose(CALL_INFO, 1, 0, "Decode:  XORI  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_ORI:
			output->verbose(CALL_INFO, 1, 0, "Decode:  ORI   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		default:

			uint32_t shamt = 0;
			decodeRType(inst, rd, rs1, shamt);

			switch(VANADIS_IMM_SHIFT_MASK & inst) {

			case VANADIS_INST_SLLI:
				output->verbose(CALL_INFO, 1, 0, "Decode:  SLLI %5" PRIu32 " %5" PRIu32 " 0x%" PRIx32 "\n", rd, rs1, shamt);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_SRLI:
				output->verbose(CALL_INFO, 1, 0, "Decode:  SRLI %5" PRIu32 " %5" PRIu32 " 0x%" PRIx32 "\n", rd, rs1, shamt);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_SRAI:
				output->verbose(CALL_INFO, 1, 0, "Decode:  SRAI %5" PRIu32 " %5" PRIu32 " 0x%" PRIx32 "\n", rd, rs1, shamt);
				decodeResp = SUCCESS;
				break;

			default:
				output->fatal(CALL_INFO, -1, "Decode Failure: IP=0x%" PRIu64 "\n", ip);
				break;

			}
		}

		return decodeResp;
	}

};

}
}

#endif
