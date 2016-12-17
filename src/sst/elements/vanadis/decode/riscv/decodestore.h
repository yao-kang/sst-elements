
#ifndef _H_SST_VANADIS_DECODE_BLOCK_STORE
#define _H_SST_VANADIS_DECODE_BLOCK_STORE

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// STORE MASKS                                       ***     *******
#define VANADIS_STORE_FAMILY      0b00000000000000000000000000100011

// STORE MASKS                                       ***     *******
#define VANADIS_INST_SB           0b00000000000000000000000000100011
#define VANADIS_INST_SH			  0b00000000000000000001000000100011
#define VANADIS_INST_SW			  0b00000000000000000010000000100011
#define VANADIS_INST_SD			  0b00000000000000000011000000100011

class VanadisDecodeStore : public VanadisDecodeBlock {

public:
	VanadisDecodeStore() {

	}

	~VanadisDecodeStore() {}

	std::string getBlockName() {
		return "StoreDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_STORE_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rd  = 0;
		uint32_t rs1 = 0;
		uint64_t imm = 0;

		decodeSType(inst, rd, rs1, imm);

		switch(VANADIS_INST_IRSSB_TYPE & inst) {

		case VANADIS_INST_SB:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SB   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SH:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SH   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SW   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SD   %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
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
