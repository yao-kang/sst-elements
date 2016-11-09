

#ifndef _H_SST_VANADIS_PRINT_UTILS
#define _H_SST_VANADIS_PRINT_UTILS

#include <stdlib.h>
#include <limits.h>

namespace SST {
namespace Vanadis {

void binaryStringize64(const uint64_t v, char* buffer);
void binaryStringize8(const char v, char* buffer);
void binaryStringize32(const uint32_t, char* buffer);

}
}

#endif
