

#ifndef _H_SST_VANADIS_DECODE_BLOCK_MATH32
#define _H_SST_VANADIS_DECODE_BLOCK_MATH32

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

//                                                   ***     *******
#define VANADIS_32B_INST_MASK     0b00000000000000000000000001111111
#define VANADIS_INST_IRSSB_TYPE   0b00000000000000000111000001111111

// LOAD MASKS                                        ***     *******
#define VANADIS_MATH_FAMILY       0b00000000000000000000000000110011

// MATH MASKS                       *******          ***     *******
#define VANADIS_INST_MATH_MASK    0b11111110000000000111000001111111

// MATH MASKS                       *******          ***     *******
#define VANADIS_INST_ADD          0b00000000000000000000000000110011
#define VANADIS_INST_SUB          0b01000000000000000000000000110011
#define VANADIS_INST_SLL          0b00000000000000000001000000110011
#define VANADIS_INST_SLT          0b00000000000000000010000000110011
#define VANADIS_INST_SLTU         0b00000000000000000011000000110011
#define VANADIS_INST_XOR          0b00000000000000000100000000110011
#define VANADIS_INST_SRL          0b00000000000000000101000000110011
#define VANADIS_INST_SRA          0b01000000000000000101000000110011
#define VANADIS_INST_OR           0b00000000000000000110000000110011
#define VANADIS_INST_AND          0b00000000000000000111000000110011

#define VANADIS_INST_MUL          0b00000010000000000000000000110011
#define VANADIS_INST_MULH         0b00000010000000000001000000110011
#define VANADIS_INST_MULHSU       0b00000010000000000010000000110011
#define VANADIS_INST_MULHU        0b00000010000000000011000000110011
#define VANADIS_INST_DIV          0b00000010000000000100000000110011
#define VANADIS_INST_REM          0b00000010000000000110000000110011
#define VANADIS_INST_REMU         0b00000010000000000111000000110011

class VanadisDecodeMath32 : public VanadisDecodeBlock {

public:
	VanadisDecodeMath32() {

	}

	~VanadisDecodeMath32() {}

	std::string getBlockName() {
		return "Math32Decoder";
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

		switch(VANADIS_INST_MATH_MASK & inst) {

		case VANADIS_INST_ADD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   ADD  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SUB:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SUB  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SLL:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SLL  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SLT:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SLT  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SLTU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SLTU %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_XOR:
			output->verbose(CALL_INFO, 1, 0, "Decode:   XOR  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SRL:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SRL  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_SRA:
			output->verbose(CALL_INFO, 1, 0, "Decode:   SRA  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_OR:
			output->verbose(CALL_INFO, 1, 0, "Decode:   OR   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_AND:
			output->verbose(CALL_INFO, 1, 0, "Decode:   AND  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_MUL:
			output->verbose(CALL_INFO, 1, 0, "Decode:   MUL  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_MULH:
			output->verbose(CALL_INFO, 1, 0, "Decode:   MULH %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_MULHSU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   MULHSU %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_MULHU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   MULHU %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_DIV:
			output->verbose(CALL_INFO, 1, 0, "Decode:   DIV %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_REM:
			output->verbose(CALL_INFO, 1, 0, "Decode:   REM %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_REMU:
			output->verbose(CALL_INFO, 1, 0, "Decode:   REMU %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2);
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
