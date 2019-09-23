// Copyright 2019 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2019, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include <sst_config.h>
#include "mips_4kc.h"

#include <sst/core/params.h>
#include <sst/core/simulation.h>
#include <sst/core/rng/marsaglia.h>
#include <sst/elements/memHierarchy/memEvent.h>

using namespace SST;
using namespace SST::MIPS4KCComponent;

MIPS4KC::MIPS4KC(ComponentId_t id, Params& params) :
    Component(id), 
    break_inst(NULL), program_break(0), 
    breakpoint_reinsert(0), text_seg(0), text_modified(0), text_top(0),
    data_seg(0), data_modified(0), data_seg_h(0), data_seg_b(0), 
    data_top(0), DATA_BOT(0), cycle_level(0), cycle_running(0)
{
    uint32_t outputLevel = params.find<uint32_t>("verbose", 0);
    out.init("MIPS4KC:@p:@l: ", outputLevel, 0, Output::STDOUT);

    // tell the simulator not to end without us
    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

    // init memory
    memory = dynamic_cast<Interfaces::SimpleMem*>(loadSubComponent("memHierarchy.memInterface", this, params));
    if (!memory) {
        out.fatal(CALL_INFO, -1, "Unable to load memHierarchy.memInterface subcomponent\n");
    }
    memory->initialize("mem_link", new Interfaces::SimpleMem::Handler<MIPS4KC> (this, &MIPS4KC::handleEvent));

    //set our clock
    std::string clockFreq = params.find<std::string>("clock", "1GHz");
    clockHandler = new Clock::Handler<MIPS4KC>(this, &MIPS4KC::clockTic);
    clockTC = registerClock(clockFreq, clockHandler);

    // SPIM-CL config
    pipe_out =  stdout;
    console_out =  stdout;
    message_out =  stdout;   
    console_in = 0;
    mapped_io = 0;
    cycle_level = 1;
    bare_machine = 1;
    quiet = 1;
    /*tlb_on = 0;
    icache_on = 0;
    dcache_on = 0;*/

    program_starting_address = 0;
    initial_text_size = TEXT_SIZE;
    initial_data_size = DATA_SIZE;
    initial_data_limit = DATA_LIMIT;
    initial_stack_size = STACK_SIZE;
    initial_stack_limit = STACK_LIMIT;
    initial_k_text_size = K_TEXT_SIZE;
    initial_k_data_size = K_DATA_SIZE;
    initial_k_data_limit = K_DATA_LIMIT;

    text_seg = 0;
    data_seg = 0;
    data_seg_h = 0;
    data_seg_b = 0;
    stack_seg = 0;
    stack_seg_b = 0;
    k_text_seg = 0;
    k_data_seg_h = 0;
    k_data_seg_b = 0;
    k_data_top = 0;

    initialize_world(0);
    cl_initialize_world(1);
    read_aout_file("foo");
    PC = program_starting_address;
}

MIPS4KC::MIPS4KC() : Component(-1)
{
	// for serialization only
}


void MIPS4KC::init(unsigned int phase) {  
    // init memory
    memory->init(phase);
    
    // Everything below we only do once
    if (phase != 0) {
        return;
    }


}

// handle incoming memory
void MIPS4KC::handleEvent(Interfaces::SimpleMem::Request * req)
{
    std::map<uint64_t, void*>::iterator i = requests.find(req->id);
    if (i == requests.end()) {
	out.fatal(CALL_INFO, -1, "Request ID (%" PRIx64 ") not found in outstanding requests!\n", req->id);
    } else {
        // handle event
        // ...
        // clean up
        requests.erase(i);
    }
}

bool MIPS4KC::clockTic( Cycle_t c)
{
    bool isFalling = (c & 0x1);
    Cycle_t pipeCycle = c >> 1;
    printf("CYCLE %llu: %llu.%u\n", c, pipeCycle, isFalling);

    if (isFalling) {
        cl_run_falling (PC, 1);
    } else {
        cl_run_rising(); // issue memory requests
    }

    // return false so we keep going
    return false;
}


