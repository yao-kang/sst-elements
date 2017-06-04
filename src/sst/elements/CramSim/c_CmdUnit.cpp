// Copyright 2009-2017 Sandia Corporation. Under the terms
// of Contract DE-NA0003525 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2017, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

/*
 * c_CmdUnit.cpp
 *
 *  Created on: June 22, 2016
 *      Author: mcohen
 */

// Copyright 2015 IBM Corporation
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// C++ includes
//SST includes
#include "sst_config.h"

#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <assert.h>

// CramSim includes
#include "c_CmdUnit.hpp"
#include "c_DeviceController.hpp"
#include "c_Transaction.hpp"
#include "c_AddressHasher.hpp"
#include "c_HashedAddress.hpp"
#include "c_TokenChgEvent.hpp"
#include "c_CmdPtrPkgEvent.hpp"
#include "c_CmdReqEvent.hpp"
#include "c_CmdResEvent.hpp"
#include "c_BankState.hpp"
#include "c_BankStateIdle.hpp"
#include "c_BankCommand.hpp"


using namespace SST;
using namespace SST::n_Bank;

c_CmdUnit::c_CmdUnit(Component *owner, Params& x_params) : c_CtrlSubComponent<c_BankCommand*,c_BankCommand*>(owner, x_params){

	m_Owner=dynamic_cast<c_Controller*>(owner);

	m_thisCycleResReceived = 0;
	m_refsSent = 0;
	m_inflightWrites.clear();

	// read params here
	bool l_found = false;

	k_cmdReqQEntries = (uint32_t)x_params.find<uint32_t>("numCmdReqQEntries", 100, l_found);
	if (!l_found) {
		std::cout << "numCmdUnitReqQEntries value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_cmdResQEntries = (uint32_t)x_params.find<uint32_t>("numCmdResQEntries", 100, l_found);
	if (!l_found) {
		std::cout << "numCmdUnitResQEntries value is missing... exiting"
				<< std::endl;
		exit(-1);
	}
	m_cmdResQTokens = k_cmdResQEntries;

	k_numBytesPerTransaction = (uint32_t)x_params.find<uint32_t>("numBytesPerTransaction",
			32, l_found);
	if (!l_found) {
		std::cout << "numBytesPerTransaction value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_numChannelsPerDimm = (uint32_t)x_params.find<uint32_t>("numChannelsPerDimm", 1,
			l_found);
	if (!l_found) {
		std::cout << "numChannelsPerDimm value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_numPseudoChannels = (uint32_t)x_params.find<uint32_t>("numPseudoChannels", 1,
															 l_found);
	if (!l_found) {
		std::cout << "numPseudoChannel value is missing... "
				  << std::endl;
		//exit(-1);
	}

	k_numRanksPerChannel = (uint32_t)x_params.find<uint32_t>("numRanksPerChannel", 2,
			l_found);
	if (!l_found) {
		std::cout << "numRanksPerChannel value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_numBankGroupsPerRank = (uint32_t)x_params.find<uint32_t>("numBankGroupsPerRank", 100,
			l_found);
	if (!l_found) {
		std::cout << "numBankGroupsPerRank value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_numBanksPerBankGroup = (uint32_t)x_params.find<uint32_t>("numBanksPerBankGroup", 100,
			l_found);
	if (!l_found) {
		std::cout << "numBanksPerBankGroup value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_numColsPerBank = (uint32_t)x_params.find<uint32_t>("numColsPerBank", 100, l_found);
	if (!l_found) {
		std::cout << "numColsPerBank value is missing... exiting" << std::endl;
		exit(-1);
	}

	k_numRowsPerBank = (uint32_t)x_params.find<uint32_t>("numRowsPerBank", 100, l_found);
	if (!l_found) {
		std::cout << "numRowsPerBank value is missing... exiting" << std::endl;
		exit(-1);
	}

	k_relCommandWidth = (uint32_t)x_params.find<uint32_t>("relCommandWidth", 100, l_found);
	if (!l_found) {
		std::cout << "relCommandWidth value is missing... exiting" << std::endl;
		exit(-1);
	}

	k_useRefresh = (uint32_t)x_params.find<uint32_t>("boolUseRefresh", 1, l_found);
	if (!l_found) {
		std::cout << "boolUseRefresh param value is missing... exiting"
				<< std::endl;
		exit(-1);
	}


	/* BUFFER ALLOCATION PARAMETERS */
	k_allocateCmdResQACT = (uint32_t)x_params.find<uint32_t>("boolAllocateCmdResACT", 1,
			l_found);
	if (!l_found) {
		std::cout << "boolAllocateCmdResACT value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_allocateCmdResQREAD = (uint32_t)x_params.find<uint32_t>("boolAllocateCmdResREAD", 1,
			l_found);
	if (!l_found) {
		std::cout << "boolAllocateCmdResREAD value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_allocateCmdResQREADA = (uint32_t)x_params.find<uint32_t>("boolAllocateCmdResREADA", 1,
			l_found);
	if (!l_found) {
		std::cout << "boolAllocateCmdResREADA value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_allocateCmdResQWRITE = (uint32_t)x_params.find<uint32_t>("boolAllocateCmdResWRITE", 1,
			l_found);
	if (!l_found) {
		std::cout << "boolAllocateCmdResWRITE value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_allocateCmdResQWRITEA = (uint32_t)x_params.find<uint32_t>("boolAllocateCmdResWRITEA",
			1, l_found);
	if (!l_found) {
		std::cout << "boolAllocateCmdResWRITEA value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_allocateCmdResQPRE = (uint32_t)x_params.find<uint32_t>("boolAllocateCmdResPRE", 1,
			l_found);
	if (!l_found) {
		std::cout << "boolAllocateCmdResPRE value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_cmdQueueFindAnyIssuable = (uint32_t)x_params.find<uint32_t>(
			"boolCmdQueueFindAnyIssuable", 1, l_found);
	if (!l_found) {
		std::cout << "boolCmdQueueFindAnyIssuable value is missing... exiting"
				<< std::endl;
		exit(-1);
	}

	k_bankPolicy = (uint32_t)x_params.find<uint32_t>("bankPolicy", 0, l_found);
	if (!l_found) {
		std::cout << "bankPolicy value is missing... exiting" << std::endl;
		exit(-1);
	}

	k_useDualCommandBus = (uint32_t)x_params.find<uint32_t>("boolDualCommandBus", 0, l_found);
	if (!l_found) {
		std::cout << "boolDualCommandBus value is missing... disabled" << std::endl;
	}

	k_multiCycleACT = (uint32_t)x_params.find<uint32_t>("boolMultiCycleACT", 0, l_found);
	if (!l_found) {
		std::cout << "boolMultiCycleACT value is missing... disabled" << std::endl;
	}

	/* BANK TRANSITION PARAMETERS */
	//FIXME: Move this param reading to inside of c_BankInfo
	m_bankParams["nRC"] = (uint32_t)x_params.find<uint32_t>("nRC", 55, l_found);
	if (!l_found) {
		std::cout << "nRC value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRRD"] = (uint32_t)x_params.find<uint32_t>("nRRD", 4, l_found);
	if (!l_found) {
		std::cout << "nRRD value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRRD_L"] = (uint32_t)x_params.find<uint32_t>("nRRD_L", 6, l_found);
	if (!l_found) {
		std::cout << "nRRD_L value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRRD_S"] = (uint32_t)x_params.find<uint32_t>("nRRD_S", 4, l_found);
	if (!l_found) {
		std::cout << "nRRD_S value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRCD"] = (uint32_t)x_params.find<uint32_t>("nRCD", 16, l_found);
	if (!l_found) {
		std::cout << "nRRD_L value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nCCD"] = (uint32_t)x_params.find<uint32_t>("nCCD", 4, l_found);
	if (!l_found) {
		std::cout << "nCCD value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nCCD_L"] = (uint32_t)x_params.find<uint32_t>("nCCD_L", 5, l_found);
	if (!l_found) {
		std::cout << "nCCD_L value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nCCD_L_WR"] = (uint32_t)x_params.find<uint32_t>("nCCD_L_WR", 1, l_found);
	if (!l_found) {
		std::cout << "nCCD_L_WR value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nCCD_S"] = (uint32_t)x_params.find<uint32_t>("nCCD_S", 4, l_found);
	if (!l_found) {
		std::cout << "nCCD_S value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nAL"] = (uint32_t)x_params.find<uint32_t>("nAL", 15, l_found);
	if (!l_found) {
		std::cout << "nAL value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nCL"] = (uint32_t)x_params.find<uint32_t>("nCL", 16, l_found);
	if (!l_found) {
		std::cout << "nCL value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nCWL"] = (uint32_t)x_params.find<uint32_t>("nCWL", 16, l_found);
	if (!l_found) {
		std::cout << "nCWL value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nWR"] = (uint32_t)x_params.find<uint32_t>("nWR", 16, l_found);
	if (!l_found) {
		std::cout << "nWR value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nWTR"] = (uint32_t)x_params.find<uint32_t>("nWTR", 3, l_found);
	if (!l_found) {
		std::cout << "nWTR value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nWTR_L"] = (uint32_t)x_params.find<uint32_t>("nWTR_L", 9, l_found);
	if (!l_found) {
		std::cout << "nWTR_L value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nWTR_S"] = (uint32_t)x_params.find<uint32_t>("nWTR_S", 3, l_found);
	if (!l_found) {
		std::cout << "nWTR_S value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRTW"] = (uint32_t)x_params.find<uint32_t>("nRTW", 4, l_found);
	if (!l_found) {
		std::cout << "nRTW value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nEWTR"] = (uint32_t)x_params.find<uint32_t>("nEWTR", 6, l_found);
	if (!l_found) {
		std::cout << "nEWTR value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nERTW"] = (uint32_t)x_params.find<uint32_t>("nERTW", 6, l_found);
	if (!l_found) {
		std::cout << "nERTW value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nEWTW"] = (uint32_t)x_params.find<uint32_t>("nEWTW", 6, l_found);
	if (!l_found) {
		std::cout << "nEWTW value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nERTR"] = (uint32_t)x_params.find<uint32_t>("nERTR", 6, l_found);
	if (!l_found) {
		std::cout << "nERTR value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRAS"] = (uint32_t)x_params.find<uint32_t>("nRAS", 39, l_found);
	if (!l_found) {
		std::cout << "nRAS value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRTP"] = (uint32_t)x_params.find<uint32_t>("nRTP", 9, l_found);
	if (!l_found) {
		std::cout << "nRTP value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRP"] = (uint32_t)x_params.find<uint32_t>("nRP", 16, l_found);
	if (!l_found) {
		std::cout << "nRP value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nRFC"] = (uint32_t)x_params.find<uint32_t>("nRFC", 420, l_found);
	if (!l_found) {
		std::cout << "nRFC value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nREFI"] = (uint32_t)x_params.find<uint32_t>("nREFI", 9360, l_found);
	if (!l_found) {
		std::cout << "nREFI value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nFAW"] = (uint32_t)x_params.find<uint32_t>("nFAW", 16, l_found);
	if (!l_found) {
		std::cout << "nFAW value is missing ... exiting" << std::endl;
		exit(-1);
	}

	m_bankParams["nBL"] = (uint32_t)x_params.find<uint32_t>("nBL", 4, l_found);
	if (!l_found) {
		std::cout << "nBL value is missing ... exiting" << std::endl;
		exit(-1);
	}

	// configure the memory hierarchy
	uint32_t l_numChannels = k_numPseudoChannels * k_numChannelsPerDimm;
    m_numRanks = l_numChannels * k_numRanksPerChannel;
	m_numBankGroups = m_numRanks * k_numBankGroupsPerRank;
	m_numBanks = m_numBankGroups * k_numBanksPerBankGroup;

	for (int l_i = 0; l_i != m_numRanks; ++l_i) {
		c_Rank* l_entry = new c_Rank(&m_bankParams);
		m_ranks.push_back(l_entry);
	}

	for (int l_i = 0; l_i != m_numBankGroups; ++l_i) {
	        c_BankGroup* l_entry = new c_BankGroup(&m_bankParams, l_i);
		m_bankGroups.push_back(l_entry);
	}

	for (int l_i = 0; l_i != m_numBanks; ++l_i) {
		c_BankInfo* l_entry = new c_BankInfo(&m_bankParams, l_i); // auto init state to IDLE
		m_banks.push_back(l_entry);
	}

	// construct the addressHasher
	//c_AddressHasher::getInstance(x_params);

	// connect the hierarchy
	unsigned l_rankNum = 0;
	for (unsigned l_i = 0; l_i != l_numChannels; ++l_i) {
        c_Channel* l_channel = new c_Channel(&m_bankParams);
        m_channel.push_back(l_channel);
     //   int l_i = 0;
        for (unsigned l_j = 0; l_j != k_numRanksPerChannel; ++l_j) {
            std::cout << "Attaching Channel" << l_i << " to Rank" << l_rankNum
                    << std::endl;
            m_channel.at(l_i)->acceptRank(m_ranks.at(l_rankNum));
            m_ranks.at(l_rankNum)->acceptChannel(m_channel.at(l_i));
            ++l_rankNum;
		}
	}
	assert(l_rankNum == m_numRanks);

	unsigned l_bankGroupNum = 0;
	for (unsigned l_i = 0; l_i != m_numRanks; ++l_i) {
		for (unsigned l_j = 0; l_j != k_numBankGroupsPerRank; ++l_j) {
		  //std::cout << "Attaching Rank" << l_i <<
		  //	  " to BankGroup" << l_bankGroupNum << std::endl;
			m_ranks.at(l_i)->acceptBankGroup(m_bankGroups.at(l_bankGroupNum));
			m_bankGroups.at(l_bankGroupNum)->acceptRank(m_ranks.at(l_i));
			++l_bankGroupNum;
		}
	}
	assert(l_bankGroupNum == m_numBankGroups);

	unsigned l_bankNum = 0;
	for (int l_i = 0; l_i != m_numBankGroups; ++l_i) {
		for (int l_j = 0; l_j != k_numBanksPerBankGroup; ++l_j) {
		  //	std::cout << "Attaching BankGroup" << l_i <<
		  //	  " to Bank" << l_bankNum << std::endl;
			m_bankGroups.at(l_i)->acceptBank(m_banks.at(l_bankNum));
			m_banks.at(l_bankNum)->acceptBankGroup(m_bankGroups.at(l_i));
			l_bankNum++;
		}
	}
	assert(l_bankNum == m_numBanks);

	m_statsReqQ = new unsigned[k_cmdReqQEntries + 1];
	m_statsResQ = new unsigned[k_cmdResQEntries + 1];

	for (unsigned l_i = 0; l_i != k_cmdReqQEntries + 1; ++l_i)
		m_statsReqQ[l_i] = 0;

	for (unsigned l_i = 0; l_i != k_cmdResQEntries + 1; ++l_i)
		m_statsResQ[l_i] = 0;

	// tell the simulator not to end without us <--why?
	//registerAsPrimaryComponent();
	//primaryComponentDoNotEndSim();

	// configure links

	// CmdUnit <-> TxnUnit Links
	//// CmdUnit <- TxnUnit (Req) (Cmd)
/*	m_inTxnUnitReqPtrLink = configureLink("inTxnUnitReqPtr",
			new Event::Handler<c_CmdUnit>(this,
					&c_CmdUnit::handleInTxnUnitReqPtrEvent));
	//// CmdUnit -> TxnUnit (Req) (Token)
	m_outTxnUnitReqQTokenChgLink = configureLink("outTxnUnitReqQTokenChg",
			new Event::Handler<c_CmdUnit>(this,
					&c_CmdUnit::handleOutTxnUnitReqQTokenChgEvent));
	//// CmdUnit -> TxnUnit (Res) (Cmd)
	m_outTxnUnitResPtrLink = configureLink("outTxnUnitResPtr",
			new Event::Handler<c_CmdUnit>(this,
					&c_CmdUnit::handleOutTxnUnitResPtrEvent));

	// CmdUnit <-> Bank Links
	//// CmdUnit -> Bank (Req) (Cmd)
	m_outBankReqPtrLink = configureLink("outBankReqPtr",
			new Event::Handler<c_CmdUnit>(this,
					&c_CmdUnit::handleOutBankReqPtrEvent));
	//// CmdUnit <- Bank (Res) (Cmd)
	m_inBankResPtrLink = configureLink("inBankResPtr",
			new Event::Handler<c_CmdUnit>(this,
					&c_CmdUnit::handleInBankResPtrEvent));
*/
	// reset last data cmd issue cycle
	m_lastDataCmdIssueCycle = 0;
	m_lastDataCmdType = e_BankCommandType::READ;
	m_lastPseudoChannel = 0;

	// reset command bus
	m_blockColCmd.resize(k_numChannelsPerDimm);
	m_blockRowCmd.resize(k_numChannelsPerDimm);

	m_cmdACTFAWtracker.clear();
	m_cmdACTFAWtracker.resize(m_bankParams.at("nFAW"),0);

	//set our clock
//	registerClock("1GHz",
//			new Clock::Handler<c_CmdUnit>(this, &c_CmdUnit::clockTic));

	// set up cmd trace output
	k_printCmdTrace = (uint32_t)x_params.find<uint32_t>("boolPrintCmdTrace", 0, l_found);

	k_cmdTraceFileName = (std::string)x_params.find<std::string>("strCmdTraceFile", "-", l_found);
	k_cmdTraceFileName.pop_back(); // remove trailing newline (??)
	if(k_printCmdTrace) {
	  if(k_cmdTraceFileName.compare("-") == 0) {// set output to std::cout
	    std::cout << "Setting cmd trace output to std::cout" << std::endl;
	    m_cmdTraceStreamBuf = std::cout.rdbuf();
	  } else { // open the file and direct the cmdTraceStream to it
	    std::cout << "Setting cmd trace output to " << k_cmdTraceFileName << std::endl;
	    m_cmdTraceOFStream.open(k_cmdTraceFileName);
	    if(m_cmdTraceOFStream) {
	      m_cmdTraceStreamBuf = m_cmdTraceOFStream.rdbuf();
	    } else {
	      std::cerr << "Failed to open cmd trace output file " << k_cmdTraceFileName << ", redirecting to stdout";
	      m_cmdTraceStreamBuf = std::cout.rdbuf();
	    }
	  }
	  m_cmdTraceStream = new std::ostream(m_cmdTraceStreamBuf);
	}

	// Statistics
	//s_rowHits = registerStatistic<uint64_t>("rowHits");
}

c_CmdUnit::~c_CmdUnit() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	// for (unsigned l_i = 0; l_i != m_numBanks; ++l_i)
	// 	delete m_banks.at(l_i);
	//
	// for (unsigned l_i = 0; l_i != k_numBankGroups; ++l_i)
	// 	delete m_bankGroups.at(l_i);
}

/*
c_CmdUnit::c_CmdUnit() :
		Component(-1) {
	// for serialization only
}
*/

void c_CmdUnit::print() const {
	std::cout << "***CmdUnit " << SubComponent::getName() << std::endl;
	std::cout << "ReqQEntries=" << k_cmdReqQEntries << ", " << "ResQEntries="
			<< k_cmdResQEntries << std::endl;
	std::cout << "CmdReqQ size=" << m_inputQ.size() << ", " << "CmdResQ size="
			<< m_cmdResQ.size() << std::endl;
}

void c_CmdUnit::printQueues() {
	std::cout << "CmdReqQ: " << std::endl;
	for (auto& l_entry : m_inputQ) {
		l_entry->print();
		unsigned l_bankNum = l_entry->getHashedAddress()->getBankId();

		std::cout << " - Going to Bank " << std::dec << l_bankNum << std::endl;
	}

	std::cout << std::endl << "CmdResQ:" << std::endl;
	for (auto& l_entry : m_cmdResQ) {
		l_entry->print();
		std::cout << std::endl;
	}
}

bool c_CmdUnit::clockTic(SST::Cycle_t) {

	m_issuedACT = false;

	for (int l_i = 0; l_i != m_banks.size(); ++l_i) {
		m_banks.at(l_i)->clockTic();
		// m_banks.at(l_i)->printState();
	}
	run();
	sendRequest();

	m_cmdACTFAWtracker.push_back(m_issuedACT?static_cast<unsigned>(1):static_cast<unsigned>(0));
	m_cmdACTFAWtracker.pop_front();
}



/*
void c_CmdUnit::sendResponse() {

	if (m_cmdResQ.size() > 0) {

//		std::cout << std::endl << "@" << std::dec
//				<< Simulation::getSimulation()->getCurrentSimCycle() << ": "
//				<< __PRETTY_FUNCTION__ << std::endl;
//		m_cmdResQ.front()->print();
//		std::cout << std::endl;

		c_CmdResEvent* l_cmdResEventPtr = new c_CmdResEvent();
		l_cmdResEventPtr->m_payload = m_cmdResQ.front();
		m_cmdResQ.erase(m_cmdResQ.begin());
		m_outTxnUnitResPtrLink->send(l_cmdResEventPtr);

		// CmdUnit keeps track of its own res Q tokens
		++m_cmdResQTokens;
	}
}
*/

//
// Model a close bank policy as an approximation to a close row policy
// FIXME: update this to close row policy
//
// TODO: call this function only when a config knob k_closeBankPolicy is true
//
// Iterate through the cmdReqQ and mark the bank to which a cmd cannot go
// do not send any further cmds to that bank
void c_CmdUnit::sendReqCloseBankPolicy(
		std::deque<c_BankCommand*>::iterator x_startItr) {

	// get count of ACT cmds issued in the FAW
	unsigned l_cmdACTIssuedInFAW = 0;
	assert(m_cmdACTFAWtracker.size() == m_bankParams.at("nFAW"));
	for(auto& l_issued : m_cmdACTFAWtracker)
		l_cmdACTIssuedInFAW += l_issued;


	for (auto l_cmdPtrItr = x_startItr; l_cmdPtrItr != m_inputQ.end();
			++l_cmdPtrItr) {
		c_BankCommand* l_cmdPtr = (*l_cmdPtrItr);
		if ((l_cmdPtr)->getCommandMnemonic() == e_BankCommandType::REF)
			break;

		if ((e_BankCommandType::ACT == ((l_cmdPtr))->getCommandMnemonic()) && (l_cmdACTIssuedInFAW >= 4))
			continue;


		// block: READ after WRITE to the same address
		// block: WRITE after WRITE to the same address. Processor should make sure that the older WRITE is annulled but we will block the younger here.
		bool l_proceed = true;
		if (m_inflightWrites.find((l_cmdPtr)->getAddress())
		    != m_inflightWrites.end()) {
		  l_proceed = false;
		}

		// if this is a WRITE or WRITEA command insert it in the inflight set
		if ((e_BankCommandType::WRITE == (l_cmdPtr)->getCommandMnemonic())
				|| (e_BankCommandType::WRITEA
						== (l_cmdPtr)->getCommandMnemonic())) {

			m_inflightWrites.insert((l_cmdPtr)->getAddress());
		}

		if (l_proceed) {

			if (isCommandBusAvailable(l_cmdPtr)){   //check if the command bus is available

				unsigned l_bankNum = l_cmdPtr->getHashedAddress()->getBankId();
				c_BankInfo* l_bank = m_banks.at(l_bankNum);

				if (sendCommand((l_cmdPtr), l_bank)) {


					if (l_cmdPtr->isColCommand()) {
						assert( (m_lastDataCmdType != ((l_cmdPtr))->getCommandMnemonic()) ||
								(m_lastPseudoChannel != (l_cmdPtr->getHashedAddress()->getPChannel())) ||
						      (Simulation::getSimulation()->getCurrentSimCycle()-m_lastDataCmdIssueCycle) >= (std::min(m_bankParams.at("nBL"),std::max(m_bankParams.at("nCCD_L"),m_bankParams.at("nCCD_S")))));

						m_lastDataCmdIssueCycle = Simulation::getSimulation()->getCurrentSimCycle();
						m_lastDataCmdType = ((l_cmdPtr))->getCommandMnemonic();
						m_lastPseudoChannel = ((l_cmdPtr))->getHashedAddress()->getPChannel();
					}

					m_issuedACT = (e_BankCommandType::ACT == ((l_cmdPtr))->getCommandMnemonic());

					if(occupyCommandBus(l_cmdPtr))
						break;// all command buses are occupied, so stop
				}
			}
		}
	} // for
}

//
// Model a open row policy
// FIXME: update this to open row policy
//
// TODO: call this function only when a config knob k_closeBankPolicy is set to 1
//
// Iterate through the cmdReqQ and mark the bank to which a cmd cannot go
// do not send any further cmds to that bank
void c_CmdUnit::sendReqOpenRowPolicy() {

	c_BankCommand* l_openBankCmdPtr = nullptr;
	auto l_openBankCmdPtrItr = m_inputQ.begin();
	for (auto l_cmdPtr : m_inputQ) {
		if (l_cmdPtr->getCommandMnemonic() == e_BankCommandType::REF)
			break;

		bool l_isDataCmd = ((((l_cmdPtr))->getCommandMnemonic()
				== e_BankCommandType::READ)
				|| (((l_cmdPtr))->getCommandMnemonic()
						== e_BankCommandType::WRITE));
		ulong l_addr = ((l_cmdPtr))->getAddress();
		unsigned l_bankNum = l_cmdPtr->getHashedAddress()->getBankId();
		c_BankInfo* l_bankPtr = m_banks.at(l_bankNum);
        SimTime_t l_time = Simulation::getSimulation()->getCurrentSimCycle();

		if (l_isDataCmd && (l_bankPtr->isCommandAllowed(l_cmdPtr, l_time))
				&& (l_bankPtr->isRowOpen())
		    && (l_bankPtr->getOpenRowNum() == l_cmdPtr->getHashedAddress()->getRow()))
		{
			l_openBankCmdPtr = l_cmdPtr;

			break;
		} // if
	} // for
	if (nullptr == l_openBankCmdPtr) {

		sendReqCloseBankPolicy(m_inputQ.begin());
	} else {
		// found a command, so now remove older ACT and PRE commands
		std::list<c_BankCommand*> l_deleteList;
		l_deleteList.clear();
		for (auto l_cmdPtr : m_inputQ) {
			if (l_cmdPtr == l_openBankCmdPtr) {
				break;
			}

			bool l_isActPreCmd = ((((l_cmdPtr))->getCommandMnemonic()
					== e_BankCommandType::ACT)
					|| (((l_cmdPtr))->getCommandMnemonic()
							== e_BankCommandType::PRE));

			unsigned l_bankNum = l_cmdPtr->getHashedAddress()->getBankId();
			unsigned l_bankNumOpenBank =
			  l_openBankCmdPtr->getHashedAddress()->getBankId();

			if (l_isActPreCmd && (l_bankNum == l_bankNumOpenBank)) {
				l_deleteList.push_back(l_cmdPtr);
			}
		}
		// delete the PRE and ACT
		for (auto l_delPtr : l_deleteList) {
			// set this command as response ready
			l_delPtr->setResponseReady();
			// reclaim response queue tokens for eliminated commands
			if ((l_delPtr->getCommandMnemonic() == e_BankCommandType::ACT)
					&& k_allocateCmdResQACT) {
				++m_cmdResQTokens;
			}
			if ((l_delPtr->getCommandMnemonic() == e_BankCommandType::PRE)
					&& k_allocateCmdResQPRE) {
				++m_cmdResQTokens;
			}

			// erase entries in the command req q
			m_inputQ.erase(
					std::remove(m_inputQ.begin(), m_inputQ.end(), l_delPtr),
					m_inputQ.end());
		}
		sendReqCloseBankPolicy(
				std::find(m_inputQ.begin(), m_inputQ.end(),
						l_openBankCmdPtr));
	}

}

////////////////

//
// Model a open row policy where a timer closes a row, so no infinite open row
//
// TODO: call this function only when a config knob k_closeBankPolicy is set to 1
//
// Iterate through the cmdReqQ and mark the bank to which a cmd cannot go
// do not send any further cmds to that bank
void c_CmdUnit::sendReqPseudoOpenRowPolicy() {

	c_BankCommand* l_openBankCmdPtr = nullptr;
	auto l_openBankCmdPtrItr = m_inputQ.begin();
	for (auto l_cmdPtr : m_inputQ) {
		if (l_cmdPtr->getCommandMnemonic() == e_BankCommandType::REF)
			break;

		bool l_isDataCmd = ((((l_cmdPtr))->getCommandMnemonic()
				== e_BankCommandType::READ)
				|| (((l_cmdPtr))->getCommandMnemonic()
						== e_BankCommandType::WRITE));
		ulong l_addr = ((l_cmdPtr))->getAddress();
		unsigned l_bankNum = l_cmdPtr->getHashedAddress()->getBankId();
		c_BankInfo* l_bankPtr = m_banks.at(l_bankNum);
        SimTime_t l_time = Simulation::getSimulation()->getCurrentSimCycle();

		if (l_isDataCmd && (l_bankPtr->isCommandAllowed(l_cmdPtr, l_time))
				&& (l_bankPtr->isRowOpen())
		    && (l_bankPtr->getOpenRowNum() == l_cmdPtr->getHashedAddress()->getRow())
				&& (l_bankPtr->getAutoPreTimer() > 0)) {
			l_openBankCmdPtr = l_cmdPtr;

			break;
		} // if
	} // for
	if (nullptr == l_openBankCmdPtr) {

		sendReqCloseBankPolicy(m_inputQ.begin());
	} else {
		// found a command, so now remove older ACT and PRE commands
		std::list<c_BankCommand*> l_deleteList;
		l_deleteList.clear();
		for (auto l_cmdPtr : m_inputQ) {
			if (l_cmdPtr == l_openBankCmdPtr) {
				break;
			}

			bool l_isActPreCmd = ((((l_cmdPtr))->getCommandMnemonic()
					== e_BankCommandType::ACT)
					|| (((l_cmdPtr))->getCommandMnemonic()
							== e_BankCommandType::PRE));
			unsigned l_bankNum = l_cmdPtr->getHashedAddress()->getBankId();
			unsigned l_bankNumOpenBank =
			  l_openBankCmdPtr->getHashedAddress()->getBankId();

			if (l_isActPreCmd && (l_bankNum == l_bankNumOpenBank)) {
				l_deleteList.push_back(l_cmdPtr);
			}
		}
		// delete the PRE and ACT
		for (auto l_delPtr : l_deleteList) {
			// set this command as response ready
			l_delPtr->setResponseReady();
			// reclaim response queue tokens for eliminated commands
			if ((l_delPtr->getCommandMnemonic() == e_BankCommandType::ACT)
					&& k_allocateCmdResQACT) {
				++m_cmdResQTokens;
			}
			if ((l_delPtr->getCommandMnemonic() == e_BankCommandType::PRE)
					&& k_allocateCmdResQPRE) {
				++m_cmdResQTokens;
			}

			// erase entries in the command req q
			m_inputQ.erase(
					std::remove(m_inputQ.begin(), m_inputQ.end(), l_delPtr),
					m_inputQ.end());
		}
		sendReqCloseBankPolicy(
				std::find(m_inputQ.begin(), m_inputQ.end(),
						l_openBankCmdPtr));
	}

}

//////////////

//
// Model a open bank policy
// this is approximation to the open row policy
//
// TODO: call this function only when a config knob k_closeBankPolicy is set to 2
//
// Iterate through the cmdReqQ and mark the bank to which a cmd cannot go
// do not send any further cmds to that bank
void c_CmdUnit::sendReqOpenBankPolicy() {

	c_BankCommand* l_openBankCmdPtr = nullptr;
	auto l_openBankCmdPtrItr = m_inputQ.begin();
	for (auto l_cmdPtr : m_inputQ) {
		if (l_cmdPtr->getCommandMnemonic() == e_BankCommandType::REF)
			break;

		bool l_isDataCmd = ((((l_cmdPtr))->getCommandMnemonic()
				== e_BankCommandType::READ)
				|| (((l_cmdPtr))->getCommandMnemonic()
						== e_BankCommandType::WRITE));
		ulong l_addr = ((l_cmdPtr))->getAddress();
		unsigned l_bankNum = l_cmdPtr->getHashedAddress()->getBankId();
		c_BankInfo* l_bankPtr = m_banks.at(l_bankNum);
        SimTime_t l_time = Simulation::getSimulation()->getCurrentSimCycle();


		if (l_isDataCmd && (l_bankPtr->isCommandAllowed(l_cmdPtr, l_time))) {
			l_openBankCmdPtr = l_cmdPtr;

			break;
		} // if
	} // for
	if (nullptr == l_openBankCmdPtr) {

		sendReqCloseBankPolicy(m_inputQ.begin());
	} else {
		// found a command, so now remove older ACT and PRE commands
		std::list<c_BankCommand*> l_deleteList;
		l_deleteList.clear();
		for (auto l_cmdPtr : m_inputQ) {
			if (l_cmdPtr == l_openBankCmdPtr) {
				break;
			}

			bool l_isActPreCmd = ((((l_cmdPtr))->getCommandMnemonic()
					== e_BankCommandType::ACT)
					|| (((l_cmdPtr))->getCommandMnemonic()
							== e_BankCommandType::PRE));
			unsigned l_bankNum = l_cmdPtr->getHashedAddress()->getBankId();
			unsigned l_bankNumOpenBank =
			  l_openBankCmdPtr->getHashedAddress()->getBankId();

			if (l_isActPreCmd && (l_bankNum == l_bankNumOpenBank)) {
				l_deleteList.push_back(l_cmdPtr);
			}
		}
		// delete the PRE and ACT
		for (auto l_delPtr : l_deleteList) {
			// set this command as response ready
			l_delPtr->setResponseReady();
			// reclaim response queue tokens for eliminated commands
			if ((l_delPtr->getCommandMnemonic() == e_BankCommandType::ACT)
					&& k_allocateCmdResQACT) {
				++m_cmdResQTokens;
			}
			if ((l_delPtr->getCommandMnemonic() == e_BankCommandType::PRE)
					&& k_allocateCmdResQPRE) {
				++m_cmdResQTokens;
			}

			// erase entries in the command req q
			m_inputQ.erase(
					std::remove(m_inputQ.begin(), m_inputQ.end(), l_delPtr),
					m_inputQ.end());
		}
		sendReqCloseBankPolicy(
				std::find(m_inputQ.begin(), m_inputQ.end(),
						l_openBankCmdPtr));
	}
}

void c_CmdUnit::run() {

	// if REFs are enabled, check the Req Q's head for REF
	if (k_useRefresh && m_inputQ.size() > 0) {
		if (m_inputQ.front()->getCommandMnemonic() == e_BankCommandType::REF) {
			sendRefresh();
			return;
		}
	}

	// Requests to the bank are only sent out if the res q has space to
	// accept them when they come back
//	std::cout << std::endl << "@" << std::dec
//			<< Simulation::getSimulation()->getCurrentSimCycle() << ": "
//			<< __PRETTY_FUNCTION__;
//	std::cout << ": m_inputQ.size() = " << m_inputQ.size()
//			<< " m_cmdResQTokens = " << m_cmdResQTokens << std::endl;

	//if (m_inputQ.size() > 0 && m_cmdResQTokens > 0) {
	if (m_inputQ.size() > 0 ) {
//		 std::cout << std::endl << "@" << std::dec
//		 		<< Simulation::getSimulation()->getCurrentSimCycle() << ": "
//		 		<< __PRETTY_FUNCTION__ << std::endl;

		// do the member var setup up before calling any req sending policy function
		if(m_inflightWrites.size()>0)
			m_inflightWrites.clear();
		releaseCommandBus();  //update the command bus status

		switch (k_bankPolicy) {
		case 0:
			sendReqCloseBankPolicy(m_inputQ.begin());
			break;
		case 1:
			sendReqOpenRowPolicy();
			break;
		case 2:
			sendReqOpenBankPolicy();
			break;
		case 3:
			sendReqPseudoOpenRowPolicy();
			break;
		default:
			std::cout << "ERROR: k_bankPolicy not set to the support choice";
			exit(-1);
		}
	}
}

void c_CmdUnit::sendRefresh() {
//  std::cout << std::endl << "@" << std::dec
 // 		    << Simulation::getSimulation()->getCurrentSimCycle() << ": "
 // 		    << __PRETTY_FUNCTION__ << std::endl;

  c_BankCommand *l_refCmd = m_inputQ.front();
  assert(l_refCmd->getCommandMnemonic() == e_BankCommandType::REF);

  // determine if all banks are ready to refresh
  for(auto l_bankId : *(l_refCmd->getBankIdVec())) {
    c_BankInfo *l_bank = m_banks.at(l_bankId);
      SimTime_t l_time = Simulation::getSimulation()->getCurrentSimCycle();
    if (!l_bank->isCommandAllowed(l_refCmd, l_time)) { // can't send all refs now, cancel!
      return;
    }
  }

  bool l_first = true;
  c_BankCommand *l_cmdToSend = l_refCmd;
  for(auto l_bankId : *(l_refCmd->getBankIdVec())) {
    c_BankInfo *l_bank = m_banks.at(l_bankId);
    if(l_first) {
      l_first = false;
    } else {
      // make a new bank command for each bank after the first
      // each bank deletes their own command
      l_cmdToSend = new c_BankCommand(l_refCmd->getSeqNum(), l_refCmd->getCommandMnemonic(),
				      0, l_bankId);
    }
    if(sendCommand(l_cmdToSend, l_bank)) {
      m_refsSent++;
    } else {
      assert(0);
    }
  }

}


/**
 *
 * @param l_cmdPtr
 * @return "true" if the command bus required for the input command is available.
 */
bool c_CmdUnit::isCommandBusAvailable(c_BankCommand *l_cmdPtr) {
	uint32_t l_ChannelNum = l_cmdPtr->getHashedAddress()->getChannel();
	bool l_isColCmd = l_cmdPtr->isColCommand();


	if(l_isColCmd)
		return (m_blockColCmd.at(l_ChannelNum)==0);
	else
		return (m_blockRowCmd.at(l_ChannelNum)==0);
}


/**
 *
 * @param l_cmdPtr
 * @return "true" if all buses are occupied.
 */
bool c_CmdUnit::occupyCommandBus(c_BankCommand *l_cmdPtr) {
	uint32_t l_ChannelNum=0;
	uint32_t l_NumAvailableBus=0;

	l_ChannelNum = l_cmdPtr->getHashedAddress()->getChannel();
	uint32_t l_cmdCycle;

	if(l_cmdPtr->getCommandMnemonic()== e_BankCommandType::ACT && k_multiCycleACT)
			l_cmdCycle=2;			//HBM requires two cycles for the active command
	else
			l_cmdCycle=1;

	//Occupy the command bus
	if (k_useDualCommandBus) {
		if (l_cmdPtr->isColCommand())
			m_blockColCmd.at(l_ChannelNum) = l_cmdCycle;
		else
			m_blockRowCmd.at(l_ChannelNum) = l_cmdCycle;
	}
	else {
		m_blockColCmd.at(l_ChannelNum) = 1;
		m_blockRowCmd.at(l_ChannelNum) = 1;
	}

	//Check whether all command buses are occupied
	for(auto & value: m_blockColCmd)
		if(value == 0)
			l_NumAvailableBus++;

	for(auto & value: m_blockRowCmd)
		if(value == 0)
			l_NumAvailableBus++;

	if(l_NumAvailableBus>0) {
		return false;
	}
	else {
		return true;
	}
}

/**
 *
 */
void c_CmdUnit::releaseCommandBus() {
	for(auto & value: m_blockColCmd)
	{
		if(value>0) value--;
	}

	for(auto & value: m_blockRowCmd)
	{
		if(value>0) value--;
	}
}


bool c_CmdUnit::sendCommand(c_BankCommand* x_bankCommandPtr,
		c_BankInfo* x_bank) {
    SimTime_t l_time = Simulation::getSimulation()->getCurrentSimCycle();

	if (x_bank->isCommandAllowed(x_bankCommandPtr, l_time)) {
	  if(k_printCmdTrace) {
	    if(x_bankCommandPtr->getCommandMnemonic() == e_BankCommandType::REF) {
	      (*m_cmdTraceStream) << "@" << std::dec
				  << Simulation::getSimulation()->getCurrentSimCycle()
				  << " " << (x_bankCommandPtr)->getCommandString()
				  << " " << std::dec << (x_bankCommandPtr)->getSeqNum();
		//<< " " << std::dec << x_bankCommandPtr->getBankId()
	      for(auto l_bankId : *(x_bankCommandPtr->getBankIdVec())) {
		std::cout << " " << l_bankId;
	      }
	      std::cout << std::endl;
	    } else {
	      (*m_cmdTraceStream) << "@" << std::dec
				  << Simulation::getSimulation()->getCurrentSimCycle()
				  << " " << (x_bankCommandPtr)->getCommandString()
				  << " " << std::dec << (x_bankCommandPtr)->getSeqNum()
				  << " 0x" << std::hex << (x_bankCommandPtr)->getAddress()
				  << " " << std::dec << x_bankCommandPtr->getHashedAddress()->getChannel()
				  << " " << std::dec << x_bankCommandPtr->getHashedAddress()->getRank()
				  << " " << std::dec << x_bankCommandPtr->getHashedAddress()->getBankGroup()
				  << " " << std::dec << x_bankCommandPtr->getHashedAddress()->getBank()
				  << " " << std::dec << x_bankCommandPtr->getHashedAddress()->getRow()
				  << " " << std::dec << x_bankCommandPtr->getHashedAddress()->getCol()
				  << " " << std::dec << x_bankCommandPtr->getHashedAddress()->getCacheline()
				  << "\t" << std::dec << x_bankCommandPtr->getHashedAddress()->getBankId()
				  << std::endl;
	    }
	  }

	        //std::cout << std::endl << "@" << std::dec
		//	  << Simulation::getSimulation()->getCurrentSimCycle() << ": "
		//	  << __PRETTY_FUNCTION__ << ": Sent command " << std::endl;
		//(x_bankCommandPtr)->print();

		// send command to BankState
		x_bank->handleCommand(x_bankCommandPtr, l_time);

//		 printQueues();
//		 std::cout << "Giving BankState a cmd ";
//		 x_bank->print();
//		 std::cout << ": ";
//		 x_bankCommandPtr->print();
//		 std::cout << std::endl;

		// send command to Dimm component
		/*c_CmdReqEvent* l_cmdReqEventPtr = new c_CmdReqEvent();
		l_cmdReqEventPtr->m_payload = x_bankCommandPtr;
		m_outBankReqPtrLink->send(l_cmdReqEventPtr);*/
		m_outputQ.push_back(x_bankCommandPtr);

		// Decrease token if we are allocating CmdResQ space.
		// CmdUnit keeps track of its own res Q tokens
		bool l_doAllocateSpace = false;
		switch (x_bankCommandPtr->getCommandMnemonic()) {
		case e_BankCommandType::ACT:
			l_doAllocateSpace = k_allocateCmdResQACT;
			break;
		case e_BankCommandType::READ:
			l_doAllocateSpace = k_allocateCmdResQREAD;
			break;
		case e_BankCommandType::READA:
			l_doAllocateSpace = k_allocateCmdResQREADA;
			break;
		case e_BankCommandType::WRITE:
			l_doAllocateSpace = k_allocateCmdResQWRITE;
			break;
		case e_BankCommandType::WRITEA:
			l_doAllocateSpace = k_allocateCmdResQWRITEA;
			break;
		case e_BankCommandType::PRE:
			l_doAllocateSpace = k_allocateCmdResQPRE;
			break;
		default:
		    break;
		}
//		if (l_doAllocateSpace)
//			m_cmdResQTokens--;

		// remove cmd from ReqQ
//		m_inputQ.remove(x_bankCommandPtr);
		m_inputQ.erase(
				std::remove(m_inputQ.begin(), m_inputQ.end(),
					    x_bankCommandPtr), m_inputQ.end());

		return true;
	} else
		return false;
}


void c_CmdUnit::sendRequest() {
	//int token=m_Owner->getToken();
	while(!m_outputQ.empty()) {
		m_Owner->sendCommand(m_outputQ.front());
		m_outputQ.pop_front();
	}
};
