#ifndef FACTORY_H
#define FACTORY_H

#include <vector>
#include <algorithm>
#include "types.h"
#include "utils.h"
#include "events.h"

struct Job {
    /*
     * Data I want:
     * 1) id
     * 2) what workspace am I on?
     * 3) what is my status?
     * 		ex: traveling, finished, being worked on, idle(in workspace but no work being done)
     * 4) time released
     *
     * Methods I want:
     * constructor
     * stageStart
     * 	- change status to active
     * stageFinish
     * 	
     *
     *
     * 
     *
     */
    unsigned int id; // aka the index of this job in the Sim's jobs vector
    unsigned int workspace; 
    JobStatus status;
    double releaseTime;

    Job(unsigned int idNum, JobStatus stat, double release);
};

struct Machine {
    int jobId = -1;
    bool busy = false;
};

struct Workspace {
    /*
     * Data I want:
     * 1) id (in order, i.e. 1 -> 2 -> -> 3)
     * 2) Vec of machines
     * 3) Queue of jobs waiting for machines
     * 4) Total WIP size (queue + active jobs)
     * 5) a Distribution
     * How do I track busy?
     */
    int id;
    std::vector<Machine> machines; // Machine contains jobId(int) and busy(bool)
    std::vector<int> queue; // jobs waiting for machines
    int wipSize; // total WIP at this workspace (queue + active jobs)
    NormDist dist;

    Workspace(int wsId, double mean, double stdDev, int machineNum); // need to update to include num of machines

    int finishMachine(int mId);
    Event startMachine(Job& job, double time);
    bool anyIdle();
    int findIdle(); // returns index of an idle machine, if none, return -1
    bool anyQueue();
    int findQueue(); // only call after anyQueue
};

#endif // FACTORY_H

