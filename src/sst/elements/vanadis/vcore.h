

#ifndef _H_SST_ELEMENTS_VANADIS_CPU_
#define _H_SST_ELEMENTS_VANADIS_CPU_

#include <sst_config.h>

#include <sst/core/output.h>
#include <sst/core/component.h>
#include <sst/core/params.h>

#include "icreader/icreader.h"
#include "elf/elfreader.h"

namespace SST {
namespace Vanadis {

class VanadisCore : public SST::Component {

public:
	VanadisCore(ComponentId_t id, Params& params);
	void init(unsigned int phase);
	~VanadisCore();

protected:
	VanadisCore(const VanadisCore&);
	void operator=(const VanadisCore&);
	void copyData(const char* src, char* dest, const size_t len);

	bool tick( SST::Cycle_t );

	InstCacheReader* icacheReader;
	SimpleMem* icacheMem;
	Clock::HandlerBase* clockHandler;

	uint64_t ip;

	int verbose;
	bool active;
	uint32_t coreID;
	Output* output;
	std::string exePath;

};

}
}

#endif
