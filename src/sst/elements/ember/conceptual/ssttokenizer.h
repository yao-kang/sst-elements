
#include <cstring>
#include <cstdio>
#include "ssttoken.h"

namespace SST {
namespace Ember {

class SSTEmberTokenizer {

public:
	SSTEmberTokenizer(FILE* file);
	~SSTEmberTokenizer();

	bool finished();
	SSTEmberToken* getNextToken();

protected:
	FILE* file;
	SSTEmberToken* nextToken;
	char* lexBuffer;
	bool reachedEOF;

	size_t lexBufferMaxSize;
	size_t currentBufferIndex;

	void consumeToLineEnd();
	void consumeBufferWhitespace(size_t* buffIdx);

	void readNextLine();
	void resizeBuffer();

};

}
}
