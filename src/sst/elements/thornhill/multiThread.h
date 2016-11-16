// Copyright 2009-2016 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2016, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_THORNHILL_MULTI_THREAD
#define _H_THORNHILL_MULTI_THREAD

#include "detailedCompute.h"

namespace SST {
namespace Thornhill {

class MultiThread : public DetailedCompute {

	struct Entry {
      Entry( size_t numThreads, std::function<int()>& _finiHandler ) : 
          numThreads(numThreads), finiHandler( _finiHandler ) {}

    	std::function<int()> finiHandler; 
        size_t numThreads;
	};

  public:

    MultiThread( Component* owner, Params& params );

    ~MultiThread(){};

    virtual void start( const std::deque< 
						std::pair< std::string, SST::Params > >&, 
                 std::function<int()> );
    virtual bool isConnected() { return ( !m_links.empty() ); }
	
	virtual std::string getModelName() {
		return "thornhill.MultiThread";
	}

  private:
    void eventHandler( SST::Event* ev ); 
    std::vector<Link*>  m_links;
	Entry* m_entry;
    unsigned    m_numThreads;
};


}
}
#endif
