
template< class T>
ShmemQueue<T>::ShmemQueue( T* cpu, Params& params ) : m_cpu( cpu ), 
    m_head(0),m_respTail(0), m_state(ReadHeadTail),m_activeReq(NULL), m_handle(0),m_respRead(NULL)
{
    m_qSize = params.find<int>("cmdQSize", 64);
    m_respQsize = params.find<int>("respQSize", params.find<int>("cmdQSize", 64) );
    nicBaseAddr = params.find<uint64_t>( "nicBaseAddr", 0x100000000 );


    uint64_t hostQueueInfoBaseAddr = params.find<uint64_t>("hostQueueInfoBaseAddr", 0 );
    size_t hostQueueInfoSizePerPe = params.find<size_t>("hostQueueInfoSizePerPe", 64 );
    uint64_t hostQueueBaseAddr = params.find<uint64_t>("hostQueueBaseAddr", 0x10000 );
    size_t hostQueueSizePerPe = params.find<size_t>("hostQueueSizePerPe", 4096 );
    m_blockSize = params.find<uint64_t>("famBlockSize", 64);
    m_numFamNodes = params.find<uint64_t>("numFamNodes", 1);
    m_firstFamNode = params.find<uint64_t>("firstFamNode", 1);

    nicMemLength = m_qSize * sizeof(NicCmd) + 64;
    nicMemLength = ((nicMemLength-1u) & ~(4096-1u)) + 4096;

    nicCmdQAddr = nicBaseAddr  + m_cpu->myThread() * nicMemLength; 
    hostQueueInfoAddr = hostQueueInfoBaseAddr + m_cpu->myThread() * hostQueueInfoSizePerPe;
    hostQueueAddr = hostQueueBaseAddr + m_cpu->myThread() *  hostQueueSizePerPe;

    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"cmdQSize=%d respQSize=%d\n", m_qSize, m_respQsize );
    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"nicCmdQAddr=%#" PRIx64 " sizerPerThread=%zu\n",  nicCmdQAddr, nicMemLength );
    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"hostQueueInfoAddr=%#" PRIx64 " hostQueueInfoSizePerPe=%zu\n", hostQueueInfoAddr, hostQueueInfoSizePerPe );
    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"hostQueueAddr=%#" PRIx64 " hostQueueSizePerPe=%zu\n",  hostQueueAddr, hostQueueSizePerPe);
}

template< class T>
void ShmemQueue<T>::inc( int pe, uint64_t addr )
{
    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_CMD_FLAG,"pe=%d addr=%#" PRIx64 "\n", pe, addr );

    Cmd* cmd = new Cmd(NULL);
    cmd->cmd.type = NicCmd::Shmem;
    cmd->cmd.numData = 1;
    cmd->cmd.data.shmem.op = NicCmd::Data::Shmem::Op::Inc; 
    cmd->cmd.data.shmem.pe = pe;
    cmd->cmd.data.shmem.destAddr = addr;
    m_cmdQ.push( cmd );
}

template< class T> 
void ShmemQueue<T>::quiet( ShmemReq* req) {

    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_CMD_FLAG,"\n");

    Cmd* cmd = new Cmd(req);
    cmd->cmd.type = NicCmd::Shmem;
    cmd->cmd.data.shmem.op = NicCmd::Data::Shmem::Op::Quiet; 
    m_cmdQ.push( cmd );
}

template< class T> 
void ShmemQueue<T>::get( uint64_t dstAddr, uint64_t srcAddr, ShmemReq* req ) {
    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_CMD_FLAG,"dstAddr=%#" PRIx64 " srcAddr=%#" PRIx64 " %p\n", dstAddr, srcAddr, req );

    Cmd* cmd = new Cmd(req,ShmemReq::FamGet);
    cmd->cmd.type = NicCmd::Fam;
    cmd->cmd.numData = 1;
    cmd->cmd.data.fam.op = NicCmd::Data::Fam::Op::Get; 
    cmd->cmd.data.fam.destNode = calcNode( srcAddr );
    cmd->cmd.data.fam.destAddr = calcAddr( srcAddr ); 
     
    m_cmdQ.push( cmd );
}

template< class T> 
void ShmemQueue<T>::put( uint64_t dstAddr, uint64_t srcAddr, ShmemReq* req ) {
    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_CMD_FLAG,"dstAddr=%#" PRIx64 " srcAddr=%#" PRIx64 "\n", dstAddr, srcAddr );

    Cmd* cmd = new Cmd(req);
    cmd->cmd.type = NicCmd::Fam;
    cmd->cmd.numData = 1;
    cmd->cmd.data.fam.op = NicCmd::Data::Fam::Op::Put; 
    cmd->cmd.data.fam.destNode = calcNode( dstAddr );
    cmd->cmd.data.fam.destAddr = calcAddr( dstAddr ); 

    m_cmdQ.push( cmd );
}

template< class T> 
void ShmemQueue<T>::handleEvent( Interfaces::SimpleMem::Request* ev) {
    SimpleMem::Request::id_t reqID = ev->id;

    assert ( m_pending.find( ev->id ) != m_pending.end() );

//    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"id=%llu %s\n", reqID,ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ? "ReadResp":"WriteResp");

    std::string cmd;
    if ( ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ) { 
        m_cpu->dbg().debug(CALL_INFO,2,DBG_SHMEM_FLAG,"ReadResp\n");
        m_pending[ev->id]->resp = ev;			
        m_pending.erase(ev->id);
    } else if ( ev->cmd == Interfaces::SimpleMem::Request::Command::WriteResp ) { 
        m_cpu->dbg().debug(CALL_INFO,2,DBG_SHMEM_FLAG,"WriteResp\n" );
        if ( m_pending[ev->id]->keep ) {
            m_cpu->dbg().debug(CALL_INFO,2,DBG_SHMEM_FLAG,"WriteResp keep\n" );
            m_pending[ev->id]->resp = ev;			
        } else {
            m_cpu->dbg().debug(CALL_INFO,2,DBG_SHMEM_FLAG,"WriteResp delete\n" );
            delete m_pending[ev->id];
            delete ev;
        }
        m_pending.erase(ev->id);
    } else {
        assert(0);
    }
}


