
#ifndef _H_SST_VANADIS_DECODE_BLOCK_FMATH64
#define _H_SST_VANADIS_DECODE_BLOCK_FMATH64

#include "sst/elements/vanadis/decode/decodeblock.h"
#include "sst/elements/vanadis/decode/decoderesponse.h"

namespace SST {
namespace Vanadis {

// FAMILY                                            ***     *******
#define VANADIS_FPMATH_FAMILY     0b00000000000000000000000001010011

// MATH MASKS                       *******          ***     *******
#define VANADIS_INST_FMATH_MASK   0b11111110000000000000000001111111
#define VANADIS_INST_MATHSGN_MASK 0b11111110000000000111000001111111
#define VANADIS_INST_CNVT_MASK    0b11111111111100000000000001111111

// MATH MASKS                       *******          ***     *******
#define VANADIS_INST_FADDD        0b00000010000000000000000001010011
#define VANADIS_INST_FSUBD        0b00001010000000000000000001010011
#define VANADIS_INST_FMULD        0b00010010000000000000000001010011
#define VANADIS_INST_FDIVD        0b00011010000000000000000001010011

#define VANADIS_INST_FADDS        0b00000000000000000000000001010011
#define VANADIS_INST_FSUBS        0b00001000000000000000000001010011
#define VANADIS_INST_FMULS        0b00010000000000000000000001010011
#define VANADIS_INST_FDIVS        0b00011000000000000000000001010011

#define VANADIS_INST_FSQRTD       0b01011010000000000011000001010011
#define VANADIS_INST_FSGNJD       0b00100010000000000000000001010011
#define VANADIS_INST_FSGNJND      0b00100010000000000001000001010011
#define VANADIS_INST_FSGNJXD      0b00100010000000000010000001010011
#define VANADIS_INST_FMIND        0b00101010000000000000000001010011
#define VANADIS_INST_FMAXD        0b00101010000000000001000001010011

#define VANADIS_INST_FSQRTS       0b01011000000000000011000001010011
#define VANADIS_INST_FSGNJS       0b00100000000000000000000001010011
#define VANADIS_INST_FSGNJNS      0b00100000000000000001000001010011
#define VANADIS_INST_FSGNJXS      0b00100000000000000010000001010011
#define VANADIS_INST_FMINS        0b00101000000000000000000001010011
#define VANADIS_INST_FMAXS        0b00101000000000000001000001010011

/// FP CONVERT                      ************             *******
#define VANADIS_INST_FCVTSD       0b01000000000100000000000001010011
#define VANADIS_INST_FCVTDS       0b01000010000000000000000001010011
#define VANADIS_INST_FCVTWD       0b11000010000000000000000001010011
#define VANADIS_INST_FCVTWUD      0b11000010000100000000000001010011
#define VANADIS_INST_FCVTDW       0b11010010000000000000000001010011
#define VANADIS_INST_FCVTDWU      0b11010010000100000000000001010011
#define VANADIS_INST_FCVTLD       0b11000010001000000000000001010011
#define VANADIS_INST_FCVTLUD      0b11000010001100000000000001010011
#define VANADIS_INST_FCVTDL       0b11010010001000000000000001010011
#define VANADIS_INST_FCVTDLU      0b11010010001100000000000001010011
#define VANADIS_INST_FCVTWS       0b11000000000000000000000001010011
#define VANADIS_INST_FCVTWUS      0b11000000000100000000000001010011
#define VANADIS_INST_FCVTSW       0b11010000000000000000000001010011
#define VANADIS_INST_FCVTSWU      0b11010000000100000000000001010011

#define VANADIS_INST_FMVDX        0b11100010000000000000000001010011
#define VANADIS_INST_FMVXD        0b11110010000000000000000001010011
#define VANADIS_INST_FMVSX        0b11100000000000000000000001010011
#define VANADIS_INST_FMVXS        0b11110000000000000000000001010011

// FP COMPARE                       *******          ***     *******
#define VANADIS_INST_FEQD         0b10100010000000000010000001010011
#define VANADIS_INST_FLTD         0b10100010000000000001000001010011
#define VANADIS_INST_FLED         0b10100010000000000000000001010011

#define VANADIS_INST_FEQS         0b10100000000000000010000001010011
#define VANADIS_INST_FLTS         0b10100000000000000001000001010011
#define VANADIS_INST_FLES         0b10100000000000000000000001010011

// FP CLASS                         ************     ***     *******
#define VANADIS_INST_FCLASSD      0b11100010000000000001000001010011
#define VANADIS_INST_FCLASSS      0b11100000000000000001000001010011

class VanadisDecodeFPMath : public VanadisDecodeBlock {

public:
	VanadisDecodeFPMath() {

	}

