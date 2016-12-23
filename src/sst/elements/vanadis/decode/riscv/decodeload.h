

#ifndef _H_SST_VANADIS_DECODE_BLOCK_LOAD
#define _H_SST_VANADIS_DECODE_BLOCK_LOAD

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// LOAD MASKS                                        ***     *******
#define VANADIS_LOAD_FAMILY       0b00000000000000000000000000000011

// LOAD MASKS                                        ***     *******
#define VANADIS_INST_LB           0b00000000000000000000000000000011
#define VANADIS_INST_LH			  0b00000000000000000001000000000011
#define VANADIS_INST_LW			  0b00000000000000000010000000000011
#define VANADIS_INST_LWU		  0b00000000000000000110000000000011
#define VANADIS_INST_LBU		  0b00000000000000000100000000000011
#define VANADIS_INST_LHU		  0b00000000000000000101000000000011
#define VANADIS_INST_LD           0b00000000000000000011000000000011

class VanadisDecodeLoad : public VanadisDecodeBlock {

public:
	VanadisDecodeLoad() {

	}

	~VanadisDecodeLoad() {}

	std::string getBlockName() {
		return "LoadDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_LOAD_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rd  = 0;
		uint32_t rs1 = 0;
		uint64_t imm = 0;

		decodeIType(inst, rd, rs1, imm);

		switch(VANADIS_INST_IRSSB_TYPE & inst) {

		case VANADIS_INST_LB:
			output->verbose(CALL_INFO, 1, 0, "Decode:   LB   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_LH:
			output->verbose(CALL_INFO, 1, 0, "Decode:   LH   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_LW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   LW   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_LWU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   LWU  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_LBU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   LBU  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_LHU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   LHU  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_LD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   LD   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
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
