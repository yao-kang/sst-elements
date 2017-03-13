
#ifndef _H_SST_EMBER_TOKEN
#define _H_SST_EMBER_TOKEN

#include <cstring>
#include <cstdlib>
#include <string>

namespace SST {
namespace Ember {

class SSTEmberToken {

public:
	SSTEmberToken(char* buff) {
		const size_t len = strlen(buff);
		token = (char*) malloc( sizeof(char) * (len + 1) );

		for( size_t i = 0; i < len; ++i ) {
			token[i] = buff[i];
		}

		token[len] = '\0';
	}

	~SSTEmberToken() {
		free(token);
	}

	bool matches(char* comp) {
		return (std::strcmp(token, comp) == 0);
	}

	std::string getTokenString() {
		std::string tokenStr(token);
		return tokenStr;
	}

protected:
	char* token;

};

}
}

#endif
