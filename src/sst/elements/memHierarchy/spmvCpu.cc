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

#include <sst_config.h>
#include <sstream>
#include <sst/core/simulation.h>
#include <sst/core/unitAlgebra.h>
#include <sst/core/timeConverter.h>
#include <sst/core/rng/marsaglia.h>

#include "../memHierarchy/shmemNicCmds.h"
#include "spmvCpu.h"

using namespace SST::MemHierarchy;

SpmvCpu::SpmvCpu(SST::ComponentId_t id, SST::Params& params) : Component(id), m_state(OuterLoopRead) {

    m_myPe = params.find<int>("pe",-1 );
    assert( m_myPe != -1 );
    m_numPes = params.find<int>("numPes",-1 );
    assert( m_numPes != -1 );
    m_numNodes = params.find<int>( "numNodes", 0 );
    assert( m_numNodes );
    m_numActiveThreadsPerNode = params.find<int>("activeThreadsPerNode", m_numPes/m_numNodes );

    int debug_level = params.find<int>("debug_level", 0);
#if 0
    if ( m_myPe != 0 ) {
	debug_level = 0;
    }	
#endif

    // Output for debug
    char buffer[100];
    snprintf(buffer,100,"@t:%d:SpmvCpu::@p():@l ",m_myPe);
    m_dbg.init( buffer, debug_level, params.find<int>("debug_mask",0), (Output::output_location_t)params.find<int>("debug", 0));

    uint64_t nicAddr = params.find<uint64_t>( "nicMemAddr", -1 );
    m_shmemQ = new ShmemQueue( this,
    	params.find<uint32_t>("cmdQSize",16), 
    	params.find<uint64_t>( "hostMemAddr", -1 ),
    	nicAddr, // head addr 
        nicAddr + 8, // pending addr 
        nicAddr + 64, // cmd q addr
        params.find<size_t>( "nicMemLength", 0 )
    );	

    m_dbg.debug(CALL_INFO,1,0,"numPes=%d numNodes=%d activeThreads=%d\n", m_numPes, m_numNodes, m_numActiveThreadsPerNode );

    matrixNx        = params.find<uint64_t>("matrix_nx", 10);
    matrixNy        = params.find<uint64_t>("matrix_ny", 10);
    elementWidth    = params.find<uint64_t>("element_width", 8);
    uint64_t nextStartAddr = 0;
    lhsVecStartAddr = params.find<uint64_t>("lhs_start_addr", nextStartAddr);
    nextStartAddr += matrixNx * elementWidth;	
    rhsVecStartAddr = params.find<uint64_t>("rhs_start_addr", nextStartAddr);
    nextStartAddr += matrixNx * elementWidth;
    localRowStart   = params.find<uint64_t>("local_row_start", 0);
    localRowEnd     = params.find<uint64_t>("local_row_end", matrixNy);
    ordinalWidth    = params.find<uint64_t>("ordinal_width", 8);
    matrixNNZPerRow = params.find<uint64_t>("matrix_nnz_per_row", 4);
    matrixRowIndicesStartAddr = params.find<uint64_t>("matrix_row_indices_start_addr", nextStartAddr);
    nextStartAddr  += (ordinalWidth * (matrixNy + 1));
    matrixColumnIndicesStartAddr = params.find<uint64_t>("matrix_col_indices_start_addr", nextStartAddr);
    nextStartAddr  += (matrixNy * ordinalWidth * matrixNNZPerRow);
    matrixElementsStartAddr = params.find<uint64_t>("matrix_element_start_addr", nextStartAddr);
    iterations = params.find<uint64_t>("iterations", 1);

    if ( 0 ==m_myPe ) { 
        m_dbg.output("%s: matrixNx %" PRIu64 ", matrixNy %" PRIu64 "\n", __func__, matrixNx, matrixNy);
        m_dbg.output("%s: elementWidthu %" PRIu64 "\n", __func__, elementWidth);   	
        m_dbg.output("%s: lhsVecStartAddr %" PRIu64 ", rhsVecStartAddr %" PRIu64"\n",__func__,lhsVecStartAddr,rhsVecStartAddr);
        m_dbg.output("%s: localRowStart %" PRIu64 ", localRowEnd %" PRIu64"\n",__func__,localRowStart, localRowEnd );
    	m_dbg.output("%s: ordinalWidth %" PRIu64 "\n",__func__,ordinalWidth );
    	m_dbg.output("%s: matrixNNZPerRow %" PRIu64 "\n",__func__,matrixNNZPerRow );
    	m_dbg.output("%s: matrixRowIndicesStartAddr %" PRIu64 "\n",__func__,matrixRowIndicesStartAddr );
    	m_dbg.output("%s: matrixColumnIndicesStartAddr %" PRIu64 "\n",__func__, matrixColumnIndicesStartAddr );
    	m_dbg.output("%s: matrixElementsStartAddr %" PRIu64 "\n",__func__,matrixElementsStartAddr );
    }


    const int verbose = params.find<int>("verbose", 0);
    std::stringstream prefix;
    prefix << "@t:" <<getName() << ":SpmvCpu[@p:@l]: ";
    out = new Output( prefix.str(), verbose, 0, SST::Output::STDOUT);
        


#if 0
if ( (m_myPe % (m_numPes/m_numNodes)) < m_numActiveThreadsPerNode ) { 
#endif
if ( m_myPe == 0 ) { 
    std::string cpuClock = params.find<std::string>("clock", "2GHz");
    clockHandler = new Clock::Handler<SpmvCpu>(this, &SpmvCpu::clockTick);
    timeConverter = registerClock(cpuClock, clockHandler );
    m_dbg.debug(CALL_INFO,1,0,"active\n");
    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();
    out->verbose(CALL_INFO, 1, 0, "CPU clock configured for %s\n", cpuClock.c_str());
}else {
    //m_state=Finish;
    m_dbg.debug(CALL_INFO,1,0,"not active\n");
}


    cache_link = loadUserSubComponent<Interfaces::SimpleMem>("memory", ComponentInfo::SHARE_NONE, timeConverter, new SimpleMem::Handler<SpmvCpu>(this, &SpmvCpu::handleEvent) );
    if (!cache_link) {
        std::string interfaceName = params.find<std::string>("memoryinterface", "memHierarchy.memInterface");
        out->verbose(CALL_INFO, 1, 0, "Memory interface to be loaded is: %s\n", interfaceName.c_str());

        Params interfaceParams = params.find_prefix_params("memoryinterfaceparams.");
        interfaceParams.insert("port", "cache_link");
        cache_link = loadAnonymousSubComponent<Interfaces::SimpleMem>(interfaceName, "memory", 0, ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS, 
                    interfaceParams, timeConverter, new SimpleMem::Handler<SpmvCpu>(this, &SpmvCpu::handleEvent));
            
        if (!cache_link)
            out->fatal(CALL_INFO, -1, "%s, Error loading memory interface\n", getName().c_str());
    }

    out->verbose(CALL_INFO, 1, 0, "Loaded memory interface successfully.\n");
    m_curRow = localRowStart;
}

SpmvCpu::~SpmvCpu() {
	//printf("%s:SpmvCpu::%s\n",getName().c_str(),__func__);
	delete out;
}

void SpmvCpu::finish() {
	m_dbg.debug(CALL_INFO,1,0,"\n");
}

void SpmvCpu::init(unsigned int phase) {
    cache_link->init(phase);
}

void SpmvCpu::handleEvent( Interfaces::SimpleMem::Request* ev) {
    m_shmemQ->handleEvent( ev );
}
bool SpmvCpu::clockTick(SST::Cycle_t cycle) {

    int node;
    uint64_t addr;
    uint64_t srcAddr,dstAddr;

    m_shmemQ->process( cycle );

    switch( m_state ) {

    case OuterLoopRead:
	m_dbg.debug(CALL_INFO,1,0,"top of OuterLoop row=%" PRIu64 "\n",m_curRow);

        m_shmemQ->get( dstAddr, matrixRowIndicesStartAddr + (ordinalWidth * m_curRow), &readStart );
        m_shmemQ->get( dstAddr, matrixRowIndicesStartAddr + (ordinalWidth * (m_curRow + 1)), &readEnd );
        m_shmemQ->put( rhsVecStartAddr + (m_curRow * matrixNNZPerRow) * elementWidth, srcAddr, &writeCurValue );
        m_state = OuterLoopReadWait;
        break;
    
    case OuterLoopReadWait:
        if ( ! readStart.done || ! readEnd.done ) {
            break;
        }
        m_state = InnerLoop;

        m_curCol = m_curRow;

    case InnerLoop:
        m_dbg.debug(CALL_INFO,1,0,"top of InnerLoop row=%" PRIu64 " col=%" PRIu64 "\n",m_curRow,m_curCol);
        m_shmemQ->get( dstAddr, matrixColumnIndicesStartAddr + (m_curRow * matrixNNZPerRow + m_curCol) * ordinalWidth, &readCol );
        m_shmemQ->get( dstAddr, matrixElementsStartAddr + (m_curRow * matrixNNZPerRow + m_curCol) * elementWidth, &readMat );
        m_state = ReadColWait;
        break;

    case ReadColWait:
        if ( ! readCol.done ) {
            break;
        }

        m_shmemQ->get( dstAddr, lhsVecStartAddr + (m_curRow * matrixNNZPerRow + m_curCol) * elementWidth, &readLHS );
    	++m_curCol;

        m_dbg.debug(CALL_INFO,1,0,"bottom of InnerLoop row=%" PRIu64 " col=%" PRIu64 "\n",m_curRow,m_curCol);
        if ( m_curCol < m_curRow + matrixNNZPerRow) {
            m_state = InnerLoop;
            break;
        } else {
            m_state = QuietWait;
        }

    case QuietWait:
        if ( ! m_shmemQ->quiet( ) ) {
		    break;
	    }

        m_dbg.debug(CALL_INFO,1,0,"bottom of outerLoop row=%" PRIu64 "\n",m_curRow);

        m_shmemQ->put( rhsVecStartAddr + (m_curRow * matrixNNZPerRow) * elementWidth,srcAddr, &writeResult );
		
        ++m_curRow;

        if ( m_curRow < localRowEnd ) {
	        m_state = OuterLoopRead;
            break;
        } else {
            m_state = Finish;
        }

    case Finish:
    	if ( ! m_shmemQ->process( cycle ) ) {
            m_dbg.debug(CALL_INFO,1,0,"finished\n");
            primaryComponentOKToEndSim();
            return true;
        } else {
       	    break;
        }
    }
    return false;
}

SpmvCpu::ShmemQueue::ShmemQueue( SpmvCpu* cpu, int qSize, uint64_t hostTailAddr, uint64_t nicHeadAddr, uint64_t pendingAddr, uint64_t qAddr, size_t nicMemLength  ) : 
	m_cpu(cpu), m_qSize(qSize), hostTailAddr(hostTailAddr), nicHeadAddr(nicHeadAddr), nicPendingCntAddr(pendingAddr), nicCmdQAddr(qAddr), nicMemLength(nicMemLength), 
    m_head(0),m_state(Init),m_activeReq(NULL), m_handle(0)
{
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"cmdQSize=%d nicAddr=0x%" PRIx64 " hostMemAddr=0x%" PRIx64 "\n",
            m_qSize, nicHeadAddr, hostTailAddr );
}
bool SpmvCpu::ShmemQueue::quiet() {
	return true;
}

