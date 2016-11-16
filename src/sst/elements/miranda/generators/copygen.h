// Copyright 2009-2016 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2016, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _H_SST_MIRANDA_COPY_BENCH_GEN
#define _H_SST_MIRANDA_COPY_BENCH_GEN

#include <sst/core/rng/sstrng.h>
#include <sst/elements/miranda/mirandaGenerator.h>
#include <sst/elements/miranda/generators/generatorSplit.h>
#include <sst/core/output.h>

#include <queue>

using namespace SST::RNG;

namespace SST {
namespace Miranda {

class CopyGenerator : public RequestGenerator {

public:
	CopyGenerator( Component* owner, Params& params ) : RequestGenerator(owner, params) {
		const uint32_t verbose = params.find<uint32_t>("verbose", 0);
		out = new Output("CopyGenerator[@p:@l]: ", verbose, 0, Output::STDOUT);

		readAddr  = params.find<uint64_t>("read_start_address",  0);
		reqLength = params.find<uint64_t>("operandwidth", 8);
		itemCount = params.find<uint64_t>("request_count", 1024);
		
		n_per_call = params.find<uint64_t>("n_per_call", 2);

		// Write address default is sized for number of requests * req lengtgh
		writeAddr = params.find<uint64_t>("write_start_address",
			readAddr + (reqLength * itemCount));

		// Start generator at 0
		nextItem = 0;

		out->verbose(CALL_INFO, 1, 0, "Copy count is      %" PRIu64 "\n", itemCount);
		out->verbose(CALL_INFO, 1, 0, "operandwidth       %" PRIu64 "\n", reqLength);
		out->verbose(CALL_INFO, 1, 0, "read start       0x%" PRIx64 "\n", readAddr);
		out->verbose(CALL_INFO, 1, 0, "writestart       0x%" PRIx64 "\n", writeAddr);
		out->verbose(CALL_INFO, 1, 0, "N-per-generate     %" PRIu64 "\n", n_per_call);
	}

	~CopyGenerator() {
		delete out;
	}

	void generate(MirandaRequestQueue<GeneratorRequest*>* q) {
	for ( int i = 0; i < n_per_call; i++ ) {
		if(nextItem == itemCount) {
			return;
		}

		MemoryOpRequest* read = new MemoryOpRequest(readAddr  + (nextItem * reqLength), reqLength, READ);
		MemoryOpRequest* write = new MemoryOpRequest(writeAddr + (nextItem * reqLength), reqLength, WRITE);
		write->addDependency(read->getRequestID());
		q->push_back(read);
		q->push_back(write);

		nextItem++;
    }
	}

	bool isFinished() {
		return (nextItem == itemCount);
	}

	void completed() {}

private:
	uint64_t nextItem;
	uint64_t readAddr;
	uint64_t writeAddr;
	uint64_t itemCount;
	uint64_t reqLength;
	uint64_t n_per_call;
	Output*  out;

};

class CopyGeneratorSplit : public GeneratorSplit {
public:
    CopyGeneratorSplit(Params& params ) {}

    std::pair<std::string, SST::Params> split( size_t numThreads, int thread, std::string name, SST::Params params ) 
    {

        SST::Params newParams;

        copyParam( newParams, params, "operandwidth" );
        copyParam( newParams, params, "verbose" );
        copyParam( newParams, params, "generatorParams.verbose" );
        copyParam( newParams, params, "n_per_call" );

        bool found;
        int width =  params.find<int>( "operandwidth" , found );
        assert ( found );

        int count =  params.find<int>( "request_count" , found );
        assert ( found );

        size_t read_start  =  params.find<size_t>( "read_start_address" , found );
        assert ( found );
        size_t write_start  =  params.find<size_t>( "write_start_address", read_start + (width * count) );

        read_start += (count / numThreads) * thread * width;
        write_start += (count / numThreads) * thread * width;

        if ( thread != numThreads  - 1 ) {
            count /= numThreads;
        } else {
            count %= numThreads;
            if ( 0 == count ) {
                count /= numThreads;
            }
        }

        insertParam<>( newParams, params, "read_start_address", read_start);
        insertParam<>( newParams, params, "write_start_address", write_start);
        insertParam<>( newParams, params, "request_count", count);

        return std::make_pair( name, newParams );
    }
};


}
}

#endif
