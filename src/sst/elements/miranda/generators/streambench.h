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


#ifndef _H_SST_MIRANDA_STREAM_BENCH_GEN
#define _H_SST_MIRANDA_STREAM_BENCH_GEN

#include <sst/elements/miranda/mirandaGenerator.h>
#include <sst/elements/miranda/generators/generatorSplit.h>
#include <sst/core/output.h>

#include <queue>

namespace SST {
namespace Miranda {

class STREAMBenchGenerator : public RequestGenerator {

public:
	STREAMBenchGenerator( Component* owner, Params& params );
	~STREAMBenchGenerator();
	void generate(MirandaRequestQueue<GeneratorRequest*>* q);
	bool isFinished();
	void completed();

private:
	uint64_t reqLength;

	uint64_t start_a;
	uint64_t start_b;
	uint64_t start_c;

	uint64_t n;
	uint64_t n_per_call;
	uint64_t i;

	Output*  out;

};

class STREAMBenchGeneratorSplit : public GeneratorSplit {
public: 
    STREAMBenchGeneratorSplit( Params& ) {}

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

        int count =  params.find<int>( "n" , found );
        assert ( found );

        size_t start_a  =  params.find<size_t>( "start_a" , found );
        assert ( found );

        size_t start_b  =  params.find<size_t>( "start_b" , start_a + (count * width) );
        size_t start_c  =  params.find<size_t>( "start_c" , start_b + (count * width) );

        start_a += (count / numThreads) * thread * width;
        start_b += (count / numThreads) * thread * width;
        start_c += (count / numThreads) * thread * width;

        if ( thread != numThreads  - 1 ) {
            count /= numThreads;
        } else {
            count %= numThreads;
            if ( 0 == count ) {
                count /= numThreads;
            }
        }

        insertParam<>( newParams, params, "start_a", start_a);
        insertParam<>( newParams, params, "start_b", start_b);
        insertParam<>( newParams, params, "start_c", start_c);
        insertParam<>( newParams, params, "n", count);

        return std::make_pair( name, newParams );
    }
};

}
}

#endif
