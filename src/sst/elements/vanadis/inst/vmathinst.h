
#ifndef _H_SST_VANADIS_MATH_INST_BASE
#define _H_SST_VANADIS_MATH_INST_BASE

#include "inst/vinst.h"

namespace SST {
namespace Vanadis {

class VanadisMathInstruction : public VanadisInstruction {

public:
	VanadisMathInstruction(SST::Output* output, const uint32_t id, const uint64_t iAddr,
			const uint32_t rRd, const uint32_t rR1, const uint32_t rR2) :
		VanadisInstruction(output, id, iAddr, 1),
		isaRegRd(rRd), isaRegR1(rR1), isaRegR2(rR2) {

	}

	~VanadisMathInstruction() {

	}

	VanadisInstructionFamily getFamily() {
		return MATH;
	}

	uint32_t getISAReadIntRegCount() {
		return 2;
	}

	uint32_t getISAReadFPRegCount() {
		return 0;
	}

	uint32_t getISAWriteIntRegCount() {
		return 1;
	}

	uint32_t getISAWriteFPRegCount() {
		return 0;
	}

	uint32_t getISAReadIntRegister(const uint32_t reg) {
		switch(reg) {
		case 0:
			return isaRegR1;
		case 1:
			return isaRegR2;
		default:
			output->fatal(CALL_INFO, -1, "Error: requested register %" PRIu32 " but math operations only have two read ISA registers.\n", reg);
			break;
		}

		return 0;
	}

	uint32_t getISAReadFPRegister(const uint32_t reg) {
		output->fatal(CALL_INFO, -1, "Error: math instruction requested a floating-point register.\n");
		return 0;
	}

	uint32_t getISAWriteIntRegister(const uint32_t reg) {
		switch(reg) {
			case 0:
				return isaRegRd;
			default:
				output->fatal(CALL_INFO, -1, "Error: requested register %" PRIu32 " but math operations can only have one write ISA register.\n", reg);
				break;
		}

		return 0;
	}

	uint32_t getISAWriteFPRegister(const uint32_t reg) {
		output->fatal(CALL_INFO, -1, "Error: math instruction requested a write floating-point register.\n");
	}

	void setArchReadIntRegister(const uint32_t index, const uint32_t archReg) {
		switch(index) {
		case 0:
			archRegR1 = archReg;
			break;
		case 1:
			archRegR2 = archReg;
			break;
		default:
			output->fatal(CALL_INFO, -1, "Error: attempted to set an architecture register of 2 or more in a math operation (index=%" PRIu32 ")\n",
					index);
		}
	}

	void setArchReadFPRegister(const uint32_t reg) {
		output->fatal(CALL_INFO, -1, "Error: attempted to set a floating point architecture register in a math operation.\n");
	}

	void setArchWriteIntRegister(const uint32_t reg) {
		switch(index) {
		case 0:
			archRegRd = archReg;
			break;
		default:
			output->fatal(CALL_INFO, -1, "Error: attempted to set an architecture register of 1 or more in a math operation (index=%" PRIu32 ")\n",
					index);
		}
	}

	void setArchWriteFPRegister(const uint32_t) {
		output->fatal(CALL_INFO, -1, "Error: attempted to set a floating point architecture register in a math operation.\n");
	}

	uint32_t getArchReadIntRegister(const uint32_t reg) {
		switch(reg) {
		case 0:
			return isaRegR1;
		case 1:
			return isaRegR2;
		default:
			output->fatal(CALL_INFO, -1, "Error: attempted to read architecture register greater than 1 in math operation\n");
		}

		return 0;
	}

	uint32_t getArchReadFPRegister(const uint32_t reg) {
		output->fatal(CALL_INFO, -1, "Error: attempted to get architecture FP registers in a math operation\n");
		return 0;
	}

	uint32_t getArchWriteIntRegister(const uint32_t reg) {
		switch(reg) {
		case 0:
			return isaRegR1;
		default:
			output->fatal(CALL_INFO, -1, "Error: attempted to read architecture register greater than 0 in math operation\n");
		}

		return 0;
	}

	uint32_t getArchWriteFPRegister(const uint32_t reg) {
		output->fatal(CALL_INFO, -1, "Error: attempted to get architecture FP registers in a math operation\n");
		return 0;
	}

protected:
        std::string generateRegisterString() {
            char* buff = (char*) malloc( sizeof(char) * 256 );
            
            sprintf(buff, "%" PRIu32 ", %" PRIu32 ", %" PRIu32 " (mapped to: %" PRIu32 ", %" PRIu32 ", %" PRIu32 ")",
                    isaRegRd, isaRegR1, isaRegR2, archRegRd, archRegR1, archRegR2);
            
            std::string buffStr(buff);
            free(buff);
            
            return buffStr;
        }
    
	const uint32_t isaRegRd;
	const uint32_t isaRegR1;
	const uint32_t isaRegR2;
	uint32_t archRegRd;
	uint32_t archRegR1;
	uint32_t archRegR2;

};

}
}

#endif
