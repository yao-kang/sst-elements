
#ifndef _H_SST_ELEMENTS_VANADIS_IC_READER_H
#define _H_SST_ELEMENTS_VANADIS_IC_READER_H

#include <sst/core/output.h>
#include <sst/core/link.h>
#include <sst/core/subcomponent.h>
#include <sst/core/interfaces/simpleMem.h>

#include <vector>

using namespace SST::Interfaces;

namespace SST {
namespace Vanadis {

class InstCacheReader : public SubComponent {

public:
	InstCacheReader(Component* parent, Params& params);
	~InstCacheReader();

	bool fill(const uint64_t ip, void* instBuffer, const uint64_t fillLen);
	void handleCacheResponse(SimpleMem::Request* resp);
	void initialize(const uint32_t core_id);

	void setSimpleMem(SimpleMem* newMem);

protected:
	void fillFromCurrentBuffer(const uint64_t ip, void* instBuffer, const uint64_t fillLen);
	void postReadsForNextBuffer();
	void rotateBuffers();

	SimpleMem* mem;
	std::vector<SimpleMem::Request*> readReqs;
	std::vector<SimpleMem::Request*> abandonedReqs;

	Output* output;
	int verbose;

	uint64_t currentBufferIP;
	uint64_t nextBufferIP;

	char* currentBuffer;
	char* nextBuffer;

	uint64_t bufferLength;
	uint64_t cacheLineSize;
	uint64_t cacheLinesPerBuffer;

	uint32_t coreID;
};

}
}

#endif
