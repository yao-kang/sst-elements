// Copyright 2020 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2020 NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _FAULTS_H
#define _FAULTS_H

#include <sst/core/event.h>
#include <sst/core/sst_types.h>

#include "reg.h"
#include "sst/core/rng/sstrng.h"

namespace SST {
namespace MIPS4KCComponent {

    // Object to check if we should inject a fault
    class faultChecker_t {
        typedef enum {
            NO_LOC_FAULT_IDX,
            RF_FAULT_IDX,
            ID_FAULT_IDX,
            MDU_FAULT_IDX,
            MEM_PRE_FAULT_IDX,
            MEM_POST_FAULT_IDX,
            WB_FAULT_IDX,
            LAST_FAULT_IDX
        } location_idx_t; 


        RNG::SSTRandom*  rng;
        faultTrack::location_t locations;
        int64_t faultTime[LAST_FAULT_IDX]; // when a fault should be
                                            // injected
        int64_t MDU_count;
        int64_t MEM_count;
        bool checkForFault(faultTrack::location_t); // should we inject?
        unsigned int getRand1_31(); // generate # from 1 to 31
    public:
        faultChecker_t() {rng=0;}
        void init(faultTrack::location_t loc, uint64_t period, uint32_t seed);

        // convenience functions
        faultDesc getFault(faultTrack::location_t);
        void checkAndInject_RF(reg_word R[32]);
        void checkAndInject_MDU(reg_word &hi, reg_word &lo);
        void checkAndInject_MEM_PRE(reg_word &addr, reg_word &value, bool isLoad);
        void checkAndInject_MEM_POST(reg_word &data);
        void checkAndInject_WB(reg_word &data);
    };
    
};
};


#endif //_FAULTS_H