void SpmvCpu::ShmemQueue::get( uint64_t dstAddr, uint64_t srcAddr, ShmemReq* req ) {
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"srcAddr=0x%" PRIx64 "\n", srcAddr );
    req->done = false;	
    int remoteNode = m_cpu->calcNode( srcAddr );
    uint64_t remoteAddr = m_cpu->calcAddr( srcAddr ); 
    m_cmdQ.push( new ShmemCmd( FamGet, remoteNode, remoteAddr, dstAddr, req ) );
}

void SpmvCpu::ShmemQueue::put( uint64_t dstAddr, uint64_t srcAddr, ShmemReq* req ) {
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"dstAddr=0x%" PRIx64 "\n", dstAddr );
    req->done = false;	

    int remoteNode = m_cpu->calcNode( dstAddr );
    uint64_t remoteAddr = m_cpu->calcAddr( dstAddr ); 
    m_cmdQ.push( new ShmemCmd( FamPut, remoteNode, remoteAddr, srcAddr, req ) );
}

void SpmvCpu::ShmemQueue::handleEvent( Interfaces::SimpleMem::Request* ev) {
    SimpleMem::Request::id_t reqID = ev->id;

    assert ( m_pending.find( ev->id ) != m_pending.end() );

    m_cpu->m_dbg.debug(CALL_INFO,1,0,"id=%llu %s\n", reqID,ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ? "ReadResp":"WriteResp");

    std::string cmd;
    if ( ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ) { 
        m_cpu->m_dbg.debug(CALL_INFO,1,0,"ReadResp\n");
        m_pending[ev->id]->resp = ev;			
        m_pending.erase(ev->id);
    } else if ( ev->cmd == Interfaces::SimpleMem::Request::Command::WriteResp ) { 
        m_cpu->m_dbg.debug(CALL_INFO,1,0,"WriteResp\n");
        delete m_pending[ev->id];
        m_pending.erase(ev->id);
        delete ev;
    } else {
        assert(0);
    }
}


