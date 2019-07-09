// Copyright 2009-2019 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2019, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _H_ARIEL_APP_INTERFACE
#define _H_ARIEL_APP_INTERFACE

#include <sst/core/subcomponent.h>
#include <sst/core/output.h>

#include <stdint.h>
#include <vector>

#include "ariel_shmem.h"
#include "arielmemmgr.h"

using namespace SST;

namespace SST {

namespace ArielComponent {

/* 
 * This class generically represents an incoming instruction stream from an application
 * The CPU calls launch() to tell the application stream to start
 * The CPU/cores call getCommand()/getCommandNB() to get the next instruction in the stream
 */

class ArielAppInterface : public SubComponent {

    public:
        /* Constructor takes a core count */
        SST_ELI_REGISTER_SUBCOMPONENT_API(SST::ArielComponent::ArielAppInterface, uint32_t, ArielMemoryManager*)

        ArielAppInterface(ComponentId_t id, Params& params, uint32_t cores, ArielMemoryManager* memmgr) : SubComponent(id) {
            verbosity = params.find<int>("verbose", 0);
            output = new SST::Output("ArielMemoryManager[@f:@l:@p] ",
                verbosity, 0, SST::Output::STDOUT);
        }

        ArielAppInterface(Component* comp, Params& params) : SubComponent(comp) {
            output = new SST::Output("", 0, 0, SST::Output::STDOUT);
            output->fatal(CALL_INFO, -1, "Error: ArielMemoryManager subcomponents do not support loading using legacy load functions");
        }

        virtual ~ArielAppInterface() {
            delete output;
        }

        /* 
         * Launch the application
         * Return whether launch was successful
         */
        virtual bool launch() = 0;

        /* 
         * Get the next available command for a core
         * Nonblocking
         * Return whether command is valid
         */
        virtual bool getCommandNB(uint32_t coreID, ArielCommand* ac) = 0;

        /*
         * Get the next available command for a core
         * Blocking
         */
        virtual ArielCommand getCommand(uint32_t coreID) = 0;

        /*
         * Clock function, called by ariel cpu
         */
        virtual void tick() = 0;

        virtual void emergencyShutdown() = 0; // TODO is this inherited from basecomponent?

    protected:
        Output* output;
        int verbosity;
};

}
}

#endif
