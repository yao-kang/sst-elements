
#ifndef _H_SST_VANADIS_DECODE_BLOCK_MATH64
#define _H_SST_VANADIS_DECODE_BLOCK_MATH64

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// LOAD MASKS                                        ***     *******
#define VANADIS_MATH64_FAMILY     0b00000000000000000000000000111011

// MATH MASKS                       *******          ***     *******
#define VANADIS_INST_MATH64_MASK  0b11111110000000000111000001111111

// MATH MASKS                       *******          ***     *******
#define VANADIS_INST_ADDW         0b00000000000000000000000000111011
#define VANADIS_INST_SUBW         0b01000000000000000000000000111011
#define VANADIS_INST_SLLW         0b00000000000000000001000000111011
#define VANADIS_INST_SRLW         0b00000000000000000101000000111011
#define VANADIS_INST_SRAW         0b01000000000000000101000000111011

#define VANADIS_INST_MULW         0b00000010000000000000000000111011
#define VANADIS_INST_DIVW         0b00000010000000000100000000111011
#define VANADIS_INST_DIVUW        0b00000010000000000101000000111011
#define VANADIS_INST_REMW         0b00000010000000000110000000111011
#define VANADIS_INST_REMUW        0b00000010000000000111000000111011

class VanadisDecodeMath64 : public VanadisDecodeBlock {

public:
	VanadisDecodeMath64() {

	}

	~VanadisDecodeMath64() {}

	std::string getBlockName() {
		return "Math64Decoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_MATH_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rs1 = 0;
		uint32_t rs2 = 0;
		uint32_t rd  = 0;

		decodeRType(inst, rd, rs1, rs2);

		switch(VANADIS_INST_MATH64_MASK & inst) {

		case VANADIS_INST_ADDW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   ADDW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SUBW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SUBW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SLLW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SLLW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SRLW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SRLW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SRAW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SRAW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_MULW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   MULW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_DIVW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   DIVW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_DIVUW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   DIVUW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_REMW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   REMW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_REMUW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   REMUW %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		default:
			output->fatal(CALL_INFO, -1, "Decode Failure: IP=0x%" PRIx64 "\n", ip);
			break;

		}

		return decodeResp;
	}

};

}
}

#endif
