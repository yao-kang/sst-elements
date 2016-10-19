
#ifndef _H_SST_VANADIS_CORE_ELF_READ_
#define _H_SST_VANADIS_CORE_ELF_READ_

namespace SST {
namespace Vanadis {

#define VANADIS_ELF_IDENT_SIZE 16

enum ELFObjectType {
	NONE,
	RELOCATABLE,
	EXECUTABLE,
	SHARED_OBJECT,
	CORE
};

enum ELFObjectMachineType {
	RISCV,
	UNKNOWN
};

enum ELFObjectClass {
	BIT_32,
	BIT_64
};

enum ELFObjectEndianness {
	ENDIAN_LITTLE,
	ENDIAN_BIG
};

class ELFDefinition {

public:
	ELFDefinition() {}
	~ELFDefinition() {}

	static ELFDefinition* readObject(std::string elfObjPath, SST::Output* output) {
		ELFDefinition* def = new ELFDefinition();
		def->objPath = elfObjPath;

		output->verbose(CALL_INFO, 1, 0, "Opening: %s...\n", elfObjPath.c_str());
		FILE* objFile = fopen(elfObjPath.c_str(), "rb");

		if(NULL == objFile) {
			output->verbose(CALL_INFO, 1, 0, "Error: unable to open file for ELF reading.\n");
			return NULL;
		}

		output->verbose(CALL_INFO, 1, 0, "Reading ELF header...\n");

		unsigned char* elfHeader = new unsigned char[VANADIS_ELF_IDENT_SIZE];
		size_t bytesRead = fread(elfHeader, VANADIS_ELF_IDENT_SIZE, 1, objFile);

		if(bytesRead != 1) {
			output->verbose(CALL_INFO, 1, 0, "Error: unable to read ELF header, did not get 16 bytes of head from a fread.\n");
			return NULL;
		}

		if( elfHeader[0] == 0x7F &&
		    elfHeader[1] == 0x45 &&
		    elfHeader[2] == 0x4c &&
		    elfHeader[3] == 0x46 ) {

			output->verbose(CALL_INFO, 1, 0, "ELF header recognized successfully\n");
			// Passes ELF magic number check!
		} else {
			output->verbose(CALL_INFO, 1, 0, "Error: ELF header was not recognized when reading the ELF information\n");
			return NULL;
		}

		if( elfHeader[4] == 0x01 ) {
			def->objClass = BIT_32;
		} else if (elfHeader[4] == 0x02) {
			def->objClass = BIT_64;
		} else {
			output->verbose(CALL_INFO, 1, 0, "Error: unknown ELF object class.\n");
			return NULL;
		}

		if( elfHeader[5] == 0x01 ) {
			def->objEndian = ENDIAN_LITTLE;
		} else if( elfHeader[5] == 0x02 ) {
			def->objEndian = ENDIAN_BIG;
		} else {
			output->verbose(CALL_INFO, 1, 0, "Error: unknown ELF endian type.\n");
			return NULL;
		}

		fclose(objFile);

		return def;
	}

//	ELFObjectType getObjectType() const { return objType; }
//	ELFObjectMachineType getObjectMachineType() const { return objMachineType; }
//	uint64_t getEntryPoint() const { return objEntryAddress; }

	std::string getObjectPath() const { return objPath; }
	ELFObjectClass getELFClass() const { return objClass; }
	ELFObjectEndianness getELFEndian() const { return objEndian; }

protected:
	std::string objPath;

	ELFObjectClass objClass;
	ELFObjectEndianness objEndian;

	uint16_t objType;
	uint16_t objMachineType;
	uint32_t objHeaderVersion;
	uint64_t objEntryAddress;

};

}
}

#endif
