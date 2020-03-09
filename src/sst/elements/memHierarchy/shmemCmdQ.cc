
template< class T> 
GupsCpu::ShmemQueue<T>::ShmemQueue( T* cpu, int qSize, int respQsize, 
                    uint64_t nicBaseAddr, size_t nicMemLength,
                    uint64_t hostQueueInfoBaseAddr, size_t hostQueueInfoSizePerPe,
                    uint64_t hostQueueBaseAddr, size_t hostQueueSizePerPe ) :
	m_cpu(cpu), m_qSize(qSize), m_respQsize(respQsize), 
    nicBaseAddr(nicBaseAddr), nicMemLength(nicMemLength), 
    m_head(0),m_respHead(0),m_state(ReadHeadTail),m_activeReq(NULL), m_handle(0),m_respRead(NULL)
{
    nicCmdQAddr = nicBaseAddr  + ( m_cpu->m_myPe % m_cpu->m_threadsPerNode ) * nicMemLength;
    hostQueueInfoAddr = hostQueueInfoBaseAddr + ( m_cpu->m_myPe % m_cpu->m_threadsPerNode ) * hostQueueInfoSizePerPe;
    hostQueueAddr = hostQueueBaseAddr + ( m_cpu->m_myPe % m_cpu->m_threadsPerNode ) *  hostQueueSizePerPe;

    m_cpu->m_dbg.debug(CALL_INFO,1,0,"cmdQSize=%d respQSize=%d\n", m_qSize, m_respQsize );
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"nicCmdQAddr=%#" PRIx64 " sizerPerThread=%zu\n",  nicCmdQAddr, nicMemLength );
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"hostQueueInfoAddr=%#" PRIx64 " hostQueueInfoSizePerPe=%zu\n", hostQueueInfoAddr, hostQueueInfoSizePerPe );
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"hostQueueAddr=%#" PRIx64 " hostQueueSizePerPe=%zu\n",  hostQueueAddr, hostQueueSizePerPe);
}

template< class T>
void GupsCpu::ShmemQueue<T>::inc( int pe, uint64_t addr )
{
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"pe=%d addr=%#" PRIx64 "\n", pe, addr );
    Cmd* cmd = new Cmd();
    cmd->cmd.type = NicCmd::Shmem;
    cmd->cmd.data.shmem.op = NicCmd::Data::Shmem::Op::Inc; 
    cmd->cmd.data.shmem.pe = pe;
    cmd->cmd.data.shmem.destAddr = addr;
    m_cmdQ.push( cmd );
}

template< class T> 
void GupsCpu::ShmemQueue<T>::quiet( ShmemReq* req) {
    req->done = false;
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"\n");
    Cmd* cmd = new Cmd(req);
    cmd->cmd.type = NicCmd::Shmem;
    cmd->cmd.data.shmem.op = NicCmd::Data::Shmem::Op::Quiet; 
    m_cmdQ.push( cmd );
}

template< class T> 
void GupsCpu::ShmemQueue<T>::get( uint64_t dstAddr, uint64_t srcAddr, ShmemReq* req ) {
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"srcAddr=0x%" PRIx64 "\n", srcAddr );
    req->done = false;	
    int remoteNode = m_cpu->calcNode( srcAddr );
    uint64_t remoteAddr = m_cpu->calcAddr( srcAddr ); 
    m_cmdQ.push( new Cmd( Cmd::FamGet, remoteNode, remoteAddr, dstAddr, req ) );
}

template< class T> 
void GupsCpu::ShmemQueue<T>::put( uint64_t dstAddr, uint64_t srcAddr, ShmemReq* req ) {
    m_cpu->m_dbg.debug(CALL_INFO,1,0,"dstAddr=0x%" PRIx64 "\n", dstAddr );
    req->done = false;	

    int remoteNode = m_cpu->calcNode( dstAddr );
    uint64_t remoteAddr = m_cpu->calcAddr( dstAddr ); 
    m_cmdQ.push( new Cmd( Cmd::FamPut, remoteNode, remoteAddr, srcAddr, req ) );
}

