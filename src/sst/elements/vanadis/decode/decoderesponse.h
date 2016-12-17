
#ifndef _H_SST_VANADIS_DECODE_RESPONSE
#define _H_SST_VANADIS_DECODE_RESPONSE

namespace SST {
namespace Vanadis {

enum VanadisDecodeResponse {
	SUCCESS,
	UNKNOWN_REGISTER,
	ICACHE_FILL_FAILED,
	UNKNOWN_INSTRUCTION
};

}
}

#endif