	~VanadisDecodeFPMath() {}

	std::string getBlockName() {
		return "FPMathDecoder";
	}

	uint32_t getInstructionFamily() {
		return VANADIS_FPMATH_FAMILY;
	}

	VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) {
		VanadisDecodeResponse decodeResp = UNKNOWN_INSTRUCTION;

		printInstruction(ip, inst);

		uint32_t rs1 = 0;
		uint32_t rs2 = 0;
		uint32_t rs3 = 0;
		uint32_t rd  = 0;

		decodeR4Type(inst, rd, rs1, rs2, rs3);

		switch(VANADIS_INST_FMATH_MASK & inst) {

		case VANADIS_INST_FADDD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FADDD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FSUBD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FSUBD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FADDS:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FADDS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FSUBS:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FSUBS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FMULD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FMULD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FDIVD:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FDIVD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FMULS:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FMULS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		case VANADIS_INST_FDIVS:
			output->verbose(CALL_INFO, 1, 0, "Decode:   FDIVS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
			decodeResp = SUCCESS;
			break;

		default:
			switch(VANADIS_INST_MATHSGN_MASK & inst) {

			case VANADIS_INST_FSGNJD:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FSGNJD  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FSGNJND:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FSGNJND %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FSGNJXD:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FSGNJXD %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FMIND:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FMIND   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FMAXD:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FMAXD  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FEQD:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FEQD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FLTD:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FLTD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FLED:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FLED  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FSGNJS:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FSGNJS  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FSGNJNS:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FSGNJNS %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FSGNJXS:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FSGNJXS %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FMINS:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FMINS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FMAXS:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FMAXS  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FEQS:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FEQS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FLTS:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FLTS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			case VANADIS_INST_FLES:
				output->verbose(CALL_INFO, 1, 0, "Decode:   FLES  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, rs2, rs3);
				decodeResp = SUCCESS;
				break;

			default:
				uint64_t imm = 0;
				uint32_t roundMode = 0;
				decodeIType(inst, rd, rs1, imm, roundMode);

				switch(VANADIS_INST_CNVT_MASK & inst) {
				case VANADIS_INST_FCVTSD:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTSD  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTDS:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTDS  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTWD:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTWD  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTWUD:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTWUD %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTDW:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTDW  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTDWU:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTDWU %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTLD:
					output->verbose(CALL_INFO, 1, 0, "Decode:  FCVTLD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTLUD:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTLUD %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTDL:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTDL  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTDLU:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTDLU %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FMVDX:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FMVDX   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FMVXD:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FMVXD   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCLASSD:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCLASSD %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTWS:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTWS  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTWUS:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTWUS %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTSW:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTSW  %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCVTSWU:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCVTSWU %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FMVSX:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FMVSX   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FMVXS:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FMVXS   %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				case VANADIS_INST_FCLASSS:
					output->verbose(CALL_INFO, 1, 0, "Decode:   FCLASSS %5" PRIu32 " %5" PRIu32 " %5" PRIu32 "\n", rd, rs1, roundMode);
					decodeResp = SUCCESS;
					break;

				default:
					output->fatal(CALL_INFO, -1, "Decode Failure: IP=0x%" PRIx64 "\n", ip);
					break;
				}
			}
		}

		return decodeResp;
	}

};

}
}

#endif