template< class T> 
void GupsCpu::ShmemQueue<T>::handleEvent( Interfaces::SimpleMem::Request* ev) {
    SimpleMem::Request::id_t reqID = ev->id;

    assert ( m_pending.find( ev->id ) != m_pending.end() );

//    m_cpu->m_dbg.debug(CALL_INFO,1,0,"id=%llu %s\n", reqID,ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ? "ReadResp":"WriteResp");

    std::string cmd;
    if ( ev->cmd == Interfaces::SimpleMem::Request::Command::ReadResp ) { 
//        m_cpu->m_dbg.debug(CALL_INFO,1,0,"ReadResp\n");
        m_pending[ev->id]->resp = ev;			
        m_pending.erase(ev->id);
    } else if ( ev->cmd == Interfaces::SimpleMem::Request::Command::WriteResp ) { 
//        m_cpu->m_dbg.debug(CALL_INFO,1,0,"WriteResp\n");
        delete m_pending[ev->id];
        m_pending.erase(ev->id);
        delete ev;
    } else {
        assert(0);
    }
}


template< class T> 
bool GupsCpu::ShmemQueue<T>::process( Cycle_t cycle ){

    if ( ! m_pendingReq.empty() ) {
        checkForResp();
    }

    if ( NULL == m_activeReq ) {
        if ( m_cmdQ.empty() ) {
            return false;
        } 
       	m_activeReq = m_cmdQ.front();
#if 0
		m_cpu->m_dbg.debug(CALL_INFO,1,0,"new cmd %d remoteNode %d, remoteAddr %" PRIx64 "\n",m_activeReq->op,m_activeReq->node, m_activeReq->remoteAddr);
#endif
        m_cmdQ.pop();
        m_state = ReadHeadTail;
    }

    
    switch ( m_state ) {

      case ReadHeadTail:
        m_cpu->m_dbg.debug(CALL_INFO,1,0,"ReadHeadTail\n");
        m_cmdTailRead = read( hostQueueInfoAddr, 4 );
        m_state = WaitRead;	
		break;	

      case WaitRead:

		if ( m_cmdTailRead->isDone() ) {
			m_tail = m_cmdTailRead->data();
			m_cpu->m_dbg.debug(CALL_INFO,1,0,"read of head and tail done head=%d tail=%d\n",m_head,m_tail);
			delete m_cmdTailRead;
			m_state = CheckHeadTail;
		} 
		break;

	case CheckHeadTail:
		m_cpu->m_dbg.debug(CALL_INFO,1,0,"CheckHeadTail\n");
		if ( ( m_head + 1 ) % m_qSize  == m_tail ) {
			m_state = ReadHeadTail;
		} else {

            NicCmd& cmd = m_activeReq->cmd;
            cmd.handle = genHandle();

            m_cpu->m_dbg.debug(CALL_INFO,1,0,"Write command\n");
            write( cmdQAddr(m_head), reinterpret_cast<uint8_t*>(&cmd), sizeof(cmd) );

            m_state = Fence;
        }
        break;
    case Fence:
        if ( ! m_pending.empty() ) {
            break;
        }

      case CmdDone:
		++m_head;
		m_head %= m_qSize;
        m_cpu->m_dbg.debug(CALL_INFO,1,0,"CmdDone\n");
        if ( m_activeReq->req ) {
            m_cpu->m_dbg.debug(CALL_INFO,1,0,"non-blocking\n");
            m_pendingReq[ m_activeReq->cmd.handle ] = m_activeReq->req;
        }
        delete m_activeReq;
        m_activeReq = NULL;
        break;
    }
    return false;
}

template< class T>
void GupsCpu::ShmemQueue<T>::checkForResp(  ){
    if ( NULL == m_respRead  ) {
        m_cpu->m_dbg.debug(CALL_INFO,1,0,"read response Q head\n");
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
                if ( head != m_respHead ) {
                    m_cpu->m_dbg.debug(CALL_INFO,1,0,"head has changed %d\n",head);
                    m_respRead = read( hostQueueAddr + sizeof(NicResp) * m_respHead, sizeof(NicResp) );
                    ++m_respHead; 
                    m_respHead %= m_respQsize;

                    m_respState = RespState::WaitReadCmd;
                } else {
                     m_respRead = read( hostQueueAddr + 4, 4 );
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

                m_cpu->m_dbg.debug(CALL_INFO,1,0,"handle=%d\n",resp.handle);
                 
                try {
                    auto tmp = m_pendingReq.at( resp.handle );
                    tmp->done = true; 
                } catch (const std::out_of_range& oor) {
                    assert(0);  
                }
                m_pendingReq.erase( resp.handle );
                break; 
        }
    }
}
