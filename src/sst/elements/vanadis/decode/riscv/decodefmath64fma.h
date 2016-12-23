#ifndef _H_SST_VANADIS_DECODE_BLOCK_FP64FMADD
#define _H_SST_VANADIS_DECODE_BLOCK_FP64FMADD

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                           *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_FP64FMA_TYPE      0b00000110000000000000000001111111

// LOAD MASKS                                                *******
#define VANADIS_FP64FMA_FAMILY    0b00000000000000000000000001000011

// LOAD MASKS                            **                  *******
#define VANADIS_INST_FMADDD       0b00000010000000000000000001000011

class VanadisFP64FMADecodeLoad : public VanadisDecodeBlock {

public:
	VanadisFP64FMADecodeLoad() {

	}

	~VanadisFP64FMADecodeLoad() {}

	std::string getBlockName() {
		return "FP64FMADecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_FP64FMA_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rd  = 0;
		uint32_t rs1 = 0;
		uint32_t rs2 = 0;
		uint32_t rs3 = 0;
		uint32_t roundMode = 0;

		decodeR4Type(inst, rd, rs1, rs2, rs3, roundMode);

		switch(VANADIS_FP64FMA_TYPE & inst) {

		case VANADIS_INST_FMADDD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FMADDD %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n",
					rd, rs1, rs2, rs3, roundMode);
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
