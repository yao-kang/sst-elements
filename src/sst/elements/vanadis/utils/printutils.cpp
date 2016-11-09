
#include "utils/printutils.hpp"

void SST::Vanadis::binaryStringize64(const uint64_t v, char* buffer) {
        uint64_t vCopy = v;

        for(int i = 63; i >= 0; --i) {
                buffer[i] = (vCopy & 1) ? '1' : '0';
                vCopy >>= 1;
        }
}

void SST::Vanadis::binaryStringize32(const uint32_t v, char* buffer) {
        uint32_t vCopy = v;

        for(int i = 31; i >= 0; --i) {
                buffer[i] = (vCopy & 1) ? '1' : '0';
                vCopy >>= 1;
        }
}


void SST::Vanadis::binaryStringize8(const char v, char* buffer) {
        char vCopy = v;

        for(int i = 7; i >= 0; --i) {
                buffer[i] = (vCopy & 1) ? '1' : '0';
                vCopy >>= 1;
        }
}
