struct NicCmd {
    enum Type { Shmem=1, Fam } type;
    uint32_t handle;
    uint32_t numData;
    uint64_t respAddr;
    union Data {
        struct Shmem {
            enum Op { Quiet=1, Inc } op;
            int pe;
            uint64_t srcAddr;
            uint64_t destAddr;
        } shmem;
        struct Fam {
            enum Op { Get=1, Put  } op;
            int srcPid;
            int destNode;
            uint64_t destAddr;
        } fam;
    } data;
    uint64_t value;
    SST::SimTime_t timeStamp; // 8 bytes is need to pad to 64 bytes so we use it for statistics 
};

struct NicResp {
    uint32_t handle;
    uint32_t retval; // not currently used
    uint64_t value;
    uint64_t pad; 
    uint64_t timeStamp;
};
