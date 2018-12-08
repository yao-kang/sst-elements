// Copyright 2009-2018 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2018, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _H_SST_ARIEL_READ_EVENT
#define _H_SST_ARIEL_READ_EVENT

#include "arielevent.h"

using namespace SST;

namespace SST {
namespace ArielComponent {

class ArielReadEvent : public ArielEvent {

    public:
        ArielReadEvent(uint64_t rAddr, uint32_t length) :
            readAddress(rAddr), readLength(length), sOffset(0), fold(0) {
        }

       ArielReadEvent(uint64_t rAddr, uint32_t length, uint64_t off, bool doFold) :
           readAddress(rAddr), readLength(length), sOffset(off), fold(doFold) {
        }

        ~ArielReadEvent() {
        }

        ArielEventType getEventType() const {
                return READ_ADDRESS;
        }

        uint64_t getAddress() const {
                return readAddress;
        }

        uint32_t getLength() const {
                return readLength;
        }

        uint32_t getScratchOffset() const {
                return sOffset;
        }

        uint32_t getFold() const {
                return fold;
        }

    private:
        const uint64_t readAddress;
        const uint32_t readLength;
        const uint64_t sOffset;
        const bool fold;
};

}
}

#endif
