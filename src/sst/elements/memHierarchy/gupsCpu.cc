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
#include "gupsCpu.h"

using namespace SST::MemHierarchy;

GupsCpu::GupsCpu(SST::ComponentId_t id, SST::Params& params) :
	Component(id), m_count(0), m_state(Quiet)
#if 0
, m_head(0) 
#endif
{

    m_myPe = params.find<int>("pe",-1 );
    m_numPes = params.find<int>("numPes",-1 );
    assert( m_myPe != -1 );
    assert( m_numPes != -1 );

    // Output for debug
    char buffer[100];
    snprintf(buffer,100,"@t:%d:GupsCpu::@p():@l ",m_myPe);
    m_dbg.init( buffer,
        params.find<int>("debug_level", 0),
        params.find<int>("debug_mask",0),
        (Output::output_location_t)params.find<int>("debug", 0));

    m_shmemQ = new ShmemQueue<GupsCpu>( this,
        params.find<int>("cmdQSize", 64),
        params.find<int>("respQSize", params.find<int>("cmdQSize", 64) ),
        params.find<uint64_t>( "nicBaseAddr", 0x100000000 ),
        params.find<size_t>( "nicMemLength", 4096 ),
        params.find<uint64_t>("hostQueueInfoBaseAddr", 0 ),
        params.find<size_t>("hostQueueInfoSizePerPe", 64 ),
        params.find<uint64_t>("hostQueueBaseAddr", 0x10000 ),
        params.find<size_t>("hostQueueSizePerPe", 4096 )
    );

    m_numNodes = params.find<int>( "numNodes", 0 );
    assert( m_numNodes );
    int numThreadsPerNode = m_numPes/m_numNodes;
    m_activeThreadsPerNode = params.find<int>("activeThreadsPerNode", numThreadsPerNode );

    seed_a     = params.find<uint64_t>("seed_a", 11);
    seed_b     = params.find<uint64_t>("seed_b", m_myPe + 31);
    rng = new RNG::MarsagliaRNG(seed_a, seed_b);

	m_totalGups = params.find<uint32_t>("iterations",1); 
    m_gupsMemSize = params.find<size_t>("gupsMemSize",0);
    assert( m_gupsMemSize );

if ( m_myPe == 0 ) {
    m_dbg.debug(CALL_INFO,1,0,"myPe=%d numPes=%d \n", m_myPe, m_numPes );
    m_dbg.debug(CALL_INFO,1,0,"totalGups=%d gupsMemSize=%zu\n", m_totalGups, m_gupsMemSize );
}

	const int verbose = params.find<int>("verbose", 0);
	std::stringstream prefix;
	prefix << "@t:" <<getName() << ":GupsCpu[@p:@l]: ";
	out = new Output( prefix.str(), verbose, 0, SST::Output::STDOUT);
        
    if ( m_myPe == 0 ) {
    //if ( (m_myPe % numThreadsPerNode) < m_activeThreadsPerNode ) {
        std::string cpuClock = params.find<std::string>("clock", "2GHz");
	    clockHandler = new Clock::Handler<GupsCpu>(this, &GupsCpu::clockTick);
	    timeConverter = registerClock(cpuClock, clockHandler );
	    registerAsPrimaryComponent();
	    primaryComponentDoNotEndSim();
	    out->verbose(CALL_INFO, 1, 0, "CPU clock configured for %s\n", cpuClock.c_str());
    }


    if ( m_myPe == 0) printf("num_nodes=%d threadsPerNode=%d activeThreadsPerNode=%d\n",m_numNodes, numThreadsPerNode, m_activeThreadsPerNode);



        cache_link = loadUserSubComponent<Interfaces::SimpleMem>("memory", ComponentInfo::SHARE_NONE, timeConverter, new SimpleMem::Handler<GupsCpu>(this, &GupsCpu::handleEvent) );
        if (!cache_link) {
	    std::string interfaceName = params.find<std::string>("memoryinterface", "memHierarchy.memInterface");
	    out->verbose(CALL_INFO, 1, 0, "Memory interface to be loaded is: %s\n", interfaceName.c_str());

	    Params interfaceParams = params.find_prefix_params("memoryinterfaceparams.");
            interfaceParams.insert("port", "cache_link");
	    cache_link = loadAnonymousSubComponent<Interfaces::SimpleMem>(interfaceName, "memory", 0, ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS, 
                    interfaceParams, timeConverter, new SimpleMem::Handler<GupsCpu>(this, &GupsCpu::handleEvent));
            
            if (!cache_link)
                out->fatal(CALL_INFO, -1, "%s, Error loading memory interface\n", getName().c_str());
        }

	out->verbose(CALL_INFO, 1, 0, "Loaded memory interface successfully.\n");
    m_statAddrIncOp = registerStatistic<uint64_t>("addrIncOp");
    m_statLoopLat = registerStatistic<uint64_t>("loopLatency");
    m_statFenceLat = registerStatistic<uint64_t>("fenceLatency");
    m_statReadLat = registerStatistic<uint64_t>("readLatency");
}

