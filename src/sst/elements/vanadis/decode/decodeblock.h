
#ifndef _H_SST_VANADIS_DECODE_BLOCK
#define _H_SST_VANADIS_DECODE_BLOCK

#include <sst/core/subcomponent.h>
#include <sst/core/output.h>

#include "decoderesponse.h"


namespace SST {
namespace Vanadis {

class VanadisDecodeBlock {

public:
	VanadisDecodeBlock(){};
	~VanadisDecodeBlock() {};

	virtual std::string getBlockName() = 0;

	void setOutput(SST::Output* newOut) {
		output = newOut;
	}

	virtual uint32_t getInstructionFamily() = 0;
	virtual VanadisDecodeResponse decode(const uint64_t ip, const uint32_t inst) = 0;

protected:
	SST::Output* output;

	void printInstruction(const uint64_t ip, const uint32_t inst) {
		char* instString = (char*) malloc( sizeof(char) * 33 );
		instString[32] = '\0';

		binaryStringize32(inst, instString);

		output->verbose(CALL_INFO, 2, 0, "INST: ip=%15" PRIu64 " | 0x%010" PRIx64 " : 0x%010" PRIx32 " | %s\n",
			ip, ip, inst, instString);
	}

	void decodeUJType(const uint32_t& inst, uint32_t& rd, uint64_t& imm) {
		//                                                  #####
		const uint32_t rd_mask       = 0b00000000000000000000111110000000;
		//
		const uint32_t immMSB_mask   = 0b10000000000000000000000000000000;
		//
		const uint32_t bit10_1_mask  = 0b01111111111000000000000000000000;
		//
		const uint32_t bit11_mask    = 0b00000000000100000000000000000000;
		//
		const uint32_t bit19_12_mask = 0b00000000000011111111000000000000;

		const uint64_t fullBits      = 0b1111111111111111111111111111111111111111111000000000000000000000;

		rd = (inst & rd_mask) >> 7;

		const uint32_t immTemp = ( (inst & bit10_1_mask)  >> 21 ) +
								 ( (inst & bit11_mask)    >> 9  ) +
								 ( (inst & bit19_12_mask)       ) +
								 ( (inst & immMSB_mask)   >> 11 );

		const uint32_t immMSBTemp = inst & immMSB_mask;

		imm = static_cast<uint64_t>(immTemp);

		if(immMSBTemp) {
			imm += fullBits;
		}
	}

	void decodeUType(const uint32_t& inst, uint32_t& rd, uint64_t& imm) {
		//                                                 #####
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		const uint32_t immMSB_mask = 0b10000000000000000000000000000000;
		//                                                 #####*******
		const uint32_t imm_mask    = 0b00000000000000000000111111111111;
		const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111100000000000000000000;

		rd = (inst & rd_mask) >> 7;

		const uint32_t immTemp = (inst & imm_mask) >> 12;
		const uint32_t immMSBTemp = (inst & immMSB_mask);

		if(immMSBTemp) {
			imm += fullBits;
		}
	}

	void decodeSType(const uint32_t& inst, uint32_t& rs1, uint32_t& rs2, uint64_t& imm) {
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                                    *****#####***#####*******
		const uint32_t rs2_mask    = 0b00000001111100000000000000000000;
		//                                    *****#####***#####*******
		const uint32_t immL_mask   = 0b00000000000000000000111110000000;
		//                                    *****#####***#####*******
		const uint32_t immU_mask   = 0b11111110000000000000000000000000;
		const uint32_t immMSB_mask = 0b10000000000000000000000000000000;
		const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111111111111000000000000;

		rs1 = (inst & rs1_mask) >> 15;
		rs2 = (inst & rs2_mask) >> 20;

		const uint32_t immTemp = ((inst & immL_mask) >> 7) + ((inst & immU_mask) >> 18);

		imm = static_cast<uint64_t>(immTemp);

		if(immTemp) {
			imm += fullBits;
		}
	}

    void decodeSBType(const uint32_t& inst, uint32_t& rs1, uint32_t& rs2, uint64_t& imm) {
        //                                         #####***#####*******
        const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
        //                                    *****#####***#####*******
        const uint32_t rs2_mask    = 0b00000001111100000000000000000000;
        //                                    *****#####***#####*******
        const uint32_t imm11_mask  = 0b00000000000000000000000010000000;
        //                                    *****#####***#####*******
        const uint32_t imm4_mask   = 0b00000000000000000000111100000000;
        //                                    *****#####***#####*******
        const uint32_t imm12_mask  = 0b10000000000000000000000000000000;
        //                                    *****#####***#####*******
        const uint32_t imm10_mask  = 0b01111110000000000000000000000000;


        const uint32_t immMSB_mask =                                 0b10000000000000000010000000000000;
        const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111111111110000000000000;

        rs1 = (inst & rs1_mask) >> 15;
        rs2 = (inst & rs2_mask) >> 20;

        const uint32_t immTemp = ((inst & imm4_mask) >> 6) + ((inst & imm10_mask) >> 19) +
            ((inst & imm11_mask) << 4) + ((inst & imm12_mask) >> 18);

        imm = static_cast<uint64_t>(immTemp);

        if(immTemp & immMSB_mask) {
            imm += fullBits;
        }
    }

