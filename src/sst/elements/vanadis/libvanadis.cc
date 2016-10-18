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
 	{ "exe",                "Sets the exectuable to be simulated", "" },
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

static const ElementInfoPort vcore_ports[] = {
        { "icache_link",      	"Link to instruction cache", NULL },
        { NULL, NULL, NULL }
};

static const ElementInfoComponent components[] = {
        {
                "VanadisCore",
                "Creates a processor core for Vanadis",
                NULL,
                load_VanadisCore,
		vcore_params,
		vcore_ports,
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