GupsCpu::~GupsCpu() {
	//printf("%s:GupsCpu::%s\n",getName().c_str(),__func__);
	delete out;
}

void GupsCpu::finish() {
	//m_dbg.debug(CALL_INFO,1,0,"\n");
}

void GupsCpu::init(unsigned int phase) {
    cache_link->init(phase);
}

void GupsCpu::handleEvent( Interfaces::SimpleMem::Request* ev) {
    m_shmemQ->handleEvent( ev );
}
#if 0
	SimpleMem::Request::id_t reqID = ev->id;
	assert ( m_pending.find( ev->id ) != m_pending.end() );

    m_dbg.debug(CALL_INFO,1,0,"id=%llu %s\n",
        reqID,ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ? "ReadResp":"WriteResp");

	std::string cmd;
	if ( ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ) { 
		cmd = "ReadResp";

		m_dbg.debug(CALL_INFO,1,0,"ReadResp\n");
		m_pending[ev->id]->resp = ev;			
		m_pending.erase(ev->id);
	} else if ( ev->cmd == Interfaces::SimpleMem::Request::Command::WriteResp ) { 
		cmd = "WriteResp";
		m_dbg.debug(CALL_INFO,1,0,"WriteResp\n");
		delete m_pending[ev->id];
		m_pending.erase(ev->id);
		delete ev;
	} else {
		assert(0);
	}
	//out->verbose(CALL_INFO, 2, 0, "Recv event for processing from interface id=%llu %s\n",reqID,cmd.c_str());
#endif

bool GupsCpu::clockTick(SST::Cycle_t cycle) {

    if ( m_count < m_totalGups ) {
        if ( ! m_shmemQ->full() ) {
            
            int pe   = rng->generateNextUInt64() % m_numPes;
            while ( pe == m_myPe ) {
                pe   = rng->generateNextUInt64() % m_numPes;
            } 

            const uint64_t rand_addr = rng->generateNextUInt64();
            // Ensure we have a reqLength aligned request
            uint64_t addr = (rand_addr % ( m_gupsMemSize / 8 ) ) * 8;

            m_dbg.debug(CALL_INFO,1,0,"dest pe %d, dest addr %" PRIu64 "\n",pe,addr); 
            m_shmemQ->inc( pe, addr );
            ++m_count;
        }
    } else {
        switch ( m_state ) {
          case Quiet:
            m_shmemQ->quiet(&m_quietReq);
            m_state = QuietWait;
            break;
          case QuietWait:
            if ( ! m_quietReq.done ) {
                break;
            }
            if ( m_myPe ==0  ) {
                double startSeconds = 0;
                double endSeconds = getCurrentSimTimeNano() * 1.0e-9;
                double updateTotal = (double) m_totalGups * (double) (m_activeThreadsPerNode * m_numNodes);

                printf("GUpdates    = %25.6f\n", (updateTotal / (1000.0 * 1000.0 * 1000.0) ));
                printf("Time        = %25.6f\n", (endSeconds - startSeconds));
                printf("GUP/s       = %25.6f\n", ((updateTotal / (1000.0 * 1000.0 * 1000.0 )) / (endSeconds - startSeconds)));
            }
            primaryComponentOKToEndSim();
			return true;
        }
    }
    m_shmemQ->process(cycle);
 	
	return false;
}

#include "shmemCmdQ.cc"

