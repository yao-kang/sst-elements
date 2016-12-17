
#ifndef _H_SST_VANADIS_DECODE_BLOCK_PCHANDLER
#define _H_SST_VANADIS_DECODE_BLOCK_PCHANDLER

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111

#define VANADIS_INST_LUI          0b00000000000000000000000000110111
#define VANADIS_INST_AUIPC        0b00000000000000000000000000010111
#define VANADIS_INST_JAL          0b00000000000000000000000001101111
#define VANADIS_INST_JALR         0b00000000000000000000000001100111

class VanadisDecodePCHandler : public VanadisDecodeBlock {

public:
	VanadisDecodePCHandler() {

	}

	~VanadisDecodePCHandler() {}

	std::string getBlockName() {
		return "PCHandlerDecoder";
	}

	uint32_t getInstructionFamily() {
		return 0;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rd;
		uint32_t rs1;
		uint64_t imm;

		switch(VANADIS_32B_INST_MASK & inst) {

		case VANADIS_INST_LUI:
			decodeUType(inst, rd, imm);
			output->verbose(CALL_INFO, 1, 0, "Decode:   LUI   %5" PRIu32 " 0x%" PRIx64 "\n", rd, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_AUIPC:
			decodeUType(inst, rd, imm);
			output->verbose(CALL_INFO, 1, 0, "Decode:   AUIPC %5" PRIu32 " 0x%" PRIx64 "\n", rd, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_JAL:
			decodeUJType(inst, rd, imm);
			output->verbose(CALL_INFO, 1, 0, "Decode:   JAL   %5" PRIu32 " 0x%" PRIx64 "\n", rd, imm);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_JALR:
			decodeIType(inst, rd, rs1, imm);
			output->verbose(CALL_INFO, 1, 0, "Decode:   JALR  %5" PRIu32 " %5" PRIu32 " 0x%" PRIx64 "\n", rd, rs1, imm);
			decodeResp = SUCCESS;
			break;

		default:
			break;

		}

		return decodeResp;
	}

};

}
}

#endif
