#ifndef _H_SST_VANADIS_DECODE_BLOCK_FPLOAD
#define _H_SST_VANADIS_DECODE_BLOCK_FPLOAD

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// LOAD MASKS                                        ***     *******
#define VANADIS_FPLOAD_FAMILY     0b00000000000000000000000000000111

// LOAD MASKS                                        ***     *******
#define VANADIS_INST_FLD          0b00000000000000000011000000000111
#define VANADIS_INST_FLW          0b00000000000000000010000000000111

class VanadisFPDecodeLoad : public VanadisDecodeBlock {

public:
	VanadisFPDecodeLoad() {

	}

	~VanadisFPDecodeLoad() {}

	std::string getBlockName() {
		return "LoadFPDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_FPLOAD_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rd  = 0;
		uint32_t rs1 = 0;
		uint64_t imm = 0;

		decodeIType(inst, rd, rs1, imm);

		switch(VANADIS_INST_IRSSB_TYPE & inst) {

		case VANADIS_INST_FLD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FLD   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FLW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FLW   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
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
