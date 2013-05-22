
#ifndef _H_ISIS_MPI_P2P_RECV_EVENT
#define _H_ISIS_MPI_P2P_RECV_EVENT

#include "isismpievent.h"
#include "isismpip2pev.h"

namespace SST {
namespace IsisComponent {
namespace IsisMPI {

	class IsisMPIPt2PtRecvEvent : IsisMPIPt2PtEvent {
		public:
			IsisMPIPt2PtRecvEvent(int count, IsisMPIDataType type, int source,
				int tag, IsisMPICommGroup group);
			int   getSource();
			
		protected:
			int 	source;
	}

}
}
}


#endif