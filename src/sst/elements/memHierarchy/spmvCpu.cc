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
    m_numPes = params.find<int>("numPes",-1 );
    assert( m_myPe != -1 );
    assert( m_numPes != -1 );

    // Output for debug
    char buffer[100];
    snprintf(buffer,100,"@t:%d:SpmvCpu::@p():@l ",m_myPe);
    m_dbg.init( buffer,
        params.find<int>("debug_level", 0),
        params.find<int>("debug_mask",0),
        (Output::output_location_t)params.find<int>("debug", 0));

    m_numNodes = params.find<int>( "numNodes", 0 );
    assert( m_numNodes );
    m_threadsPerNode = params.find<int>( "threadsPerNode", 1 );
    int numThreadsPerNode = m_numPes/m_numNodes;
    m_activeThreadsPerNode = params.find<int>("activeThreadsPerNode", numThreadsPerNode );

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

    if ( m_myPe == 0 ) {
    //if ( (m_myPe % numThreadsPerNode) < m_activeThreadsPerNode ) {

        m_dbg.debug(CALL_INFO,1,DBG_APP_FLAG,"myPe=%d numPes=%d threadsPerNode=%d\n", m_myPe, m_numPes, m_threadsPerNode );
        m_dbg.debug(CALL_INFO,1,DBG_APP_FLAG,"numNodes=%d activeThreadsPerNode=%d\n", m_numNodes, m_activeThreadsPerNode );

        m_shmemQ = new ShmemQueue<SpmvCpu>( this,
            params.find<int>("cmdQSize", 64),
            params.find<int>("respQSize", params.find<int>("cmdQSize", 64) ),
            params.find<uint64_t>( "nicBaseAddr", 0x100000000 ),
            params.find<uint64_t>("hostQueueInfoBaseAddr", 0 ),
            params.find<size_t>("hostQueueInfoSizePerPe", 64 ),
            params.find<uint64_t>("hostQueueBaseAddr", 0x10000 ),
            params.find<size_t>("hostQueueSizePerPe", 4096 )
        );

        std::string cpuClock = params.find<std::string>("clock", "2GHz");
        clockHandler = new Clock::Handler<SpmvCpu>(this, &SpmvCpu::clockTick);
        timeConverter = registerClock(cpuClock, clockHandler );
        registerAsPrimaryComponent();
        primaryComponentDoNotEndSim();
        out->verbose(CALL_INFO, 1, 0, "CPU clock configured for %s\n", cpuClock.c_str());
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

bool SpmvCpu::clockTick(SST::Cycle_t cycle) {

    int node;
    uint64_t addr;
    uint64_t srcAddr = 0, dstAddr = 0;

    m_shmemQ->process( cycle );

    switch( m_state ) {

    case OuterLoopRead:
	m_dbg.debug(CALL_INFO,1,DBG_APP_FLAG,"top of OuterLoop row=%" PRIu64 "\n",m_curRow);

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
        m_dbg.debug(CALL_INFO,1,DBG_APP_FLAG,"top of InnerLoop row=%" PRIu64 " col=%" PRIu64 "\n",m_curRow,m_curCol);
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

        m_dbg.debug(CALL_INFO,1,DBG_APP_FLAG,"bottom of InnerLoop row=%" PRIu64 " col=%" PRIu64 "\n",m_curRow,m_curCol);
        if ( m_curCol < m_curRow + matrixNNZPerRow) {
            m_state = InnerLoop;
            break;
        } else {
            m_state = QuietWait;
            m_shmemQ->quiet( &m_quietReq );
        }

    case QuietWait:
        if ( ! m_quietReq.done ) {
		    break;
	    }

        m_dbg.debug(CALL_INFO,1,DBG_APP_FLAG,"bottom of outerLoop row=%" PRIu64 "\n",m_curRow);

        m_shmemQ->put( rhsVecStartAddr + (m_curRow * matrixNNZPerRow) * elementWidth,srcAddr, &writeResult );
		
        ++m_curRow;

        if ( m_curRow < localRowEnd ) {
	        m_state = OuterLoopRead;
            break;
        } else {
            m_shmemQ->quiet( &m_quietReq );
            m_state = Finish;
        }

    case Finish:
        if ( ! m_quietReq.done ) {
		    break;
	    }

    	if ( ! m_shmemQ->process( cycle ) ) {
            m_dbg.debug(CALL_INFO,1,DBG_APP_FLAG,"finished\n");
            primaryComponentOKToEndSim();
            return true;
        } else {
       	    break;
        }
    }
    return false;
}
