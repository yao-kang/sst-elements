
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
			output->verbose(CALL_INFO, 1, 0, "Error: unknown ELF object class (not 32-bit or 64-bit).\n");
			return NULL;
		}

		if( elfHeader[5] == 0x01 ) {
			def->objEndian = ENDIAN_LITTLE;
		} else if( elfHeader[5] == 0x02 ) {
			def->objEndian = ENDIAN_BIG;
		} else {
			output->verbose(CALL_INFO, 1, 0, "Error: unknown ELF endian type (not LSB or MSB).\n");
			return NULL;
		}
		
		/*
		    uint16_t      e_type;
               uint16_t      e_machine;
               uint32_t      e_version;
               ElfN_Addr     e_entry;
               ElfN_Off      e_phoff;
               ElfN_Off      e_shoff;
               uint32_t      e_flags;
               uint16_t      e_ehsize;
               uint16_t      e_phentsize;
               uint16_t      e_phnum;
               uint16_t      e_shentsize;
               uint16_t      e_shnum;
               uint16_t      e_shstrndx;
        */
        
        bytesRead = fread(&def->objType, sizeof(def->objType), 1, objFile);
        
        if(bytesRead != 1) {
        	output->verbose(CALL_INFO, 1, 0, "Error: unable to read ELF object type, did not get uint16_t from read\n");
        	return NULL;
        }
        
        bytesRead = fread(&def->objMachineType, sizeof(def->objMachineType), 1, objFile);
        
        if(bytesRead != 1) {
        	output->verbose(CALL_INFO, 1, 0, "Error: unable to read ELF object machine type, did not get uint16_t from read\n");
        	return NULL;
        }
        
        bytesRead = fread(&def->objHeaderVersion, sizeof(def->objHeaderVersion), 1, objFile);
        
        if(bytesRead != 1) {
        	output->verbose(CALL_INFO, 1, 0, "Error: unable to read ELF object version, did not get a uint32_t from read\n");
        	return NULL;
        }
        
        bytesRead = fread(&def->objEntryAddress, sizeof(def->objEntryAddress), 1, objFile);
        
        if(bytesRead != 1) {
        	output->verbose(CALL_INFO, 1, 0, "Error: unable to read the ELF start address for the binary, expected a uint64_t\n");
        	return NULL;
        }

		fclose(objFile);

		return def;
	}

//	ELFObjectType getObjectType() const { return objType; }
//	ELFObjectMachineType getObjectMachineType() const { return objMachineType; }
	
	uint64_t getEntryPoint() const { return objEntryAddress; }

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
