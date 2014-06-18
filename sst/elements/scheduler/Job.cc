// Copyright 2009-2014 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2014, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include "sst_config.h"
#include "Job.h"

#include <stdio.h>
#include <string>

#include "exceptions.h"
#include "Machine.h"
#include "misc.h"
#include "output.h"
#include "Statistics.h"
#include "TaskCommInfo.h"

using namespace SST::Scheduler;

static long nextJobNum = 0;  //used setting jobNum

Job::Job(unsigned long arrivalTime, int procsNeeded, unsigned long actualRunningTime, unsigned long estRunningTime) 
{
    schedout.init("", 8, 0, Output::STDOUT);
    initialize(arrivalTime, procsNeeded, actualRunningTime, estRunningTime);
}

Job::Job(long arrivalTime, int procsNeeded, long actualRunningTime,
         long estRunningTime, std::string ID) 
{
    initialize(arrivalTime, procsNeeded, actualRunningTime, estRunningTime);
    this -> ID = ID;
}

//copy constructor
Job::Job(Job* j)
{
    arrivalTime = j -> arrivalTime;
    procsNeeded = j -> procsNeeded;
    actualRunningTime = j -> actualRunningTime;
    estRunningTime = j -> estRunningTime;
    jobNum = j -> jobNum;
    ID = j -> ID;
    startTime = j -> startTime;
    hasRun = j -> hasRun;
    started = j -> started;
}

Job::~Job()
{
	if(taskCommInfo != NULL){
		delete taskCommInfo;
	}
}

//Helper for constructors
void Job::initialize(unsigned long arrivalTime, int procsNeeded,
                     unsigned long actualRunningTime, unsigned long estRunningTime) 
{

    //make sure estimate is valid; workload log uses -1 for "no estimate"
    if (estRunningTime < actualRunningTime || (unsigned long)-1 == estRunningTime) {
        estRunningTime = actualRunningTime;
    }

    this -> arrivalTime = arrivalTime;
    this ->  procsNeeded = procsNeeded;
    this -> actualRunningTime = actualRunningTime;
    this -> estRunningTime = estRunningTime;

    startTime = -1;

    jobNum = nextJobNum;
    nextJobNum++;
    hasRun = false;
    started = false;
    taskCommInfo = NULL;
}

std::string Job::toString() 
{
    char retVal[100];
    snprintf(retVal, 100, "Job #%ld (%ld, %d, %ld, %ld, null)", jobNum,
             arrivalTime, procsNeeded, actualRunningTime, estRunningTime);
    return retVal;
}

//starts a job
void Job::start(unsigned long time, Machine* machine, AllocInfo* allocInfo,
                Statistics* stats) 
{
    if ((unsigned long)-1 != startTime) {
        schedout.fatal(CALL_INFO, 1, "attempt to start an already-running job: %s\n", toString().c_str());
    }
    started = true; 

    startTime = time;
    machine -> allocate(allocInfo);
    stats -> jobStarts(allocInfo, time);
}

void Job::reset()
{
    startTime = -1;
    hasRun = false;
    started = false;
}

void Job::startsAtTime(unsigned long time)
{
    startTime = time;
    hasRun = true; 
    started = true; 
}

bool Job::hasStarted() {
    return started;
}

