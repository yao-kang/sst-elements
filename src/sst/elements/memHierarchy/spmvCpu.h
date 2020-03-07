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

class SpmvCpu : public SST::Component {
public:

	SpmvCpu(SST::ComponentId_t id, SST::Params& params);
	void finish();
	void init(unsigned int phase);

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
	~SpmvCpu();

	void handleEvent( SimpleMem::Request* ev );
	bool clockTick( SST::Cycle_t );

 	Output* out;

	TimeConverter* timeConverter;
	Clock::HandlerBase* clockHandler;
	SimpleMem* cache_link;

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

    struct ShmemReq {
        ShmemReq(): done(true) {}
        bool done;
    };

    class ShmemQueue {

        enum Op { ShmemInc, ShmemPut, FamGet, FamPut };
	    struct ShmemCmd {
    
            ShmemCmd( Op op, int node, uint64_t remoteAddr, uint64_t localAddr, ShmemReq* req ) : 
                    op(op), node(node), remoteAddr(remoteAddr), localAddr(localAddr), req(req) {}
            Op op;
            int node;
            uint64_t remoteAddr;
            uint64_t localAddr;
            ShmemReq* req; 
	    };

      public:
        ShmemQueue( SpmvCpu* cpu, int qSize, uint64_t hostTailAddr, uint64_t nicHeadAddr, uint64_t pendingAddr, uint64_t qAddr, size_t nicMemLength  );
        bool process( Cycle_t );
        void get( uint64_t, uint64_t, ShmemReq* );
        void put( uint64_t, uint64_t, ShmemReq* );
        bool quiet();
        void handleEvent( SimpleMem::Request* ev );
        std::queue<ShmemCmd*> m_cmdQ;
     private:
        SpmvCpu* m_cpu;
        uint32_t m_head;
        uint32_t m_tail;
        int m_qSize;
    	uint64_t hostTailAddr;
    	uint64_t nicHeadAddr;
    	uint64_t nicPendingCntAddr;
    	uint64_t nicCmdQAddr;
        size_t   nicMemLength;
        enum State { Init, ReadHeadTail, WaitRead, CheckHeadTail, Fence, WriteHead, Quiet, WaitReadQuiet  } m_state;
        std::map<SimpleMem::Request::id_t, MyRequest*> m_pending;

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
            return nicCmdQAddr + pos * sizeof(NicCmd);
        }

        MyRequest* write( uint64_t addr, uint64_t data, int num ) {

            SimpleMem::Request* req = new SimpleMem::Request( SimpleMem::Request::Write, addr, num );
            if ( notCached( addr ) ) {
                req->flags = SimpleMem::Request::F_NONCACHEABLE;
            }

            m_cpu->m_dbg.debug(CALL_INFO,1,0,"addr=0x%" PRIx64 " data=%llu id=%llu\n",addr,data,req->id);

            for ( int i = 0; i < num; i++ ) {
                req->data.push_back( (data >> i*8) & 0xff );
            }

            m_pending[ req->id ] = new MyRequest;
            m_cpu->cache_link->sendRequest(req);
            return m_pending[ req->id ];
        }

        MyRequest* read( uint64_t addr, int size ) {
            SimpleMem::Request* req = new SimpleMem::Request( SimpleMem::Request::Read, addr, size );
    	    if ( notCached( addr ) ) {
			    req->flags = SimpleMem::Request::F_NONCACHEABLE;
		    }
		    m_pending[ req->id ] = new MyRequest;
		    m_cpu->m_dbg.debug(CALL_INFO,1,0,"addr=0x%" PRIx64 " id=%llu\n",addr,req->id);
		    m_cpu->cache_link->sendRequest(req);
		    return m_pending[ req->id ];
	    }

	    bool notCached( uint64_t addr ) {
		    return  addr >= nicHeadAddr  && addr < nicHeadAddr + nicMemLength;
	    }

	
        State m_nextState;
        MyRequest* m_readReq;
        ShmemCmd*  m_activeReq;
        int m_handle;
    };

    int calcNode( uint64_t addr ) {
        return 1;
    }	

    uint64_t calcAddr( uint64_t addr ) {
        return addr;
    }	

    ShmemQueue* m_shmemQ;
    ShmemReq readStart;
    ShmemReq readEnd;
    ShmemReq readCol;
    ShmemReq readLHS;
    ShmemReq readMat;
    ShmemReq writeResult;
    ShmemReq writeCurValue;

    enum State { OuterLoopRead, OuterLoopReadWait, InnerLoop, ReadColWait, QuietWait, Finish  } m_state;

    Output m_dbg;

    int m_myPe;
    int m_numPes;
    int m_numNodes;
    int m_numActiveThreadsPerNode;

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

