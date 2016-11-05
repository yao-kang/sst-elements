
#include <sst_config.h>
#include <sst/core/interfaces/simpleMem.h>

#include "vcore.h"

using namespace SST::Interfaces;
using namespace SST::Vanadis;

VanadisCore::VanadisCore(ComponentId_t id, Params& params) :
	Component(id) {

	verbose = params.find<int>("verbose", 0);
	coreID  = params.find<uint32_t>("coreid", 0);

	char* outputPrefix = new char[256];
	sprintf(outputPrefix, "VanadisCore[%" PRIu32 "][@f:@l:@p]: ", coreID);
	output = new SST::Output(outputPrefix, verbose, 0, SST::Output::STDOUT);

	std::string icacheRdr = params.find<std::string>("icachereader", "vanadis.InstCacheReader");

	output->verbose(CALL_INFO, 1, 0, "Loading instruction cache reader subcomponent (\"%s\")...\n", icacheRdr.c_str());

	Params iRdrParams = params.find_prefix_params("icachereader.");
	icacheReader = dynamic_cast<InstCacheReader*>( loadSubComponent(icacheRdr, this, iRdrParams) );

	if(NULL == icacheReader) {
		output->fatal(CALL_INFO, -1, "Unable to load instruction cache reader subcomponent: %s\n",
			icacheRdr.c_str());
	} else {
		output->verbose(CALL_INFO, 1, 0, "Load instruction cache reader successful.\n");
	}

	std::string icacheMemInt = params.find<std::string>("icachememiface", "memHierarchy.memInterface");
	output->verbose(CALL_INFO, 1, 0, "Loading instruction cache memory interface: \"%s\" ...\n",
		icacheMemInt.c_str());

	Params icacheIFaceParams = params.find_prefix_params("icachememifaceparams.");
	icacheMem = dynamic_cast<SimpleMem*>( loadModuleWithComponent(icacheMemInt, this, icacheIFaceParams) );

	if(NULL == icacheMem) {
		output->fatal(CALL_INFO, -1, "Error: unable to load instruction cache interface: %s\n",
			icacheMemInt.c_str());
	} else {
		output->verbose(CALL_INFO, 1, 0, "Loading instruction cache interface successfully\n");
	}

	const bool icacheWireupSuccess = icacheMem->initialize("icache_link",
		new SimpleMem::Handler<InstCacheReader>(icacheReader, &InstCacheReader::handleCacheResponse) );

	if(! icacheWireupSuccess) {
		output->fatal(CALL_INFO, -1, "Error: unable to write up icache_link for instruction cache reading\n");
	}

	icacheReader->setSimpleMem(icacheMem);

	const std::string cpuClock = params.find<std::string>("clock", "1 GHz");
	output->verbose(CALL_INFO, 1, 0, "Core: %" PRIu32 " register clock at: %s\n",
		coreID, cpuClock.c_str());

	clockHandler = new Clock::Handler<VanadisCore>(this, &VanadisCore::tick);
	registerClock( cpuClock, clockHandler );

	active = true;
	ip = params.find<uint64_t>("startip", 0);

	exePath = params.find<std::string>("exe", "");

	if("" == exePath) {
		output->fatal(CALL_INFO, -1, "Executable was not specified! Please add the \"exe\" parameter\n");
	} else {
		output->verbose(CALL_INFO, 1, 0, "Executable: \"%s\"\n", exePath.c_str());
	}

	if( 0 == coreID ) {
		output->verbose(CALL_INFO, 1, 0, "Opening %s for ELF reading...\n", exePath.c_str());
		ELFDefinition* elfInfo = ELFDefinition::readObject(exePath, output);
		output->verbose(CALL_INFO, 1, 0, "ELF read complete, analyzing...\n");

		if(NULL == elfInfo) {
			output->fatal(CALL_INFO, 1, 0, "Unable to successfully read ELF for: %s\n", exePath.c_str());
		}

		output->verbose(CALL_INFO, 1, 0, "ELF Information (%s)\n", exePath.c_str());
		output->verbose(CALL_INFO, 1, 0, "-> Binary Class:      %s\n",
			(elfInfo->getELFClass() == BIT_32 ? "32-bit" : "64-bit"));
		output->verbose(CALL_INFO, 1, 0, "-> Endian:            %s\n",
			(elfInfo->getELFEndian() == ENDIAN_LITTLE) ? "Little-Endian" : "Big-Endian");
		output->verbose(CALL_INFO, 1, 0, "-> Start Address:     0x%" PRIx64 "\n",
			elfInfo->getEntryPoint());
			
		output->verbose(CALL_INFO, 1, 0, "Setting instruction pointer of core 0 to: 0x%" PRIx64 "...\n",
			elfInfo->getEntryPoint());

		ip = elfInfo->getEntryPoint();
	}
}

