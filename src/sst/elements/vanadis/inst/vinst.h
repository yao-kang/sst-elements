
#ifndef _H_VANADIS_INSTRUCTION_BASE
#define _H_VANADIS_INSTRUCTION_BASE

#include <sst/core/output.h>

namespace SST {
namespace Vanadis {

enum VanadisInstructionFamily {
	LOAD = 0,
	STORE = 1,
	MATH = 2,
	FPMATH = 3,
	BRANCH = 4,
	SYSTEM = 5,
	UNKNOWN = 99
};

enum VanadisInstructionError {
	NONE,
	DECODE_ERROR,
	DIV_ZERO
};

class VanadisInstruction {

	VanadisInstruction(SST::Output* output, const uint32_t uniqID, const uint64_t iAddr) :
		id(uniqID), addr(iAddr), intRF(NULL), fpRF(NULL) {

		instErr = NONE;
		cyclesSpentExecuting = 0;
	}

	VanadisInstruction(SST::Output* output, const uint32_t uniqID, const uint64_t iAddr, const uint32_t executeCycles,
                        VanadisRegisterFile* intRegFile, VanadisRegisterFile* fpRegFile) :
			id(uniqID), addr(iAddr), cyclesSpentExecuting(executeCycles),
                        intRF(intRegFile), fpRF(fpRegFile) {

		instErr = NONE;
	}

	~VanadisInstruction() {}

	virtual VanadisInstructionFamily getFamily() = 0;

	uint64_t getInstAddr() const {
		return addr;
	}

	bool raisesError() const {
		return (instErr != NONE);
	}

	void markError(VanadisInstructionError newErr) {
		instErr = newErr;
	}

	void decrementPending() {
		if(cyclesSpentExecuting)
			cyclesSpentExecuting--;
	}

	bool completed() const {
		return cyclesSpentExecuting != 0;
	}

	uint32_t getInstID() const {
		return id;
	}

	virtual void execute() = 0;

	virtual std::string toString() = 0;
	virtual std::string toMnemonic() = 0;

	virtual uint32_t getISAReadIntRegCount() = 0;
	virtual uint32_t getISAReadFPRegCount() = 0;
	virtual uint32_t getISAWriteIntRegCount() = 0;
	virtual uint32_t getISAWriteFPRegCount() = 0;

	virtual uint32_t getISAReadIntRegister(const uint32_t reg) = 0;
	virtual uint32_t getISAReadFPRegister(const uint32_t reg) = 0;
	virtual uint32_t getISAWriteIntRegister(const uint32_t reg) = 0;
	virtual uint32_t getISAWriteFPRegister(const uint32_t reg) = 0;

	virtual void setArchReadIntRegister(const uint32_t reg) = 0;
	virtual void setArchReadFPRegister(const uint32_t reg) = 0;
	virtual void setArchWriteIntRegister(const uint32_t reg) = 0;
	virtual void setArchWriteFPRegister(const uint32_t) = 0;

	virtual uint32_t getArchReadIntRegister(const uint32_t reg) = 0;
	virtual uint32_t getArchReadFPRegister(const uint32_t reg) = 0;
	virtual uint32_t getArchWriteIntRegister(const uint32_t reg) = 0;
	virtual uint32_t getArchWriteFPRegister(const uint32_t reg) = 0;

protected:
	const uint32_t id;
	SST::Output* output;
	VanadisInstructionError instErr;
	VanadisInstructionFamily family;
	uint32_t cyclesSpentExecuting;
	uint64_t addr;
        VanadisRegisterFile* intRF;
        VanadisRegisterFile* fpRF;
};

}
}

#endif