	void decodeRType(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint32_t& rs2) {
		//                                              ***#####*******
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                                    *****#####***#####*******
		const uint32_t rs2_mask    = 0b00000001111100000000000000000000;


		rd  = (inst & rd_mask)  >> 7;
		rs1 = (inst & rs1_mask) >> 15;
		rs2 = (inst & rs2_mask) >> 20;
	}

	void decodeRType(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint32_t& rs2, uint32_t roundMode) {
		//                                              ***#####*******
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                                    *****#####***#####*******
		const uint32_t rs2_mask    = 0b00000001111100000000000000000000;
		//
		const uint32_t rm_mask     = 0b00000000000000000111000000000000;

		rd  = (inst & rd_mask)  >> 7;
		rs1 = (inst & rs1_mask) >> 15;
		rs2 = (inst & rs2_mask) >> 20;
		roundMode = (inst & rm_mask) >> 12;
	}

	void decodeR4Type(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint32_t& rs2, uint32_t& rs3) {
		//                                              ***#####*******
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                                    *****#####***#####*******
		const uint32_t rs2_mask    = 0b00000001111100000000000000000000;
		//                             *****##*****#####***#####*******
		const uint32_t rs3_mask    = 0b11111000000000000000000000000000;

		rd  = (inst & rd_mask)  >> 7;
		rs1 = (inst & rs1_mask) >> 15;
		rs2 = (inst & rs2_mask) >> 20;
		rs3 = (inst & rs3_mask) >> 27;
	}

	void decodeR4Type(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint32_t& rs2, uint32_t& rs3, uint32_t roundMode) {
		//                                              ***#####*******
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                                    *****#####***#####*******
		const uint32_t rs2_mask    = 0b00000001111100000000000000000000;
		//                             *****##*****#####***#####*******
		const uint32_t rs3_mask    = 0b11111000000000000000000000000000;
		//
		const uint32_t rm_mask     = 0b00000000000000000111000000000000;

		rd  = (inst & rd_mask)  >> 7;
		rs1 = (inst & rs1_mask) >> 15;
		rs2 = (inst & rs2_mask) >> 20;
		rs3 = (inst & rs3_mask) >> 27;
		roundMode = (inst & rm_mask) >> 12;
	}

	void decodeIType(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint64_t& imm, uint32_t roundMode) {
			//                                              ***#####*******
			const uint32_t rd_mask     = 0b00000000000000000000111110000000;
			//                                         #####***#####*******
			const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
			//                             ????????????#####***#####*******
			const uint32_t imm_mask    = 0b11111111111100000000000000000000;
			const uint32_t immMSB_mask = 0b10000000000000000000000000000000;
			//                                                 ????????????
			const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111111111111000000000000;
			//
			const uint32_t rm_mask     = 0b00000000000000000111000000000000;

			rd  = (inst & rd_mask)  >> 7;
			rs1 = (inst & rs1_mask) >> 15;
			roundMode = (inst & rm_mask) >> 12;

			const uint32_t immTemp    = (inst & imm_mask);
			const uint32_t immMSBTemp = (inst & immMSB_mask);

			imm = static_cast<uint64_t>(immTemp);

			// Do we need to sign extend the immediate or not?
			if(immMSBTemp) {
				imm += fullBits;
			}
	}

	void decodeIType(const uint32_t& inst, uint32_t& rd, uint32_t& rs1, uint64_t& imm) {
		//                                              ***#####*******
		const uint32_t rd_mask     = 0b00000000000000000000111110000000;
		//                                         #####***#####*******
		const uint32_t rs1_mask    = 0b00000000000011111000000000000000;
		//                             ????????????#####***#####*******
		const uint32_t imm_mask    = 0b11111111111100000000000000000000;
		const uint32_t immMSB_mask = 0b10000000000000000000000000000000;
		//                                                 ????????????
		const uint64_t fullBits    = 0b1111111111111111111111111111111111111111111111111111000000000000;

		rd  = (inst & rd_mask)  >> 7;
		rs1 = (inst & rs1_mask) >> 15;

		const uint32_t immTemp    = (inst & imm_mask);
		const uint32_t immMSBTemp = (inst & immMSB_mask);

		imm = static_cast<uint64_t>(immTemp);

		// Do we need to sign extend the immediate or not?
		if(immMSBTemp) {
			imm += fullBits;
		}
	}


};

}
}

#endif
