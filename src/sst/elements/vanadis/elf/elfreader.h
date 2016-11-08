
#ifndef _H_SST_VANADIS_CORE_ELF_READ_
#define _H_SST_VANADIS_CORE_ELF_READ_

namespace SST {
namespace Vanadis {

#define VANADIS_ELF_IDENT_SIZE 16

typedef struct {
	unsigned char ident[VANADIS_ELF_IDENT_SIZE];
	uint16_t      type;
	uint16_t      machine;
	uint32_t      version;
	uint64_t      entryPoint;
	uint64_t      prgHeaderOffset;
	uint64_t      shoff;
	uint32_t      flags;
	uint16_t      ehsize;
	uint16_t      prgHeaderEntrySize;
	uint16_t      prgHeaderEntries;
	uint16_t      shentsize;
	uint16_t      shnum;
	uint16_t      shstrndx;
} ELFHeader;

typedef struct {
	uint32_t   	  type;
	uint32_t   	  flags;
	uint64_t      offset;
	uint64_t      virtAddress;
	uint64_t      physAddress;
	uint64_t      filesz;
	uint64_t      memsz;
	uint64_t      align;
} ELFProgramHeader;

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

		size_t objRead = fread(&def->header, sizeof(header), 1, objFile);

		if(objRead != 1) {
			output->verbose(CALL_INFO, 1, 0, "Error: unable to read ELF header from executable.\n");
			return NULL;
		} else {
			output->verbose(CALL_INFO, 1, 0, "ELF Header data was read successfully.\n");
		}

		if( def->header.ident[0] == 0x7F &&
			def->header.ident[1] == 0x45 &&
		    def->header.ident[2] == 0x4c &&
		    def->header.ident[3] == 0x46 	) {

			output->verbose(CALL_INFO, 1, 0, "ELF header recognized successfully\n");
			// Passes ELF magic number check!
		} else {
			output->verbose(CALL_INFO, 1, 0, "Error: ELF header was not recognized when reading the ELF information\n");
			return NULL;
		}

		
		if( def->header.ident[4] == 0x01 ) {
			def->objClass = BIT_32;
		} else if (def->header.ident[4] == 0x02) {
			def->objClass = BIT_64;
		} else {
			output->verbose(CALL_INFO, 1, 0, "Error: unknown ELF object class (not 32-bit or 64-bit).\n");
			return NULL;
		}

		if( def->header.ident[5] == 0x01 ) {
			def->objEndian = ENDIAN_LITTLE;
		} else if( def->header.ident[5] == 0x02 ) {
			def->objEndian = ENDIAN_BIG;
		} else {
			output->verbose(CALL_INFO, 1, 0, "Error: unknown ELF endian type (not LSB or MSB).\n");
			return NULL;
		}
		
		const uint64_t prgHeaderOffset = def->getProgramHeaderOffset();
		const uint64_t prgHeaderSize   = static_cast<uint64_t>(def->getProgramHeaderEntrySize());
		const uint16_t prgHeaderCount  = def->getProgramHeaderEntryCount();
		
		if( sizeof(ELFProgramHeader) != prgHeaderSize ) {
			output->fatal(CALL_INFO, -1, "Error: Program Header Size: %" PRIu64 " != Structure Size: %" PRIu64 "\n",
				prgHeaderSize, static_cast<uint64_t>(sizeof(ELFProgramHeader)));
		}
		
		def->programHeaders = (ELFProgramHeader*) malloc( prgHeaderSize * prgHeaderCount );
		
		fseek(objFile, prgHeaderOffset, SEEK_SET);
		
		const size_t headerObj = fread(def->programHeaders, prgHeaderSize, prgHeaderCount, objFile);
		
		if( headerObj != prgHeaderCount ) {
			output->fatal(CALL_INFO, -1, "Error: reading program header failed, wanted to read: %" PRIu16 " headers, but read: %" PRIu64 "\n",
				prgHeaderCount, headerObj);
		} else {
			output->verbose(CALL_INFO, 1, 0, "Successfully read %" PRIu16 " program headers from file.\n",
				prgHeaderCount);
		}

		fclose(objFile);

		return def;
	}

//	ELFObjectType getObjectType() const { return objType; }
//	ELFObjectMachineType getObjectMachineType() const { return objMachineType; }
	
	uint64_t 			getEntryPoint() const { return header.entryPoint; }
	uint64_t			getProgramHeaderOffset() const { return header.prgHeaderOffset; }
	uint16_t			getProgramHeaderEntryCount() const { return header.prgHeaderEntries; }
	uint16_t			getProgramHeaderEntrySize() const { return header.prgHeaderEntrySize; }
	
	std::string 		getObjectPath() const { return objPath; }
	ELFObjectClass 		getELFClass() const { return objClass; }
	ELFObjectEndianness getELFEndian() const { return objEndian; }

	ELFProgramHeader*   getELFProgramHeader(uint16_t index) { return &programHeaders[index]; }

protected:
	std::string objPath;
	
	ELFHeader header;
	ELFProgramHeader* programHeaders;

	ELFObjectClass objClass;
	ELFObjectEndianness objEndian;

	uint16_t objMachineType;
	uint32_t objHeaderVersion;

};

}
}

#endif
