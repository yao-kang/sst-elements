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

#include <sst_config.h>
#include <sst/core/rng/mersenne.h>
#include "mips_4kc.h"
#include "faults.h"

using namespace SST;
using namespace SST::RNG;
using namespace SST::MIPS4KCComponent;
using namespace faultTrack;


void faultChecker_t::init(faultTrack::location_t loc, uint64_t period, 
                          uint32_t seed) {
    locations = loc;
    printf("Fault Injector: Inject faults at 0x%x\n", locations);

    //init RNG
    if (seed != 0) {
        rng = new MersenneRNG(seed);
    } else {
        rng = new MersenneRNG();
    }

    for (int i = NO_LOC_FAULT_IDX; i < LAST_FAULT_IDX; ++i) {
        event_count[i] = 0;
    }

    // generate when different faults should occur
#define GEN_F_TIME(FAULT) \
    if (locations & FAULT) {                                            \
        faultTime[FAULT##_IDX] = rng->generateNextUInt64() % period;     \
        printf(" Will Inject %s at %lld\n", #FAULT ,faultTime[FAULT##_IDX]); \
    } else {                                                            \
        faultTime[FAULT##_IDX] = -1;                                     \
    }

    GEN_F_TIME(RF_FAULT);
    GEN_F_TIME(ID_FAULT);
    GEN_F_TIME(MDU_FAULT);
    GEN_F_TIME(MEM_PRE_FAULT);
    GEN_F_TIME(MEM_POST_FAULT);
    GEN_F_TIME(WB_FAULT);
    GEN_F_TIME(ALU_FAULT);

#undef GEN_F_TIME

}

// should we inject?
bool faultChecker_t::checkForFault(faultTrack::location_t loc) {
    switch (loc) {
    case RF_FAULT:
        event_count[RF_FAULT_IDX]++;
        if (reg_word::getNow() == faultTime[RF_FAULT_IDX]) {return true;}
        break;
    case ID_FAULT:
        // ???
        event_count[ID_FAULT_IDX]++;
        break;
    case MDU_FAULT:
        event_count[MDU_FAULT_IDX]++;
        if(event_count[MDU_FAULT_IDX] == faultTime[MDU_FAULT_IDX]) {
            return true;
        }
        break;
    case MEM_PRE_FAULT:
        // this assumes that MEM_PRE must be checked everytime since
        // we don't increment in MEM_POST
        event_count[MEM_PRE_FAULT_IDX]++;
        if(event_count[MEM_PRE_FAULT_IDX] == faultTime[MEM_PRE_FAULT_IDX]) {return true;}
        break;
    case MEM_POST_FAULT:
        event_count[MEM_POST_FAULT_IDX]++;
        if(event_count[MEM_PRE_FAULT_IDX] == faultTime[MEM_POST_FAULT_IDX]) {return true;}
        break;
    case WB_FAULT:
        event_count[WB_FAULT_IDX]++;
        if (reg_word::getNow() == faultTime[WB_FAULT_IDX]) {return true;}
        break;
    case ALU_FAULT:
        event_count[ALU_FAULT_IDX]++;
        if (reg_word::getNow() == faultTime[ALU_FAULT_IDX]) {return true;}
        break;
    default:
        printf("Unknown fault location\n");
    }

    // default
    return false;
}

// handy for picking which register in register file
unsigned int faultChecker_t::getRand1_31() {
    unsigned int ret;
    do {
        ret = rng->generateNextUInt32() & 0x1f;
    } while (ret == 0);
    return ret;
}

faultDesc faultChecker_t::getFault(faultTrack::location_t loc) {
    int bit = rng->generateNextUInt32() & 0x1f;
    return faultDesc(loc, bit);
}

void faultChecker_t::checkAndInject_RF(reg_word R[32]) {
    if(checkForFault(faultTrack::RF_FAULT)) {
        unsigned int reg = getRand1_31();
        printf("INJECTING RF FAULT reg %d @ %lld\n", reg, reg_word::getNow());
        R[reg].fault(getFault(RF_FAULT));
    }
}


void faultChecker_t::checkAndInject_MDU(reg_word &hi, reg_word &lo) {
    if (checkForFault(MDU_FAULT)) {
        uint32_t roll = rng->generateNextUInt32();
        bool faultHi = roll & 0x1;
        roll >>= 1;

        int bit = roll & 0x1f;

        printf("INJECTING MDU FAULT reg %s:%d @ %lld\n", 
               (faultHi) ? "hi" : "lo", bit, reg_word::getNow());
        if (faultHi) {
            hi.fault(faultDesc(MDU_FAULT, bit));
        } else {
            lo.fault(faultDesc(MDU_FAULT, bit));
        }
    }
}

// possibly fault data and address
// isLoad ignored for now
void faultChecker_t::checkAndInject_MEM_PRE(reg_word &addr, 
                                            reg_word &value, bool isLoad) {
    if (checkForFault(MEM_PRE_FAULT)) {
        uint32_t roll = rng->generateNextUInt32();
        bool faultAddr = roll & 0x1;
        roll >>= 1;

        int bit = roll & 0x1f;

        printf("INJECTING MEM_PRE FAULT %s:%d %s @ %lld\n", 
               (faultAddr) ? "Address" : "Data", bit,
               (isLoad) ? "(isLoad)" : "(isStore)",
               reg_word::getNow());
        if (isLoad && !faultAddr) {
            printf(" INJECTING fault to data on load: no effect\n");
        }
        if (faultAddr) {
            addr.fault(faultDesc(MEM_PRE_FAULT, bit));
        } else {
            value.fault(faultDesc(MEM_PRE_FAULT, bit));
        }
    }
}

// inject fault after MEM. Note: if inject into 'store' may have no
// value
void faultChecker_t::checkAndInject_MEM_POST(reg_word &data) {
    if(checkForFault(MEM_POST_FAULT)) {
        printf("INJECTING MEM_POST Fault @ %lld\n", reg_word::getNow());
        data.fault(getFault(MEM_POST_FAULT));
    }
}

void faultChecker_t::checkAndInject_WB(reg_word &data) {
    if(checkForFault(WB_FAULT)) {
        printf("INJECTING WB Fault  @ %lld\n", reg_word::getNow());
        data.fault(getFault(WB_FAULT));
    }
}

void faultChecker_t::checkAndInject_ALU(reg_word &data) {
    if(checkForFault(ALU_FAULT)) { 
        printf("INJECTING ALU Fault  @ %lld\n", reg_word::getNow());
        data.fault(getFault(ALU_FAULT));
    } 
}


void faultChecker_t::printStats() {
    printf("Faultable Events:\n");

#define PF(STR) printf("\t%s_event : %llu\n", #STR, event_count[STR##_IDX]);
    
    PF(RF_FAULT);
    PF(ID_FAULT);
    PF(MDU_FAULT);
    PF(MEM_PRE_FAULT);
    PF(MEM_POST_FAULT);
    PF(WB_FAULT);
    PF(ALU_FAULT);

#undef PF

}
