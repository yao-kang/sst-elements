#ifndef _H_SST_VANADIS_DECODE_BLOCK_SYSTEM
#define _H_SST_VANADIS_DECODE_BLOCK_SYSTEM

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                           *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_SYSTEM_TYPE       0b11111111111100000000000001111111

// LOAD MASKS                                                *******
#define VANADIS_SYSTEM_FAMILY     0b00000000000000000000000001110011

// LOAD MASKS                                                *******
#define VANADIS_INST_SCALL        0b00000000000000000000000001110011
#define VANADIS_INST_SBREAK       0b00000000000100000000000001110011

class VanadisSystemDecodeStore : public VanadisDecodeBlock {

public:
	VanadisSystemDecodeStore() {

	}

	~VanadisSystemDecodeStore() {}

	std::string getBlockName() {
		return "SystemDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_FPSTORE_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		switch(VANADIS_SYSTEM_TYPE & inst) {

		case VANADIS_INST_SCALL:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SCALL\n");
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SBREAK:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SBREAK\n");
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
