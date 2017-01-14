
#ifndef _H_SST_VANADIS_INT_REGISTER_FILE
#define _H_SST_VANADIS_INT_REGISTER_FILE

#include <deque>

namespace SST {
namespace Vanadis {

class VanadisRegisterFile {

public:
	VanadisRegisterFile(int regWidth, int physCount) :
		registerWidth(regWidth), physRefCount(physCount) {

	}

	int getRegisterWidth() {
		return registerWidth;
	}

	int countPhysicalRegisters() {
		return physRegCount;
	}

				template<typename T>
        void setReg(uint32_t reg, T val) {
            if( sizeof(T) > registerWidth ) {
							// Error?
						}
        }

				template<typename T>
        T getReg(uint32_t reg) {
            if( sizeof(T) > registerWidth ) {
							// Error?
						}
        }

protected:
	int registerWidth;
	int physRegCount;

	std::deque<>

};

}
}

#endif