template< class T> 
bool ShmemQueue<T>::process( Cycle_t cycle ){

    if ( ! m_pendingReq.empty() ) {
        checkForResp();
    }

    if ( NULL == m_activeReq ) {
        if ( m_cmdQ.empty() ) {
            return false;
        } 
       	m_activeReq = m_cmdQ.front();
#if 0
		m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"new cmd %d remoteNode %d, remoteAddr %" PRIx64 "\n",m_activeReq->op,m_activeReq->node, m_activeReq->remoteAddr);
#endif
        m_cmdQ.pop();
        m_state = ReadHeadTail;
    }

    
    switch ( m_state ) {

      case ReadHeadTail:
        m_cpu->dbg().debug(CALL_INFO,2,DBG_SHMEM_FLAG,"ReadHeadTail\n");
        m_cmdTailRead = read( hostQueueInfoAddr, 4 );
        m_state = WaitRead;	
		break;	

      case WaitRead:

		if ( m_cmdTailRead->isDone() ) {
			m_tail = m_cmdTailRead->data();
			m_cpu->dbg().debug(CALL_INFO,2,DBG_SHMEM_FLAG,"read of head and tail done head=%d tail=%d\n",m_head,m_tail);
			delete m_cmdTailRead;
			m_state = CheckHeadTail;
		} 
		break;

	case CheckHeadTail:
		m_cpu->dbg().debug(CALL_INFO,2,DBG_SHMEM_FLAG,"CheckHeadTail m_head=%d m_tail=%d\n",m_head,m_tail);
		if ( ( m_head + 1 ) % m_qSize  == m_tail ) {
			m_state = ReadHeadTail;
		} else {

            NicCmd& cmd = m_activeReq->cmd;
            cmd.handle = genHandle();

            cmd.timeStamp = m_cpu->getCurrentSimTimeNano();

            m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"Write command handle=%d %p\n",cmd.handle, m_activeReq->req);
            m_cmdWrite = write( cmdQAddr(m_head), reinterpret_cast<uint8_t*>(&cmd), sizeof(cmd), true );
            m_activeReq->req->startTime = m_cpu->getCurrentSimTimeNano();

            m_state = WaitWrite;
        }
        break;

    case WaitWrite:
        // wait for the write to complete so commands land in NIC memory in order
        if ( ! m_cmdWrite->isDone() ) {
            break;
        }
        delete m_cmdWrite;

    case CmdDone:
		++m_head;
		m_head %= m_qSize;
        m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"CmdDone handle=%d\n",m_activeReq->cmd.handle);
        if ( m_activeReq->req ) {
            m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"non-blocking handle=%d\n", m_activeReq->cmd.handle);
            m_pendingReq[ m_activeReq->cmd.handle ] = m_activeReq->req;
        }
        delete m_activeReq;
        m_activeReq = NULL;
        break;
    }
    return false;
}

template< class T>
void ShmemQueue<T>::checkForResp(  ){
    if ( NULL == m_respRead  ) {
        m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"read response Q head\n");
        // read head pointer
        m_respRead = read( hostQueueInfoAddr + 4, 4 );
        m_respState = RespState::WaitReadHead;
    } else {

        switch ( m_respState ) {       
            case RespState::WaitReadHead: {
                if ( ! m_respRead->isDone() ) {
                    break;
                }
                int head = m_respRead->data();
                delete m_respRead;
                if ( head != m_respTail ) {
                    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"head=%d != tail=%d read cmd at %#" PRIx64 "\n",
                                                head, m_respTail, hostQueueAddr + sizeof(NicResp) * m_respTail);
                    // read the command
                    m_respRead = read( hostQueueAddr + sizeof(NicResp) * m_respTail, sizeof(NicResp) );
                    ++m_respTail; 
                    m_respTail %= m_respQsize;
                    // write the tail pointer 
                    write( nicCmdQAddr + nicMemLength - 4, reinterpret_cast<uint8_t*>(&m_respTail), 4 );
                    m_respState = RespState::WaitReadCmd;

                } else {
                    m_respRead = read( hostQueueInfoAddr + 4, 4 );
                    break;
                }
            }
            case RespState::WaitReadCmd:
                if ( ! m_respRead->isDone() ) {
                    break;
                }

                NicResp resp;
                m_respRead->copyData( &resp, sizeof(resp) );
                delete m_respRead;
                m_respRead = NULL;

                 
                try {
                    auto tmp = m_pendingReq.at( resp.handle );
                    m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_FLAG,"got response body, handle=%d timeStamp=%" PRIu64 " startTime=%" PRIu64 " %p\n",
                            resp.handle, resp.timeStamp, tmp->startTime, tmp );
                    m_cpu->addData( ShmemReq::Type::Foobar, m_cpu->getCurrentSimTimeNano() - resp.timeStamp );
                    tmp->done = true; 
                    m_cpu->addData( tmp->type, m_cpu->getCurrentSimTimeNano() - tmp->startTime );
                } catch (const std::out_of_range& oor) {
                    assert(0);  
                }
                m_pendingReq.erase( resp.handle );
                break; 
        }
    }
}

