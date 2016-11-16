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

#include <sstream>
#include <sst_config.h>
#include <sst/core/module.h>
#include <sst/core/subcomponent.h>
#include <sst/core/link.h>

#include "sst/elements/miranda/mirandaEvent.h"
#include "sst/elements/miranda/generators/copygen.h"

#include "multiThread.h"

using namespace SST;
using namespace SST::Miranda;
using namespace SST::Thornhill;

MultiThread::MultiThread( Component* owner, 
        Params& params )
        : DetailedCompute( owner )
{
    std::string prefix = params.find<std::string>( "portName", "detailed" );

    std::stringstream numSS;
   
    int num = 0; 
    numSS << num;
    std::string portName = prefix + numSS.str();
    while ( owner->isPortConnected( portName.c_str() ) ) {
        m_links.push_back( configureLink( portName.c_str(), "0ps", 
                new Event::Handler<MultiThread>( this,&MultiThread::eventHandler ) ) ); 
        assert(m_links.back());
        
        numSS.str(std::string());
        numSS << ++ num;
        portName = prefix + numSS.str();
    }
    m_numThreads =  params.find<unsigned>( "numThreads", m_links.size() );
    assert( m_numThreads <= m_links.size() ); 
}

void MultiThread::eventHandler( SST::Event* ev )
{
    MirandaRspEvent* event = static_cast<MirandaRspEvent*>(ev);

	Entry* entry = static_cast<Entry*>((void*)event->key);
    --entry->numThreads;
    if ( 0 == entry->numThreads ) {
	    entry->finiHandler();
	    delete entry;
    }
    delete ev;
}

static std::pair<std::string, SST::Params> splitGenerator( 
            Component* comp, size_t numThreads, int thread, std::string name, SST::Params params )
{
    std::pair<std::string, SST::Params> retval;
    Params notused;

    GeneratorSplit* ptr = dynamic_cast<GeneratorSplit*>( comp->loadModule( name + "Split", notused ) );

    retval = ptr->split( numThreads, thread, name, params );

    delete ptr;
    return retval; 
} 

static MirandaReqEvent* splitGenerators( 
        Component* comp, size_t numThreads, int thread, const std::deque< std::pair< std::string, SST::Params> >& generators )
{
    MirandaReqEvent* event = new MirandaReqEvent;

    std::deque< std::pair< std::string, SST::Params> >::const_iterator iter = generators.begin();

    for ( ; iter != generators.end(); ++iter ) {
        event->generators.push_back( splitGenerator( comp, numThreads, thread, iter->first, iter->second) );
    }  
    return event;
} 

void MultiThread::start( const std::deque< std::pair< std::string, SST::Params> >& generators,
                 std::function<int()> finiHandler )
{
    Entry* entry = new Entry( m_numThreads, finiHandler );

    for ( unsigned i = 0; i < m_numThreads; i++ ) {

        MirandaReqEvent* event = splitGenerators( parent, m_numThreads, i, generators );	
	    event->key = (uint64_t) entry;

	    m_links[i]->send( 0, event );
    }
}
