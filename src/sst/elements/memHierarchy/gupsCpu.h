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
	void finish(){ }
	void init(unsigned int phase) {
        cache_link->init(phase);
    }

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

#if 0
	struct MyRequest {
		MyRequest() : resp( NULL ) { }
		~MyRequest() {
			delete resp;
		}
		bool isDone() { return resp != NULL; }
		SimpleMem::Request* resp;
		uint64_t data() {
			uint64_t tmp = 0;
			for ( int i = 0; i < resp->data.size(); i++) {
                tmp |= resp->data[i] << i * 8;
			}
			//printf("%s() size=%zu data=0x%" PRIx64 "\n",__func__, resp->data.size(), tmp);
			return tmp;
		}
	};
#endif

#include "shmemCmdQ.h"

    ShmemQueue<GupsCpu>* m_shmemQ;

    ShmemReq m_quietReq;

#if 0
	MyRequest* write64( uint64_t addr, uint64_t* data ) {
		return write( addr, *data, 8 );
	}

	MyRequest* write64( uint64_t addr, uint64_t data ) {
		return write( addr, data, 8 );
	}

	MyRequest* write32( uint64_t addr, uint32_t data ) {
		return write( addr, data, 4 );
	}

	uint64_t cmdQAddr( int pos ) {
		return NicCmdQAddr + pos * sizeof(NicCmd);
	}

	MyRequest* write( uint64_t addr, uint64_t data, int num ) {

		SimpleMem::Request* req = new SimpleMem::Request( SimpleMem::Request::Write, addr, num );
    	if ( notCached( addr ) ) {
			req->flags = SimpleMem::Request::F_NONCACHEABLE;
		}

		m_dbg.debug(CALL_INFO,1,0,"addr=0x%" PRIx64 " data=%llu id=%llu\n",addr,data,req->id);

		for ( int i = 0; i < num; i++ ) {
            req->data.push_back( (data >> i*8) & 0xff );
        }

		m_pending[ req->id ] = new MyRequest;
		cache_link->sendRequest(req);
		return m_pending[ req->id ];
	}

	MyRequest* read( uint64_t addr, int size ) {
		SimpleMem::Request* req = new SimpleMem::Request( SimpleMem::Request::Read, addr, size );
    	if ( notCached( addr ) ) {
			req->flags = SimpleMem::Request::F_NONCACHEABLE;
		}
		m_pending[ req->id ] = new MyRequest;
		m_dbg.debug(CALL_INFO,1,0,"addr=0x%" PRIx64 " id=%llu\n",addr,req->id);
		cache_link->sendRequest(req);
		return m_pending[ req->id ];
	}

	bool notCached( uint64_t addr ) {
		return  addr >= NicHeadAddr  && addr < NicHeadAddr + m_nicMemLength;
	}
#endif
	
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

#if 0
	enum State { Init, ReadHeadTail, WaitRead, CheckHeadTail, Fence, WriteHead, Quiet, WaitReadQuiet, Finish } m_state;
    State m_nextState;
	std::map<SimpleMem::Request::id_t, MyRequest*> m_pending;
    enum { ShmemInc, ShmemPut };
	uint64_t HostTailAddr;
	uint64_t HostPendingCntAddr;
	uint64_t NicHeadAddr;
	//uint64_t NicPendingCntAddr;
	uint64_t NicCmdQAddr;
    size_t  m_nicMemLength;

	uint32_t m_head;
	uint32_t m_tail;
	int m_qSize;
    uint64_t m_fenceStart;
    uint64_t m_loopStart;
#endif
};


}
}
#endif /* _GupsCpu_H */

