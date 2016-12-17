
#ifndef _H_SST_VANADIS_DECODE_BLOCK_BRANCH
#define _H_SST_VANADIS_DECODE_BLOCK_BRANCH

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// STORE MASKS                                       ***     *******
#define VANADIS_BRANCH_FAMILY     0b00000000000000000000000001100011

// BRANCH MASKS                                      ***     *******
#define VANADIS_INST_BEQ	      0b00000000000000000000000001100011
#define VANADIS_INST_BNE	      0b00000000000000000001000001100011
#define VANADIS_INST_BLT	      0b00000000000000000100000001100011
#define VANADIS_INST_BGE	      0b00000000000000000101000001100011
#define VANADIS_INST_BLTU	      0b00000000000000000110000001100011
#define VANADIS_INST_BGEU	      0b00000000000000000111000001100011

class VanadisDecodeBranch : public VanadisDecodeBlock {

public:
	VanadisDecodeBranch() {

	}

	~VanadisDecodeBranch() {}

	std::string getBlockName() {
		return "BranchDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_BRANCH_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rs1 = 0;
		uint32_t rs2 = 0;

		uint64_t imm = 0;

		decodeSBType(inst, rs1, rs2, imm);

		switch(VANADIS_INST_IRSSB_TYPE & inst) {

		case VANADIS_INST_BEQ:
			output->verbose(CALL_INFO, 1, 0, "Decode:   BEQ  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_BNE:
			output->verbose(CALL_INFO, 1, 0, "Decode:   BNE  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_BLT:
			output->verbose(CALL_INFO, 1, 0, "Decode:   BLT  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_BGE:
			output->verbose(CALL_INFO, 1, 0, "Decode:   BGE  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_BLTU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   BLTU %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_BGEU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   BGEU %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
			decodeResp = SUCCESS;
			break;

		default:
			output->fatal(CALL_INFO, -1, "Decode Failure: IP=0x%" PRIu64 "\n", ip);
			break;

		}

		return decodeResp;
	}

};

}
}

#endif