#if  0
	switch( m_state ) {
	case Init:
		m_dbg.debug(CALL_INFO,1,0,"Init\n");
        //srand( m_myPe );
		write32( HostTailAddr, 0 );

	case ReadHeadTail:
        m_loopStart =  getCurrentSimTimeNano();
		m_dbg.debug(CALL_INFO,1,0,"ReadHeadTail\n");
		m_readReq = read( HostTailAddr, 4 );
		m_state = WaitRead;	
		break;	

	case WaitRead:

		//printf("%s() WaitRead\n",__func__);
		if ( m_readReq->isDone() ) {
            m_statReadLat->addData(  getCurrentSimTimeNano() - m_loopStart );

			m_tail = m_readReq->data();
			m_dbg.debug(CALL_INFO,1,0,"read of head and tail done head=%d tail=%d\n",m_head,m_tail);
			delete m_readReq;
			m_state = CheckHeadTail;
		} 
		break;

	case CheckHeadTail:
		m_dbg.debug(CALL_INFO,1,0,"CheckHeadTail\n");
		if ( ( m_head + 1 ) % m_qSize  == m_tail ) {
			m_state = ReadHeadTail;
		} else {

            int pe   = rng->generateNextUInt64() % m_numPes;
            while ( pe == m_myPe ) {
                pe   = rng->generateNextUInt64() % m_numPes;
            } 

            const uint64_t rand_addr = rng->generateNextUInt64();
            // Ensure we have a reqLength aligned request
            uint64_t addr = (rand_addr % ( m_gupsMemSize / 8 ) ) * 8;

            m_dbg.debug(CALL_INFO,1,0,"dest pd %d, dest addr %" PRIu64 "\n",pe,addr); 

            NicCmd cmd;
            cmd.type = NicCmd::Shmem;
            cmd.data.shmem.op = ShmemInc;
            cmd.data.shmem.destPe = pe;
            cmd.data.shmem.destAddr = addr;

            m_statAddrIncOp->addData( addr );

            for ( int i = 0; i < sizeof(NicCmd)/8; i++ ) {
			    write64( cmdQAddr(m_head) + i*8, (uint64_t*) &cmd + i );
            }

            m_fenceStart =  getCurrentSimTimeNano();
            m_state = Fence;
            m_nextState = WriteHead;
        }
        break;

    case Fence:
        if ( m_pending.empty() ) {
            m_statFenceLat->addData(  getCurrentSimTimeNano() - m_fenceStart );
            m_state = m_nextState;
        }
        break;

    case WriteHead:
		++m_head;
		m_head %= m_qSize;


        printf("%s %d\n",__func__,m_head);
		write32( NicHeadAddr, m_head );
        m_fenceStart =  getCurrentSimTimeNano();
		++m_count;
#if 0
        if ( m_myPe == 0 ) {
            printf("%d %d\n",m_count, m_myPe);
        }
#endif
		if ( m_count == m_totalGups ) {
            m_dbg.debug(CALL_INFO,1,0,"set state to Finish\n");
            m_state = Fence;
		    m_nextState = Quiet;
		} else {
            m_statLoopLat->addData(  getCurrentSimTimeNano() - m_loopStart );
			m_state = ReadHeadTail;
		}
		break;

	case Quiet:
        m_dbg.debug(CALL_INFO,1,0,"Quiet\n");
		m_readReq = read( HostPendingCntAddr, 8 );
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
#if 0 
        printf("%d pending count is 0, we can finish\n",m_myPe);
#endif
		m_dbg.debug(CALL_INFO,1,0,"pending count is 0, we can finish\n");
		delete m_readReq;
		m_state = Finish;
        // fall through

	case Finish:
		if ( m_pending.empty() ) {
            if ( m_myPe ==0  ) {
                double startSeconds = 0;
                double endSeconds = getCurrentSimTimeNano() * 1.0e-9;
                double updateTotal = (double) m_totalGups * (double) (m_activeThreadsPerNode * m_numNodes);
//                double updateTotal = (double) m_totalGups * (double) m_numPes - 32;

                printf("GUpdates    = %25.6f\n", (updateTotal / (1000.0 * 1000.0 * 1000.0) ));
                printf("Time        = %25.6f\n", (endSeconds - startSeconds));
                printf("GUP/s       = %25.6f\n", ((updateTotal / (1000.0 * 1000.0 * 1000.0 )) / (endSeconds - startSeconds)));
            }
            //SimTime_t x = getCurrentSimTimeNano();
			//printf("Finish Total updates %d, time=%f, %f GUPS\n", m_totalGups, x/1000000000.0, (double) m_totalGups/ x );
            primaryComponentOKToEndSim();
			return true;
		}
		break;
	}
#endif

