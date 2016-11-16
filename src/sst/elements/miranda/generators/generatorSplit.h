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


#ifndef _H_SST_MIRANDA_GEN_SPLIT_GEN
#define _H_SST_MIRANDA_GEN_SPLIT_GEN

#include <sst/core/module.h>


namespace SST {
namespace Miranda {

class GeneratorSplit : public Module {
public:
    virtual std::pair<std::string, SST::Params> split( size_t numThreads, int thread, std::string name, SST::Params params ) = 0; 

protected:
    void copyParam( SST::Params& newParams, SST::Params& oldParams,std::string key )
    {
        bool found;
        std::string value =  oldParams.find<std::string>( key , found );
        if ( found ) {
            newParams.insert( key, value );
       }
    }

    template < typename T>
    void insertParam( SST::Params& newParams, SST::Params& oldParams,std::string key, T value )
    {
        std::stringstream tmp;

        tmp << value;
        newParams.insert( key, tmp.str() );
    }
};

}
}

#endif
