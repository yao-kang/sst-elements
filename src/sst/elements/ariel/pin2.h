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


#ifndef _H_ARIEL_APP_INTERFACE_PIN2
#define _H_ARIEL_APP_INTERFACE_PIN2

#include <sst/core/subcomponent.h>
#include <sst/core/output.h>

#include <stdint.h>
#include <vector>

#include "arielappinterface.h"
#include "ariel_shmem.h"

#define STRINGIZE(input) #input

using namespace SST;

namespace SST {

namespace ArielComponent {

class ArielPin2Interface : public ArielAppInterface {

    public:
        SST_ELI_REGISTER_SUBCOMPONENT_DERIVED(ArielPin2Interface, "ariel", "interface.pin2", SST_ELI_ELEMENT_VERSION(1,0,0),
                "Pin 2.14 launcher and interface", SST::ArielComponent::ArielAppInterface)
        
        SST_ELI_DOCUMENT_PARAMS(
            {"verbose", "Verbosity for debugging. Increased numbers for increased verbosity.", "0"},
            {"profilefunctions", "Profile functions for Ariel execution, 0 = none, >0 = enable", "0" },
            {"maxcorequeue", "Maximum queue depth per core", "64"},
            {"arieltool", "Path to the Ariel PIN-tool shared library", ""},
            {"launcher", "Specify the launcher to be used for instrumentation, default is path to PIN", STRINGIZE(PINTOOL_EXECUTABLE)},
            {"executable", "Executable to trace", ""},
            {"launchparamcount", "Number of parameters supplied for the launch tool", "0" },
            {"launchparam%(launchparamcount)", "Set the parameter to the launcher", "" },
            {"envparamcount", "Number of environment parameters to supply to the Ariel executable, default=-1 (use SST environment)", "-1"},
            {"envparamname%(envparamcount)", "Sets the environment parameter name", ""},
            {"envparamval%(envparamcount)", "Sets the environment parameter value", ""},
            {"appargcount", "Number of arguments to the traced executable", "0"},
            {"apparg%(appargcount)d", "Arguments for the traced executable", ""},
            {"arielmode", "Tool interception mode, set to 1 to trace entire program (default), set to 0 to delay tracing until ariel_enable() call., set to 2 to attempt auto-detect", "2"},
            {"arielinterceptcalls", "Toggle intercepting library calls", "0"},
            {"arielstack", "Dump stack on malloc calls (also requires enabling arielinterceptcalls). May increase overhead due to keeping a shadow stack.", "0"},
            {"mallocmapfile", "File with valid 'ariel_malloc_flag' ids", ""},
            {"writepayloadtrace", "Trace write payloads and put real memory contents into the memory system", "0"})

        /* Constructor takes a core count */
        ArielPin2Interface(ComponentId_t id, Params& params, uint32_t cores, ArielMemoryManager* memmgr);

        ArielPin2Interface(Component* comp, Params& params) : ArielAppInterface(comp, params) { }

        virtual ~ArielPin2Interface();

        /* 
         * Launch the application
         * Return whether launch was successful
         */
        virtual bool launch();

        /* 
         * Get the next available command for a core
         * Non-blocking
         * Return whether command is valid
         */
        virtual bool getCommandNB(uint32_t coreID, ArielCommand* ac);

        /*
         * Get the next available command for a core
         * Blocking
         */
        virtual ArielCommand getCommand(uint32_t coreID);

        /*
         * Clock function 
         */
        virtual void tick();

        virtual void emergencyShutdown();

#ifdef HAVE_CUDA
        GpuReturnTunnel* getGpuReturnTunnel();
        GpuDataTunnel* getGpuDataTunnel();

    private:
        GpuReturnTunnel* tunnelR;
        GpuDataTunnel* tunnelD;
#endif

    private:
        int forkPINChild(const char* app, char** args, std::map<std::string,std::string>& app_env);

        ArielTunnel* tunnel;

        // Launch
        pid_t child_pid;
        std::string appLauncher;
        char **execute_args;
        std::map<std::string,std::string> execute_env;
};

}
}

#endif
