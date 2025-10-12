#include "simulation.h"

void Sim::run() {
    Event polCheck(0.0, EventType::POLICY_CHECK, -1, -1); // first policy check event
    schedule(polCheck);

    while (timeline.size() > 0) { // Event Loop, 
        Event e = timeline.top(); // pop doesn't return the element popped, so need this line
        timeline.pop();
        now = e.time;
        handleEvent(e);
    }
}

void Sim::handleEvent(const Event& e) {
    /*
     * Question of, should handleEvent update all the Job stuff or should that be dealt
     * with by the methods of the workspace?
     *
     *
    */
    switch(e.type) {
        case EventType::SERVICE_END:
        {
            Workspace& station = workspaces[e.workspaceId];
            int jId = station.finishMachine(e.machineId);
            progressJob(jId);
            break;
        }
        case EventType::POLICY_CHECK:
            // Workspace firstWS = workspaces[0];
            // if (firstWS.anyIdle()) { // this is where a policy check would go
            //                  /*
            //                   * right now this checks if there are any idle machines at the original workspace
            //                   * then, releases a job to it, schedules a serviceEnd, and inserts a release tick at now + delay
            //                  */
            //     Job newJob = Job(static_cast<unsigned int>(jobs.size()), JobStatus::IDLE, now);
            //     jobs.push_back(newJob);
            //     Event new = firstWS.startMachine(jobs.back(), now);
            //     schedule(newReleaseEvent); 
            // }
            //                 Event newServiceEnd = (now + delay, EventType:MATERIAL_TICK, -1, -1);
            //                 schedule(newServiceEnd);
            runPolicy();
            Event newPolicyCheck = Event(now + delay, EventType::POLICY_CHECK, -1, -1);
            schedule(newPolicyCheck);
            break;
    }
}

void Sim::schedule(const Event& e) {
    timeline.push(e);
}

void Sim::progressJob(int jId) {
    /*
     * called when a machine has finished working on a job
     *
     * what should it do?
     * 1) either progress the job to the next station or account for it being complete
     *  - change status, workspace and machine to account for that
     *  - need to update the workspace it moves to's wip vector
    */
    Job& job = jobs[jId];
    job.workspace++;
    if (job.workspace >= workspaces.size()) { // if at the last workspace
        job.status = JobStatus::COMPLETED;
        totalTh++;
    }
    else {
        job.status = JobStatus::IDLE;
        workspaces[job.workspace].wip.push_back(jId);
    }
}

void Sim::runPolicy() {
    /*
     * Go through each workspace and check if it should be loaded
     *  - including workspace 0, aka a material release
    */
    switch(policy) {
        case PolicyType::RUN_IMMEDIATELY:
            for (Workspace& ws : workspaces) {
                // check if there's any wip, check if there's any free machines, and then load them
                if (ws.anyIdle() && ws.id == 0) {
                    jobs.emplace_back(static_cast<int>(jobs.size()), JobStatus::IDLE, now);
                    ws.wip.push_back(jobs.back().id);
                } 
                if (ws.anyIdle() && ws.anyWIP()) {
                    Event newServiceEnd = ws.startMachine(jobs[ws.findWIP()], now);
                    schedule(newServiceEnd);
                }
            }
            break;
    }


}

void Sim::loadFromConfig(const std::string& configPath) {
    // Read and parse the JSON configuration file
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        throw std::runtime_error("Could not open config file: " + configPath);
    }
    
    nlohmann::json config;
    configFile >> config;
    configFile.close();
    
    // Clear existing workspaces
    workspaces.clear();
    
    // Create workspaces from configuration
    const auto& stations = config["stations"];
    for (size_t i = 0; i < stations.size(); ++i) {
        const auto& station = stations[i];
        
        int wsId = static_cast<int>(i);
        int machineNum = station["m"];
        double mean = station["mean"];
        double stdev = station["stdev"];
        
        workspaces.emplace_back(wsId, mean, stdev, machineNum);
    }
}
void Sim::stepUntil(double runUntil) {
    while (timeline.size() != 0 && timeline.top().time <= runUntil) {
        Event e = timeline.top();
        timeline.pop();
        handleEvent(e);
    }
}
std::vector<WorkspaceView> Sim::getWorkspaceView() {
    std::vector<WorkspaceView> ret;

    for (const Workspace& station : workspaces) {
        int id = station.id;
        int m = static_cast<int>(station.machines.size());
        int busy = std::count_if(station.machines.begin(), station.machines.end(), 
                                [](const Machine& machine) { return machine.busy; });
        int wip = static_cast<int>(station.wip.size());
        ret.push_back(WorkspaceView{id, m, busy, wip});
    }
    return ret;
}

MetricsView Sim::getMetricsView() {
    double time = now;
    double avgTh = (time > 0) ? static_cast<double>(totalTh) / time : 0.0; // need to control for divide by 0
    int wip = 0;
    
    for (const Workspace& station : workspaces) {
        wip += static_cast<int>(station.wip.size());
    }
    
    return MetricsView{time, avgTh, wip};

}