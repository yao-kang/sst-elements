#ifndef _H_SST_VANADIS_DECODE_BLOCK_MATH_IW
#define _H_SST_VANADIS_DECODE_BLOCK_MATH_IW

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

#define VANADIS_INST_IW_SHIFT_M   0b11111110000000000111000001111111
//                                                   ***     *******
#define VANADIS_INST_ADDIW        0b00000000000000000000000000011011
#define VANADIS_INST_SLLIW        0b00000000000000000001000000011011
#define VANADIS_INST_SRLIW        0b00000000000000000101000000011011
#define VANADIS_INST_SRAIW        0b01000000000000000101000000011011

class VanadisDecodeMathIW : public VanadisDecodeBlock {

public:
	VanadisDecodeMathIW() {

	}

	~VanadisDecodeMathIW() {}

	std::string getBlockName() {
		return "MathIWDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_INST_ADDIW;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rd  = 0;
		uint32_t rs1 = 0;
		uint64_t imm = 0;

		decodeSType(inst, rd, rs1, imm);

		switch(VANADIS_INST_IRSSB_TYPE & inst) {

		case VANADIS_INST_ADDIW:
			output->verbose(CALL_INFO, 1, 0, "Decode:   ADDIW %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			printf("Returning successssssss\n");
			break;

		default:
			switch( VANADIS_INST_IW_SHIFT_M & inst ) {
			case VANADIS_INST_SLLIW:
				output->verbose(CALL_INFO, 1, 0, "Decode:   SLLIW %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_SRLIW:
				output->verbose(CALL_INFO, 1, 0, "Decode:   SRLIW %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_SRAIW:
				output->verbose(CALL_INFO, 1, 0, "Decode:   SRAIW %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
				decodeResp = SUCCESS;
				break;

			default:
				output->fatal(CALL_INFO, -1, "Decode Failure: IP=0x%" PRIx64 "\n", ip);
				break;

			}
		}

		return decodeResp;
	}

};

}
}

#endif
