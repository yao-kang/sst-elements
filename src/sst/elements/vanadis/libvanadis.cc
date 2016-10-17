#include <sst_config.h>
#include <sst/core/element.h>
#include <sst/core/subcomponent.h>

#include "icreader/icreader.h"
#include "vcore.h"

using namespace SST;
using namespace SST::Vanadis;

static SubComponent* load_InstCacheReader(Component* owner, Params& params) {
	return new InstCacheReader(owner, params);
};

static Component* load_VanadisCore(ComponentId_t id, Params& params) {
	return new VanadisCore(id, params);
};

static const ElementInfoParam instCacheReader_params[] = {
	{ "verbose",		"Sets verbosity of output",	"0" },
	{ NULL, NULL, NULL }
};

static const ElementInfoParam vcore_params[] = {
	{ "verbose",		"Sets verbostiy of output", 	"0" },
	{ "coreid", 		"Sets the ID of this core",     "0" },
	{ NULL, NULL, NULL }
};

static const ElementInfoSubComponent subcomponents[] = {
	{
		"InstCacheReader",
		"Reads from the instruction cache",
		NULL,
		load_InstCacheReader,
		instCacheReader_params,
		NULL,
		"SST::Vanadis::InstCacheReader"
	},
	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

static const ElementInfoComponent components[] = {
        {
                "BaseCPU",
                "Creates a base Miranda CPU ready to load an address generator",
                NULL,
                load_VanadisCore,
		vcore_params,
		NULL, // ports
                COMPONENT_CATEGORY_PROCESSOR,
		NULL // stats
        },
        { NULL, NULL, NULL, NULL, NULL, NULL, 0 }
};

extern "C" {
    ElementLibraryInfo vanadis_eli = {
        "vanadis",
        "Vanadis RISC Processor Core",
        components,
        NULL, // events
        NULL, // introspectors
        NULL, // modules
        subcomponents,
        NULL, // partitioners
        NULL, // python module generators
        NULL  // generators
    };
}
