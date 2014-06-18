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

#ifndef SST_SCHEDULER_TASKMAPPER_H__
#define SST_SCHEDULER_TASKMAPPER_H__

#include "AllocInfo.h"
#include "Job.h"
#include "TaskCommInfo.h"
#include "TaskMapInfo.h"

typedef boost::bimaps::bimap< int, int > taskMapType;

namespace SST {
    namespace Scheduler {

        class AllocInfo;
        class TaskMapInfo;
        class TaskCommInfo;

        class TaskMapper {

            public:
		        TaskMapper(Job *job) { this -> job = job; }

		        virtual ~TaskMapper() {};

		        //returns task mapping info of a single job; does not map the tasks
		        virtual TaskMapInfo* mapTasks(Machine* mach, AllocInfo* allocInfo, TaskCommInfo* tci) = 0;

		        Job* job;

	        protected:
		        //Machine* machine;
        };
    }
}
#endif /* SST_SCHEDULER_TASKMAPPER_H__ */
