#ifndef _H_SST_VANADIS_DECODE_BLOCK_FPSTORE
#define _H_SST_VANADIS_DECODE_BLOCK_FPSTORE

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// LOAD MASKS                                        ***     *******
#define VANADIS_FPSTORE_FAMILY    0b00000000000000000000000000100111

// LOAD MASKS                                        ***     *******
#define VANADIS_INST_FSD          0b00000000000000000011000000100111
#define VANADIS_INST_FSW          0b00000000000000000010000000100111

class VanadisFPDecodeStore : public VanadisDecodeBlock {

public:
	VanadisFPDecodeStore() {

	}

	~VanadisFPDecodeStore() {}

	std::string getBlockName() {
		return "StoreFPDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_FPSTORE_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rs1 = 0;
		uint32_t rs2 = 0;
		uint64_t imm = 0;

		decodeSType(inst, rs1, rs2, imm);

		switch(VANADIS_INST_IRSSB_TYPE & inst) {

		case VANADIS_INST_FSD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FSD   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FSW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FSW   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rs1, rs2, imm);
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
