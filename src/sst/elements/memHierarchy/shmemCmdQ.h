
#define DBG_SHMEM_FLAG (1<<0)
#define DBG_SHMEM_LL_FLAG (1<<1)
#define DBG_SHMEM_CMD_FLAG (1<<2)
#define DBG_APP_FLAG (1<<3)

struct ShmemReq {
    ShmemReq(): done(true), type(Invalid) {}
    bool done;
    
    enum Type { FamGet, Foobar, Total, Invalid=Total } type;
    uint64_t startTime;
};

struct MyRequest {
    MyRequest( bool keep = false ) : resp( NULL ), keep(keep) { }
    ~MyRequest() {
        if ( resp ) {
            delete resp;
        }
    }
    bool isDone() { return resp != NULL; }
    SimpleMem::Request* resp;
    bool keep;

    void copyData( void* dest, size_t length ) {
        assert( length == resp->data.size() );
        memcpy( dest, resp->data.data(), length );
    }

    uint64_t data() {
        uint64_t tmp = 0;
        for ( int i = 0; i < resp->data.size(); i++) {
            tmp |= resp->data[i] << i * 8;
        }
        //printf("%s() size=%zu data=0x%" PRIx64 "\n",__func__, resp->data.size(), tmp);
        return tmp;
    }
};

template < class T > 
class ShmemQueue {

    struct Cmd {
        Cmd( ShmemReq* req, ShmemReq::Type type = ShmemReq::Type::Invalid ) : req(req) { bzero(&cmd, sizeof(NicCmd) ); 
            if ( req ) {
                req->done = false;
                req->type = type;
            }
        }
        NicCmd cmd;
        ShmemReq* req; 
    };

  public:
    ShmemQueue( T* cpu, Params& Params );

    bool process( Cycle_t );
    bool full() { return m_cmdQ.size() == 16; } 

    void get( uint64_t, uint64_t, ShmemReq* );
    void put( uint64_t, uint64_t, ShmemReq* );
    void quiet( ShmemReq* );
    void inc( int pe, uint64_t addr );
    void handleEvent( SimpleMem::Request* ev );
	void printInfo() {
		printf("pending=%zu %zu %zu\n",m_pending.size(),m_cmdQ.size(), m_pendingReq.size());
	}

  private:

    enum State { ReadHeadTail, WaitRead, CheckHeadTail, WaitWrite, CmdDone  } m_state;
    enum RespState { WaitReadHead, WaitReadCmd } m_respState;

    std::map<SimpleMem::Request::id_t, MyRequest*> m_pending;

    uint64_t cmdQAddr( int pos ) {
        return nicCmdQAddr + pos * sizeof(NicCmd);
    }

    MyRequest* write( uint64_t addr, uint8_t* data, int num, bool keep = false ) {

        SimpleMem::Request* req = new SimpleMem::Request( SimpleMem::Request::Write, addr, num );
        if ( notCached( addr ) ) {
            req->flags = SimpleMem::Request::F_NONCACHEABLE;
        }

#if 0
        std::ostringstream tmp(std::ostringstream::ate);
        tmp << std::hex;
#endif
    
        for ( int i = 0; i < num; i++ ) {
            req->data.push_back( data[i] );
#if 0
            tmp << "0x" << (int) data[i] << ",";
#endif
        }

        m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_LL_FLAG,"id=%" PRIu64 " addr=%#" PRIx64 " num=%d %s\n",req->id, addr, num, notCached(addr) ? "":"cached");

        m_pending[ req->id ] = new MyRequest(keep);
        m_cpu->sendRequest(req);
        return m_pending[ req->id ];
    }

    MyRequest* read( uint64_t addr, int size ) {
        SimpleMem::Request* req = new SimpleMem::Request( SimpleMem::Request::Read, addr, size );
        if ( notCached( addr ) ) {
            req->flags = SimpleMem::Request::F_NONCACHEABLE;
        }
        m_pending[ req->id ] = new MyRequest;
        m_cpu->dbg().debug(CALL_INFO,1,DBG_SHMEM_LL_FLAG,"id=%" PRIu64 " addr=0x%" PRIx64 " num=%d %s\n",req->id, addr, size, notCached(addr) ? "":"cached" );
        m_cpu->sendRequest(req);
        return m_pending[ req->id ];
    }

    bool notCached( uint64_t addr ) {
        return  addr >= nicBaseAddr  && addr < nicBaseAddr + nicMemLength * m_cpu->threadsPerNode();
    }

    size_t m_blockSize;
    int    m_numFamNodes;
    int    m_firstFamNode;

    size_t calcBlockNum( uint64_t addr ) {
        return addr/m_blockSize;
    }

    int calcNode( uint64_t addr ) {
        uint64_t globalBlockNum = calcBlockNum(addr);
        
        int node = (globalBlockNum % m_numFamNodes) + m_firstFamNode;
#if 0
        printf("%s() gBlock=%" PRIu64" node=%d\n",__func__,globalBlockNum, node);
#endif
        return node;
    }

    uint64_t calcAddr( uint64_t addr ) {
        uint64_t globalBlockNum = calcBlockNum(addr);
        uint64_t nodeBlockNum = globalBlockNum / m_numFamNodes;  
        uint64_t tmp = addr & (m_blockSize - 1);
        uint64_t nodeAddr = nodeBlockNum * m_blockSize + tmp;
#if 0
        printf("%s() gBlock=%" PRIu64 " nodeBLock=%" PRIu64 " addr=%#" PRIx64" %#" PRIx64 " nodeAddr=%#" PRIx64 "\n",
            __func__, globalBlockNum, nodeBlockNum, addr, tmp, nodeAddr );
#endif
        return nodeAddr;
    }

    uint32_t genHandle() { return m_handle++; }

    void checkForResp();

    std::queue<Cmd*> m_cmdQ;
    T* m_cpu;
    uint32_t m_head;
    uint32_t m_tail;

    uint32_t m_respTail;

    int      m_qSize;
    int      m_respQsize;
    size_t   nicMemLength;
    uint64_t hostQueueInfoAddr;
    uint64_t hostQueueAddr;

    uint64_t nicBaseAddr;
    uint64_t nicCmdQAddr;
    Cmd*     m_activeReq;
    int      m_handle;
    MyRequest* m_cmdWrite;
    MyRequest* m_cmdTailRead;

    std::map< uint32_t, ShmemReq* > m_pendingReq;
    MyRequest* m_respRead;
};

#include "shmemCmdQ.cc"
