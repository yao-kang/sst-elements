
#include <sst_config.h>
#include <sst/core/interfaces/simpleMem.h>

#include "vcore.h"

using namespace SST::Interfaces;
using namespace SST::Vanadis;

VanadisCore::VanadisCore(ComponentId_t id, Params& params) {

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

	icacheReader->setSimpleMem(icacheMem);

	const std::string cpuClock = params.find<std::string>("clock", "1GHz");
	output->verbose(CALL_INFO, 1, 0, "Core: %" PRIu32 " register clock at: %s\n",
		coreID, cpuClock.c_str());

	registerClock( cpuClock, new Clock::Handler<VanadisCore>(this, &VanadisCore::tick) );

	active = true;
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



	if(verbose) {
		output->verbose(CALL_INFO, 2, 0, "Core: %" PRIu32 " End-Tick: %" PRIu64 "\n",
			coreID, cycle);
	}

	return active;

}
