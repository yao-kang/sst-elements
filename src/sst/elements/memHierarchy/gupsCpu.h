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

class CPURequest {
public:
	CPURequest(const uint64_t origID) :
		originalID(origID), issueTime(0), outstandingParts(0) {}
	void incPartCount() { outstandingParts++; }
	void decPartCount() { outstandingParts--; }
	bool completed() const { return 0 == outstandingParts; }
	void setIssueTime(const uint64_t now) { issueTime = now; }
	uint64_t getIssueTime() const { return issueTime; }
	uint64_t getOriginalReqID() const { return originalID; }
	uint32_t countParts() const { return outstandingParts; }
protected:
	uint64_t originalID;
	uint64_t issueTime;
	uint32_t outstandingParts;
};

class GupsCpu : public SST::Component {
public:

	GupsCpu(SST::ComponentId_t id, SST::Params& params);
	void finish(){ 
#if 0
		printf( "state=%d count=%d totalGups=%d\n",m_state,m_count,m_totalGups );
		if ( m_shmemQ ) {
			m_shmemQ->printInfo();
		}
#endif
	}
	void init(unsigned int phase) {
        cache_link->init(phase);
    }

    Output& dbg() { return m_dbg; }
    int myPe() { return m_myPe; }
    int myThread() { return m_myPe % m_threadsPerNode; }
    int threadsPerNode() { return m_threadsPerNode; }
    void sendRequest( SimpleMem::Request* req ) { cache_link->sendRequest(req); }

	SST_ELI_REGISTER_COMPONENT(
        	GupsCpu,
        	"memHierarchy",
        	"gupsCpu",
        	SST_ELI_ELEMENT_VERSION(1,0,0),
        	"Creates a SHMEM GUPS CPU",
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


	GupsCpu();  // for serialization only
	GupsCpu(const GupsCpu&); // do not implement
	void operator=(const GupsCpu&); // do not implement
	~GupsCpu() {
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

    ShmemQueue<GupsCpu>* m_shmemQ;

    ShmemReq m_quietReq;

	enum State { Quiet, QuietWait } m_state;

    Output m_dbg;

    Statistic<uint64_t>* m_statAddrIncOp;
    Statistic<uint64_t>* m_statLoopLat;
    Statistic<uint64_t>* m_statReadLat;
    Statistic<uint64_t>* m_statFenceLat;

	MyRequest* m_readReq;
	int m_totalGups;
	int m_count;
    int m_myPe;
    int m_numPes;
    int m_numNodes;
    int m_threadsPerNode;
    int m_activeThreadsPerNode;
    uint64_t seed_a;
    uint64_t seed_b;
    RNG::SSTRandom* rng;
    size_t m_gupsMemSize;
};


}
}
#endif /* _GupsCpu_H */

