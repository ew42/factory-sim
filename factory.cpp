#include "factory.h"

Job::Job(unsigned int idNum, JobStatus stat, double release)
    : id(idNum), workspace(0), status(stat), releaseTime(release) {}

Workspace::Workspace(int wsId, double mean, double stdDev, int machineNum)
    : id(wsId), dist(mean, stdDev), machines(machineNum) {}

int Workspace::finishMachine(int mId) {
    // return jId
    machines[mId].busy = false;
    int jId = machines[mId].jobId;
    machines[mId].jobId = -1;
    
    // remove job from wip
    auto it = std::find(wip.begin(), wip.end(), jId);
    if (it != wip.end()) {
        wip.erase(it);
    }
    
    return jId;
}    

Event Workspace::startMachine(Job& job, double startTime) { // should only be called after calling anyIdle
    job.status = JobStatus::ACTIVE;
    int machineId = findIdle();
    Machine& machine = machines[machineId];
    machine.busy = true;
    machine.jobId = job.id;
    
    double delay = dist.sample();
    
    return Event(startTime + delay, EventType::SERVICE_END, job.workspace, machineId);
}

bool Workspace::anyIdle() {
    for (const Machine& machine : machines) {
        if (!machine.busy) {
            return true;
        }
    }
    return false;
}

int Workspace::findIdle() { // returns index of an idle machine, if none, return -1
    for (size_t i = 0; i < machines.size(); i++) {
        if (!machines[i].busy) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool Workspace::anyWIP() {
    return wip.size() > 0;
}

int Workspace::findWIP() { // only call after anyWIP
    return wip.front();
}

