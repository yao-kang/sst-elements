
#include <sst_config.h>
#include "vcore.h"

VanadisCore::VanadisCore(ComponentId_t id, Params& params) {

	verbose = params.find<int>("verbose", 0);
	coreID  = params.find<uint32_t>("coreid", 0);

	char* outputPrefix = new char[256];
	sprintf(outputPrefix, "VanadisCore[%" PRIu32 "][@f:@l:@p]: ", coreID);
	output = new SST::Output(outputPrefix, verbose, 0, SST::Output::STDOUT);

	std::string clock = params.find<std::string>("clock", "1.0GHz");
	output->verbose(CALL_INFO, 1, 0, "Core: %" PRIu32 " register clock at: %s\n",
		coreID, clock.c_str());

	registerClock( clock, new Clock::Handler<VanadisCore>(this, &VanadisCore::tick) );
	active = true;
}

VanadisCore::~VanadisCore() {
	if(NULL != output) {
		delete output;
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
