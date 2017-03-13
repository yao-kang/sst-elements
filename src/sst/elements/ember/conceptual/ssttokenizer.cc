
#include "ssttokenizer.h"

#include <cstdlib>
#include <cctype>

using namespace SST::Ember;

SSTEmberTokenizer::SSTEmberTokenizer(FILE* theFile) :
	currentBufferIndex(0), file(theFile), reachedEOF(false) {

	#define DEFAULT_BUFFER_SIZE 128

	lexBuffer = (char*) malloc( sizeof(char) * DEFAULT_BUFFER_SIZE );

	for(size_t i = 0; i < DEFAULT_BUFFER_SIZE; ++i) {
		lexBuffer[i] = '\0';
	}

	lexBufferMaxSize = DEFAULT_BUFFER_SIZE;

	// Start the parser off with the first line of processing
	readNextLine();

	#undef DEFAULT_BUFFER_SIZE
}

SSTEmberTokenizer::~SSTEmberTokenizer() {
	free(lexBuffer);
	fclose(file);
}

bool SSTEmberTokenizer::finished() {
	return reachedEOF;
}

void SSTEmberTokenizer::readNextLine() {
	int tmp = fgetc(file);

	if(EOF == tmp) {
		reachedEOF = true;
		return;
	}

	size_t bufferIndex = 0;

	while( (tmp != EOF) &&
		(tmp != '\n') &&
		(tmp != '\r')) {

		if( bufferIndex == lexBufferMaxSize ) {
			resizeBuffer();
		}

		lexBuffer[bufferIndex] = static_cast<char>(tmp);
		bufferIndex++;

		tmp = fgetc(file);
	}

	if( bufferIndex == lexBufferMaxSize ) {
		resizeBuffer();
	}

	lexBuffer[bufferIndex] = '\0';
	bufferIndex++;

	currentBufferIndex = 0;

	// Check we have not just read a comment into the buffer, if we have we need to read another line

	size_t tmpIndex = 0;

	while( tmpIndex < lexBufferMaxSize && std::isspace( static_cast<int>(lexBuffer[tmpIndex]) ) != 0) {
		tmpIndex++;
	}

	if( tmpIndex < lexBufferMaxSize ) {
		if( '#' == lexBuffer[tmpIndex] ) {
			// This is a comment.
			readNextLine();
		}
	}
}

void SSTEmberTokenizer::consumeBufferWhitespace( size_t* index ) {
	while( ((*index) != lexBufferMaxSize) &&
		std::isspace( static_cast<int>( lexBuffer[(*index)] ) ) != 0 ) {
		(*index)++;
	}
}

SSTEmberToken* SSTEmberTokenizer::getNextToken() {
	if(reachedEOF) {
		return NULL;
	} else if( currentBufferIndex == lexBufferMaxSize ||
		lexBuffer[currentBufferIndex] == '\0' ) {

		printf("Reached the end of the line, so reading the next one...\n");
		readNextLine();
		return getNextToken();
	} else {
		char* tokenBuffer = (char*) malloc( sizeof(char) * (lexBufferMaxSize + 1) );
		size_t tokenBufferIndex = 0;

		// Eat any white space before the next token
		consumeBufferWhitespace(&currentBufferIndex);

		while(currentBufferIndex == lexBufferMaxSize) {
			readNextLine();
			consumeBufferWhitespace(&currentBufferIndex);
		}

		printf("Processing the line...\n");

		if( feof(file) ) {
			printf("File EOF reached!\n");
			return NULL;
		} else {
			printf("cbi=%lu max=%lu\n", currentBufferIndex, lexBufferMaxSize);
			bool handleString = false;

			while( (currentBufferIndex < lexBufferMaxSize) &&
				lexBuffer[currentBufferIndex] != '\0') {

				char nextBuffChar = lexBuffer[currentBufferIndex];

				if( '\"' == nextBuffChar ) {
					handleString = !handleString;
					currentBufferIndex++;
				} else if( '\0' == nextBuffChar ) {
					if(handleString) {
						fprintf(stderr, "Error: end of line without terminating string literal.\n");
					}
				} else {
					if(handleString) {
						tokenBuffer[tokenBufferIndex] = nextBuffChar;
						tokenBufferIndex++;
						currentBufferIndex++;
					} else {
						if( (0 != std::isspace( static_cast<int>( nextBuffChar ) ) ) ) {
							break;
						} else {
							currentBufferIndex++;

							if( nextBuffChar != '_' &&
								(0 != std::ispunct( static_cast<int>( nextBuffChar ) ) ) ) {

								if(0 == tokenBufferIndex) {
									tokenBuffer[tokenBufferIndex] = nextBuffChar;
									tokenBufferIndex++;
								} else {
									// Rewind
									currentBufferIndex--;
								}

								break;
							} else {
								tokenBuffer[tokenBufferIndex] = nextBuffChar;
								tokenBufferIndex++;
							}
						}
					}
				}
			}

			tokenBuffer[tokenBufferIndex] = '\0';

			SSTEmberToken* token = new SSTEmberToken(tokenBuffer);
			free(tokenBuffer);

			return token;
		}
	}
}

void SSTEmberTokenizer::consumeToLineEnd() {
	int tmp = fgetc(file);

	// Just eat the data until we get to the end of the line
	while( (tmp != EOF) && (tmp != '\n') &&
		(tmp != '\r')) {

		tmp = fgetc(file);
	}
}

void SSTEmberTokenizer::resizeBuffer() {
	char* newBuffer = (char*) malloc( sizeof(char) * (lexBufferMaxSize + 128) );

	for( size_t i = 0; i < lexBufferMaxSize; ++i ) {
		newBuffer[i] = lexBuffer[i];
	}

	free(lexBuffer);

	lexBuffer = newBuffer;
	lexBufferMaxSize = lexBufferMaxSize + 128;
}
