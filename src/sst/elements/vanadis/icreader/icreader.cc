
#include <sst_config.h>
#include "icreader.h"

using namespace SST::Vanadis;

InstCacheReader::InstCacheReader(Component* parent, Params& params) :
	SubComponent(parent) {

	coreID          = 0;

	verbose         = params.find<int>("verbose", 0);
	output          = new Output("InstCacheReader[@f:@l:@p]: ",
				verbose, 0, SST::Output::STDOUT);

	currentBufferIP = 0;
	nextBufferIP    = 0;

	cacheLineSize   = params.find<uint64_t>("cachelinesize", 64);
	bufferLength    = params.find<uint64_t>("bufferlength", cacheLineSize);

	currentBuffer   = new char[bufferLength];
	nextBuffer      = new char[bufferLength];

	// Buffer Length must be muliple of cache line size
	// assert(bufferLength % cacheLineSize == 0);
	if( 0 != (bufferLength % cacheLineSize) ) {
		output->fatal(CALL_INFO, -1, "Error: instruction buffer must be a multiple of cache line size. Buffer Length=%" PRIu64 ", cache line size=%" PRIu64 "\n",
			bufferLength, cacheLineSize);
	}

	cacheLinesPerBuffer = bufferLength / cacheLineSize;

	// Ensure our buffers are up to size
	readReqs.reserve(cacheLinesPerBuffer);
	abandonedReqs.reserve(cacheLinesPerBuffer * 2);

	//Params interfaceParams = params.find_prefix_params("icacheparams.");
	//mem = dynamic_cast<SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, interfaceParams));
	//
	//if(NULL == mem) {
	//	output->fatal(CALL_INFO, -1, "Error: unable to initialize instruction cache Simple Mem interface.\n");
	//}
	//
	//bool init_success = mem->initialize("icache_link", new SimpleMem::Handler<InstCacheReader>(
        //        this, &InstCacheReader::handleCacheResponse) );
	//
	//if(init_success) {
	//	output->verbose(CALL_INFO, 1, 0, "Simple Mem initialization for instruction cache completed successfully.\n");
	//} else {
	//	output->fatal(CALL_INFO, -1, "Error: unable to initialize Simple Mem for instruction cache.\n");
	//}

	mem = NULL;

	if(verbose) {
		output->verbose(CALL_INFO, 1, 0, "Instruction Cache Reader Parameters:\n");
		output->verbose(CALL_INFO, 1, 0, "- Cache Line Size:   %" PRIu64 "\n", cacheLineSize);
		output->verbose(CALL_INFO, 1, 0, "- Buffer Length:     %" PRIu64 "\n", bufferLength);
	}
}

void InstCacheReader::initialize(const uint32_t core_id) {
	coreID = core_id;

	char* linkBuffer = new char[256];
	sprintf(linkBuffer, "icache_link_%" PRIu32, coreID);

	mem->initialize(linkBuffer, new SimpleMem::Handler<InstCacheReader>(
		this, &InstCacheReader::handleCacheResponse));

	delete[] linkBuffer;
}

InstCacheReader::~InstCacheReader() {
	if(NULL != currentBuffer) {
		delete[] currentBuffer;
	}

	if(NULL != nextBuffer) {
		delete[] nextBuffer;
	}

	if(NULL != mem) {
		delete mem;
	}

	output->verbose(CALL_INFO, 1, 0, "Instruction Cache Reader shutdown complete.\n");

	if(NULL != output) {
		delete output;
	}
}

void InstCacheReader::handleCacheResponse(SimpleMem::Request* resp) {

	bool found = false;

	for(auto abandonReqItr = abandonedReqs.begin(); abandonReqItr != abandonedReqs.end(); abandonReqItr++) {
		if( (*abandonReqItr)->id == resp->id ) {
			abandonedReqs.erase(abandonReqItr);
			found = true;
			break;
		}
	}

	if(found) {
		return;
	}

	const uint64_t offset = resp->addr - nextBufferIP;

	// This shouldn't happen but we are still debugging code so lets keep it
	// as a sanity check
	if( (offset + resp->size) > (nextBufferIP + bufferLength) ) {
		output->fatal(CALL_INFO, -1, "Error: offset (%" PRIu64 ") + RespSize (%" PRIu64 ") exceeds buffer size for nextBufferIP=%" PRIu64 ", BuffLen=%" PRIu64 "\n",
			offset, static_cast<uint64_t>(resp->size), nextBufferIP, bufferLength);
	}
	
	for(int i = 0; i < resp->size; i++) {
		printf("[%d]=%d ", i, (int) resp->data[i]);
	}
	
	printf("\n");

	char* dataVecPtr = (char*) &(resp->data[0]);
	for(size_t i = 0; i < resp->size; ++i) {
		nextBuffer[offset + i] = dataVecPtr[i];
	}

	for(auto readReqItr = readReqs.begin() ; readReqItr != readReqs.end() ; readReqItr++) {
		if( (*readReqItr)->id == resp->id ) {
			readReqs.erase(readReqItr);
			found = true;
			break;
		}
	}

	if(!found) {
		output->fatal(CALL_INFO, -1, "Error: unable to find a match for response from instruction cache in pending reads.\n");
	}

	// Clear up the memory now we are done
	delete resp;
}

