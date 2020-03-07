struct NicCmd {
    enum Type { Shmem=1, Fam } type;
    uint64_t respAddr;
    uint32_t handle;
    union Data {
        struct Shmem {
            enum Op { Quiet=1, Inc }   op;
            int pe;
            uint64_t srcAddr;
            uint64_t destAddr;
        } shmem;
        struct Fam {
            int op;
            int srcPid;
            int destNode;
            uint64_t destAddr;
            uint64_t value;
        } fam;
    } data;
    uint64_t pad;
};

struct NicResp {
    uint32_t handle;
    uint32_t retval; // not currently used
    uint64_t value;
};