bool SpmvCpu::ShmemQueue::process( Cycle_t cycle ){

    if ( NULL == m_activeReq ) {
        if ( m_cmdQ.empty() ) {
            return false;
        } 
       	m_activeReq = m_cmdQ.front();
		m_cpu->m_dbg.debug(CALL_INFO,1,0,"new cmd %d remoteNode %d, remoteAddr %" PRIx64 "\n",m_activeReq->op,m_activeReq->node, m_activeReq->remoteAddr);
        m_cmdQ.pop();
        m_state = ReadHeadTail;
    }
	
    switch ( m_state ) {

	case Init:
		m_cpu->m_dbg.debug(CALL_INFO,1,0,"Init\n");
		write32( hostTailAddr, 0 );
        break;

      case ReadHeadTail:
        m_cpu->m_dbg.debug(CALL_INFO,1,0,"ReadHeadTail\n");
        m_readReq = read( hostTailAddr, 4 );
        m_state = WaitRead;	
		break;	

      case WaitRead:

		if ( m_readReq->isDone() ) {
			m_tail = m_readReq->data();
			m_cpu->m_dbg.debug(CALL_INFO,1,0,"read of head and tail done head=%d tail=%d\n",m_head,m_tail);
			delete m_readReq;
			m_state = CheckHeadTail;
		} 
		break;

	case CheckHeadTail:
		m_cpu->m_dbg.debug(CALL_INFO,1,0,"CheckHeadTail\n");
		if ( ( m_head + 1 ) % m_qSize  == m_tail ) {
			m_state = ReadHeadTail;
		} else {
            NicCmd cmd;
            cmd.type = NicCmd::Fam; 
            cmd.respAddr = 0x10dead;
            cmd.handle = m_handle++;
            cmd.data.fam.op = (int)m_activeReq->op;
            cmd.data.fam.destNode = m_activeReq->node;
            cmd.data.fam.destAddr = m_activeReq->remoteAddr;

            m_cpu->m_dbg.debug(CALL_INFO,1,0,"dest node %d, dest addr %" PRIu64 "\n",m_activeReq->node,m_activeReq->remoteAddr); 

            for ( int i = 0; i < sizeof(NicCmd)/8; i++ ) {
                write64( cmdQAddr(m_head) + i*8, (uint64_t*) &cmd + i );
            }

            m_state = Fence;
            m_nextState = WriteHead;
        }
        break;
    case Fence:
        if ( m_pending.empty() ) {
            m_state = m_nextState;
        }
        break;

    case WriteHead:
		++m_head;
		m_head %= m_qSize;
		write32( nicHeadAddr, m_head );
        m_activeReq = NULL;
		break;

	case Quiet:
		m_readReq = read( nicPendingCntAddr, 8 );
        m_state = WaitReadQuiet;
        break;

    case WaitReadQuiet:
		if ( ! m_readReq->isDone() ) {
            break;
        } 
		if ( m_readReq->data() != 0 ) {
            m_state = Quiet;
            break;
        }
		m_cpu->m_dbg.debug(CALL_INFO,1,0,"pending count is 0, we can finish\n");
		delete m_readReq;
    }
    return false;
}
