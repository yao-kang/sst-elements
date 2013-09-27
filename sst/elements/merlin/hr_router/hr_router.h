// Copyright 2009-2013 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2013, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef COMPONENTS_HR_ROUTER_HR_ROUTER_H
#define COMPONENTS_HR_ROUTER_HR_ROUTER_H

#include <sst/core/clock.h>
#include <sst/core/component.h>
#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>

#include <queue>

#include "sst/elements/merlin/router.h"

using namespace SST;

namespace SST {
namespace Merlin {

class PortControl;

class hr_router : public Router {

private:
    static int num_routers;
    static int print_debug;
    int id;
    int num_ports;
    int num_vcs;

    Topology* topo;
    XbarArbitration* arb;
    
    PortControl** ports;

    int* in_port_busy;
    int* out_port_busy;
    int* progress_vcs;

    Cycle_t unclocked_cycle;
    std::string xbar_bw;
    TimeConverter* xbar_tc;
    Clock::Handler<hr_router>* my_clock_handler;

    bool clock_handler(Cycle_t cycle);
    bool debug_clock_handler(Cycle_t cycle);
    static void sigHandler(int signal);

public:
    hr_router(ComponentId_t cid, Params& params);
    ~hr_router();
    
    void init(unsigned int phase);
    void setup();
    void finish() {}

    void notifyEvent();
    void dumpState(std::ostream& stream);

};

}
}

#endif // COMPONENTS_HR_ROUTER_HR_ROUTER_H
