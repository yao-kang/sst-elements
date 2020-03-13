// Copyright 2009-2019 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2019, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef MEMHIERARCHY_GUPS_CPU_H
#define MEMHIERARCHY_GUPS_CPU_H

#include <sst/core/component.h>
#include <sst/core/interfaces/simpleMem.h>
#include <sst/core/statapi/stataccumulator.h>
#include <queue>

using namespace SST;
using namespace SST::Interfaces;
using namespace SST::Statistics;

#include "shmemCmdQ.h"

namespace SST {
namespace MemHierarchy {

class SpmvCpu : public SST::Component {
public:

	SpmvCpu(SST::ComponentId_t id, SST::Params& params);
	void finish() { }
	void init(unsigned int phase) {
        cache_link->init(phase);
    }

    Output& dbg() { return m_dbg; }
    int myPe() { return m_myPe; }
    int threadsPerNode() { return m_threadsPerNode; }
    void sendRequest( SimpleMem::Request* req ) { cache_link->sendRequest(req); }

    int calcNode( uint64_t addr ) {
        return 1;
    }	
    uint64_t calcAddr( uint64_t addr ) {
        return addr;
    }	

	SST_ELI_REGISTER_COMPONENT(
        	SpmvCpu,
        	"memHierarchy",
        	"spmvCpu",
        	SST_ELI_ELEMENT_VERSION(1,0,0),
        	"Creates a SHMEM SpMV CPU",
        	COMPONENT_CATEGORY_PROCESSOR
    	)

	SST_ELI_DOCUMENT_PARAMS(
     		{ "verbose",               "Sets the verbosity of output produced by the CPU",     "0" },
     		{ "clock",            "Clock for the base CPU", "2GHz" },
     		{ "memoryinterface",  "Sets the memory interface module to use", "memHierarchy.memInterface" },
    	)

	SST_ELI_DOCUMENT_STATISTICS(
        { "addrIncOp",    "addr of memory access",     "addr", 1 },
        { "fenceLatency",    "time to quiesce",     "ns", 1 },
        { "loopLatency",    "",     "ns", 1 },
        { "readLatency",    "",     "ns", 1 }
	)

	SST_ELI_DOCUMENT_PORTS(
		{ "cache_link",      	"Link to Memory Controller", { "memHierarchy.memEvent" , "" } },
	)

	SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
                { "memory",     "The memory interface to use (e.g., interface to caches)", "SST::Interfaces::SimpleMem" }
    	)

private:

	SpmvCpu();  // for serialization only
	SpmvCpu(const SpmvCpu&); // do not implement
	void operator=(const SpmvCpu&); // do not implement
	~SpmvCpu() {
        delete out;
    }

	void handleEvent( SimpleMem::Request* ev ) {
        m_shmemQ->handleEvent( ev );
    }
	bool clockTick( SST::Cycle_t );

 	Output* out;

	TimeConverter* timeConverter;
	Clock::HandlerBase* clockHandler;
	SimpleMem* cache_link;

    ShmemQueue<SpmvCpu>* m_shmemQ;

    ShmemReq m_quietReq;

    ShmemReq readStart;
    ShmemReq readEnd;
    ShmemReq readCol;
    ShmemReq readLHS;
    ShmemReq readMat;
    ShmemReq writeResult;
    ShmemReq writeCurValue;

    enum State { OuterLoopRead, OuterLoopReadWait, InnerLoop, ReadColWait, QuietWait, Finish } m_state;

    Output m_dbg;

    int m_myPe;
    int m_numPes;
    int m_numNodes;
    int m_activeThreadsPerNode;
    int m_threadsPerNode;

    uint64_t m_curRow, m_curCol;
    uint64_t iterations;
    uint64_t matrixNx;
    uint64_t matrixNy;
    uint64_t elementWidth;
    uint64_t lhsVecStartAddr;
    uint64_t rhsVecStartAddr;
    uint64_t ordinalWidth;
    uint64_t matrixNNZPerRow;
    uint64_t matrixRowIndicesStartAddr;
    uint64_t localRowStart;
    uint64_t localRowEnd;
    uint64_t matrixColumnIndicesStartAddr;
    uint64_t matrixElementsStartAddr;
};


}
}
#endif /* _SpmvCpu_H */

