
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
	ip = params.find<uint64_t>("startip", 1024);

	std::string exePath = params.find<std::string>("exe", "");

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
		output->verbose(CALL_INFO, 2, 0, "Core: %" PRIu32 " Start-Tick: %" PRIu64 "\n",
			coreID, cycle);
	}

	uint32_t currentIns = 0;
	const bool fillSuccess = icacheReader->fill(ip, &currentIns, static_cast<uint64_t>(sizeof(currentIns)));

	if(fillSuccess) {
		ip += static_cast<uint64_t>(4);
	}

	if(ip >= 32768) {
		ip = 0;
	}

	if(verbose) {
		output->verbose(CALL_INFO, 2, 0, "Core: %" PRIu32 " End-Tick: %" PRIu64 "\n",
			coreID, cycle);
	}

	return false;

}