void VanadisCore::copyData(const char* src, char* dest, const size_t len) {
	for(size_t i = 0; i < len; ++i) {
		dest[i] = src[i];
	}
	
	for(size_t i = 0; i < 5; ++i) {
		printf("%c|%c ", src[i], dest[i]);
	}
	
	printf("\n");
} 

void VanadisCore::init(unsigned int phase) {

	output->verbose(CALL_INFO, 1, 0, "Init: CoreID: %" PRIu32 " Phase: %" PRIu32 "\n",
		coreID, static_cast<uint32_t>(phase));

	if( 0 == coreID && 0 == phase) {
		if( "" != exePath ){
			// Read in a populate our memory
			FILE* readExe = fopen(exePath.c_str(), "r");
		
			if(NULL == readExe) {
				output->fatal(CALL_INFO, -1, "Error: unable to read: \'%s\' for loading into memory.\n",
					exePath.c_str());
			}
		
			// Seek at end of the file, lets see how large the executable is
			fseek(readExe, 0, SEEK_END);
		
			long exeLength = ftell(readExe);
		
			if(exeLength == 0) {
				output->fatal(CALL_INFO, -1, "Error: executable (%s) is zero length.\n",
					exePath.c_str());
			} else {
				output->verbose(CALL_INFO, 1, 0, "Executable: %s will be read into the system memory for simulation.\n",
					exePath.c_str());
				output->verbose(CALL_INFO, 1, 0, "Executable length: %" PRIu64 " bytes.\n",
					static_cast<uint64_t>(exeLength));
			}
		
			// Reset file pointer back to zero
			fseek(readExe, 0, SEEK_SET);
		
			std::vector<uint8_t> exeBinary;
			exeBinary.reserve(exeLength);

			size_t objRead = fread( (char*) &exeBinary[0], 1,
				static_cast<size_t>(exeLength), readExe);

			if( objRead != static_cast<size_t>(exeLength) ) {
				output->fatal(CALL_INFO, -1, "Error reading binary (%s) expected length: %" PRIu64 ", bytes actually read: %" PRIu64 "\n",
					exePath.c_str(), static_cast<uint64_t>(exeLength), static_cast<uint64_t>(objRead));
			} else {
				output->verbose(CALL_INFO, 1, 0, "Executable binary read successfully from disk, closing file handles.\n");
			}

			output->verbose(CALL_INFO, 1, 0, "Creating huge memory write for executable...\n");

			// One single huge  request for the system to put the binary into memory
			SimpleMem::Request* writeExe = new SimpleMem::Request(SimpleMem::Request::Write,
				0, exeLength, exeBinary);

			output->verbose(CALL_INFO, 1, 0, "Done creating huge executable for memory write, will now send to the caches...\n");

			icacheMem->sendInitData(writeExe);

			output->verbose(CALL_INFO, 1, 0, "Sent to instruction caches for memory update.\n");

			fclose(readExe);
			output->verbose(CALL_INFO, 1, 0, "Init for Vanadis core is complete.\n");
		}
	}
}

VanadisCore::~VanadisCore() {
	if(NULL != output) {
		delete output;
	}

	if(NULL != icacheReader) {
		delete icacheReader;
	}
}

bool VanadisCore::tick( SST::Cycle_t cycle ) {

	if(verbose) {
		output->verbose(CALL_INFO, 2, 0, "Core: %" PRIu32 " Start-Tick: %" PRIu64 ", ip=%" PRIu64 "\n",
			coreID, cycle, ip);
	}

	uint32_t currentIns = 0;
	const bool fillSuccess = icacheReader->fill(ip, &currentIns, static_cast<uint64_t>(sizeof(currentIns)));

	if(fillSuccess) {
		ip += static_cast<uint64_t>(4);

		if(verbose) {
			output->verbose(CALL_INFO, 2, 0, "Core: %" PRIu32 " End-Tick: %" PRIu64 ", instruction=%" PRIu32 "\n",
				coreID, cycle, currentIns);
		}
	} else {
		if(verbose) {
			output->verbose(CALL_INFO, 2, 0, "Fill failed, will have to wait.\n");
		}
	}

	return false;

}
