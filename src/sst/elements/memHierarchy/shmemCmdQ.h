
struct ShmemReq {
    ShmemReq(): done(true) {}
    bool done;
};

struct MyRequest {
    MyRequest() : resp( NULL ) { }
    ~MyRequest() {
        delete resp;
    }
    bool isDone() { return resp != NULL; }
    SimpleMem::Request* resp;

    void copyData( void* dest, size_t length ) {
        assert( length <= resp->data.size() );
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
        Cmd( ShmemReq* req = NULL ) : req(req) {}
        NicCmd cmd;
        ShmemReq* req; 
    };

  public:
    ShmemQueue( T* cpu, int qSize, int respQsize, uint64_t nicBaseAddr, size_t nicMemLength, uint64_t hostQueueInfoBaseAddr, 
            size_t hostQueueInfoSizePerPe, uint64_t hostQueueBaseAddr, size_t hostQueueSizePerPe );

    bool process( Cycle_t );
    bool full() { return m_cmdQ.size() == 16; } 

    void get( uint64_t, uint64_t, ShmemReq* );
    void put( uint64_t, uint64_t, ShmemReq* );
    void quiet( ShmemReq* );
    void inc( int pe, uint64_t addr );
    void handleEvent( SimpleMem::Request* ev );

  private:

    enum State { ReadHeadTail, WaitRead, CheckHeadTail, Fence, CmdDone  } m_state;
    enum RespState { WaitReadHead, WaitReadCmd } m_respState;

    std::map<SimpleMem::Request::id_t, MyRequest*> m_pending;

    uint64_t cmdQAddr( int pos ) {
        return nicCmdQAddr + pos * sizeof(NicCmd);
    }

    MyRequest* write( uint64_t addr, uint8_t* data, int num ) {

        SimpleMem::Request* req = new SimpleMem::Request( SimpleMem::Request::Write, addr, num );
        if ( notCached( addr ) ) {
            req->flags = SimpleMem::Request::F_NONCACHEABLE;
        }

        m_cpu->m_dbg.debug(CALL_INFO,1,0,"id=%" PRIu64 " addr=%#" PRIx64 " num=%d\n",req->id, addr, num);

        for ( int i = 0; i < num; i++ ) {
            req->data.push_back( data[i] );
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
        m_cpu->m_dbg.debug(CALL_INFO,1,0,"id=%" PRIu64 " addr=0x%" PRIx64 " num=%d\n",req->id, addr, size);
        m_cpu->cache_link->sendRequest(req);
        return m_pending[ req->id ];
    }

    bool notCached( uint64_t addr ) {
        return  addr >= nicBaseAddr  && addr < nicBaseAddr + nicMemLength;
    }

    uint32_t genHandle() { return m_handle++; }

    void checkForResp();

    std::queue<Cmd*> m_cmdQ;
    T* m_cpu;
    uint32_t m_head;
    uint32_t m_tail;

    uint32_t m_respHead;

    int      m_qSize;
    int      m_respQsize;
    uint64_t nicBaseAddr;
    size_t   nicMemLength;
    uint64_t hostQueueInfoBaseAddr;
    size_t   hostQueueInfoSizePerPe;
    uint64_t hostQueueBaseAddr;
    size_t   hostQueueSizePerPe;
    uint64_t nicCmdQAddr;
    Cmd*     m_activeReq;
    int      m_handle;

    std::map< uint32_t, ShmemReq* > m_pendingReq;
    MyRequest* m_cmdTailRead;
    MyRequest* m_respRead;
};