void InstCacheReader::postReadsForNextBuffer() {
	// Step 1: any pending reads have to be moved to the abandoned read
	// case so we can ensure responses are matched and retired correctly
	for(int i = 0; i < readReqs.size(); ++i) {
		abandonedReqs.push_back(readReqs[i]);
	}

	// Step 2: clear out pending requests. We are starting new ones now
	readReqs.clear();

	if(verbose) {
		output->verbose(CALL_INFO, 4, 0, "Posting reads to fill instruction buffer:\n");
	}

	// Step 3: create one read request per cache line which will fit into
	// the buffer
	for(uint64_t i = 0; i < cacheLinesPerBuffer; ++i) {
		SimpleMem::Request* readLine = new SimpleMem::Request(
			SimpleMem::Request::Read,
			nextBufferIP + (i * cacheLineSize),
			cacheLineSize);

		// Record the read in our pending transactions
		readReqs.push_back(readLine);

		// Send the event to the cache handler
		mem->sendRequest(readLine);
	}
}

void InstCacheReader::rotateBuffers() {
	// Swap over the buffer addresses
	char* tmpBuffer = currentBuffer;
	currentBuffer   = nextBuffer;
	nextBuffer      = tmpBuffer;

	currentBufferIP = nextBufferIP;
	nextBufferIP    = currentBufferIP + bufferLength;

	if(verbose) {
		output->verbose(CALL_INFO, 8, 0, "Buffer rotation, currentBufferIP set to %" PRIu64 ", nextBufferIP=%" PRIu64 "\n", currentBufferIP, nextBufferIP);
	}

	// Tell the cache reader we want to post a set of reads
	// to populate the nextBuffer ready for reading in a few cycles
	postReadsForNextBuffer();
}

void InstCacheReader::fillFromCurrentBuffer(const uint64_t ip, void* instBuffer,
        const uint64_t fillLen) {

	const uint64_t offset = ip - currentBufferIP;
	char* instBufferChar  = static_cast<char*>(instBuffer);

        for(uint64_t i = 0; i < fillLen; ++i) {
        	instBufferChar[i] = currentBuffer[i + offset];
        }
}

bool InstCacheReader::fill(const uint64_t ip, void* instBuffer,
	const uint64_t fillLen) {

	if(verbose) {
		output->verbose(CALL_INFO, 8, 0, "Fill: IP=%" PRIu64 ", Length=%" PRIu64 ", CurBufferIP=%" PRIu64 ", NextBufferIPD=%" PRIu64 ", BufferLen=%" PRIu64 "\n",
			ip, fillLen, currentBufferIP, nextBufferIP, bufferLength);
	}

	if( (ip >= currentBufferIP) ) {
		if( (ip < (currentBufferIP + bufferLength)) ) {
			// We can read the fill from within the current instruction buffer
			fillFromCurrentBuffer(ip, instBuffer, fillLen);
			return true;
		}
	}

	if( (ip >= nextBufferIP) ) {
		if( (ip < (nextBufferIP + bufferLength)) ) {
			if( 0 == readReqs.size() ) {
				// There are no pending read requests in flight, so
				// the buffer is already filled with data we need
				rotateBuffers();
				fillFromCurrentBuffer(ip, instBuffer, fillLen);
				return true;
			} else {
				// We know we should be able to fill this soon
				// but we are waiting for the nextBuffer reads to
				// complete.
				return false;
			}
		}
	}

	// If we get down here then neither buffer is able to handle our response
	// and there is no read sequence in flight which will help us, so we
	// have to set up the nextBuffer for what we want to get and then issue
	// requests into the i-cache. This is probably going to be reasonably
	// expensive

	// We do this by getting a buffer aligned IP and then posting the reads
	// we wil need to populate the buffer. This can be initially expensive
	// since we only use part of a buffer but for longer code sequences
	// we should get up to steady state quickly and it will mean we are
	// efficiently using the cache if buffer is set to some multiple of the
	// cache line size

	const uint64_t bufferOffset    = ip % bufferLength;
	const uint64_t bufferAlignedIP = ip - bufferOffset;

	if(verbose) {
		output->verbose(CALL_INFO, 8, 0, "Req ID: %" PRIu64 " causes a buffer flush and reset\n", ip);
		output->verbose(CALL_INFO, 8, 0, "-> Will reset to base buffer pointer: %" PRIu64 ", calculated: ReqIP=%" PRIu64 ", BufLen=%" PRIu64 ", Offset=%" PRIu64 "\n",
			bufferAlignedIP, ip, bufferOffset, bufferLength);
	}

	nextBufferIP = bufferAlignedIP;
	postReadsForNextBuffer();

	return false;
}

void InstCacheReader::setSimpleMem(SimpleMem* newSimpleMem) {
	mem = newSimpleMem;
}
