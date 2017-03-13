


test: test.cc ssttokenizer.cc ssttokenizer.h ssttoken.h
	$(CXX) -g -c ssttokenizer.cc -I./
	$(CXX) -g -c test.cc -I./
	$(CXX) -g -o test test.o ssttokenizer.o


clean:
	rm test *.o
