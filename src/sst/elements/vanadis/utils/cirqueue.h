
#ifndef _H_SST_VANADIS_CIRC_Q
#define _H_SST_VANADIS_CIRC_Q

namespace SST {
namespace Vanadis {

template<typename T>
class CircularQueue {

public:
		CircularQueue(int maxEntries) {

		}

		bool empty() {

		}

		bool full() {

		}

		T pop() {

		}

		void push(T newEntry) {

		}

protected:
		T* entries;
		int currentHead;
		int currentTail;

};

}
}


#endif
