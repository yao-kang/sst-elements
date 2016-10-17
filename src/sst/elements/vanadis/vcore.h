

#ifndef _H_SST_ELEMENTS_VANADIS_CPU_
#define _H_SST_ELEMENTS_VANADIS_CPU_

#include <sst/core/output.h>
#include <sst/core/component.h>
#include <sst/core/params.h>

#include "icreader/icreader.h"

namespace SST {
namespace Vanadis {

class VanadisCore : public SST::Component {

public:
	VanadisCore(ComponentId_t id, Params& params);
	~VanadisCore();

	bool tick( SST::Cycle_t );

protected:
	InstCacheReader* icacheReader;
	int verbose;
	bool active;
	uint32_t coreID;
	Output* output;

};

}
}

#endif
