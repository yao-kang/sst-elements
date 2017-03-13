#include <cstdio>
#include <cstdlib>

#include "ssttoken.h"
#include "ssttokenizer.h"

using namespace SST::Ember;

int main(int argc, char* argv[]) {

	FILE* theFile = NULL;
	theFile = fopen(argv[1], "r");

	SSTEmberTokenizer* tokenizer = new SSTEmberTokenizer(theFile);
	SSTEmberToken* token = tokenizer->getNextToken();

	while( ! tokenizer->finished() ) {
		printf("Token: \'%s\'\n", token->getTokenString().c_str());

		delete token;
		token = tokenizer->getNextToken();
	}

	fclose(theFile);
}
