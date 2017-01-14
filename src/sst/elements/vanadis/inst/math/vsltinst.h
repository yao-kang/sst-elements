/*
// Copyright 2009-2016 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2016, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.
 */

/* 
 * File:   vsltinst.h
 * Author: sdhammo
 *
 * Created on January 13, 2017, 4:13 PM
 */

#ifndef VSLTINST_H
#define VSLTINST_H

#include "sst/elements/vanadis/inst/vmathinst.h"

namespace SST {
    namespace Vanadis {
        class VanadisSLTInstruction : public VanadisMathInstruction {
            
        public:
            VanadisSLTInstruction(SST::Output* output, const uint32_t id, const uint64_t iAddr,
			const uint32_t rRd, const uint32_t rR1, const uint32_t rR2) :
                VanadisMathInstruction(output, id, iAddr, rRd, rR1, rR2) {}
            
                void execute() {
                    output->verbose(CALL_INFO, 8, 0, "Executing: %s...\n", toString().c_str());
                    //intRF[rRd] = rR1 >> rR2;
                }

                std::string toString() {
                    std::string instStr = "SLT " + generateRegisterString();
                    return instStr;
                }
                
                std::string toMnemonic() {
                    return "SLT";
                }
        };
    }
}


#endif /* VSLTINST_H */

