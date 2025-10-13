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
            // bounds check
            if (e.workspaceId >= 0 && e.workspaceId < static_cast<int>(workspaces.size())) {
                Workspace& station = workspaces[e.workspaceId];
                if (e.machineId >= 0 && e.machineId < static_cast<int>(station.machines.size())) {
                    int jId = station.finishMachine(e.machineId);
                    if (jId >= 0 && jId < static_cast<int>(jobs.size())) {
                        progressJob(jId);
                    }
                }
            }
            break;
        }
        case EventType::POLICY_CHECK:
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
    // Safety check: validate job ID
    if (jId < 0 || jId >= static_cast<int>(jobs.size())) {
        return;
    }
    
    Job& job = jobs[jId];
    job.workspace++;
    if (job.workspace >= workspaces.size()) { // if at the last workspace
        job.status = JobStatus::COMPLETED;
        totalTh++;
    }
    else {
        job.status = JobStatus::IDLE;
        // Double-check workspace index before accessing
        if (job.workspace < workspaces.size()) {
            workspaces[job.workspace].wip.push_back(jId);
        }
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
                // Limit job creation to prevent unbounded memory growth (max 1000 active jobs)
                if (ws.anyIdle() && ws.id == 0 && jobs.size() < 1000) {
                    jobs.emplace_back(static_cast<int>(jobs.size()), JobStatus::IDLE, now);
                    ws.wip.push_back(jobs.back().id);
                } 
                if (ws.anyIdle() && ws.anyWIP()) {
                    int wipJobId = ws.findWIP();
                    // Safety check: ensure job ID is valid before accessing
                    if (wipJobId >= 0 && wipJobId < static_cast<int>(jobs.size())) {
                        Event newServiceEnd = ws.startMachine(jobs[wipJobId], now);
                        schedule(newServiceEnd);
                    }
                }
            }
            break;
    }


}

#ifdef __EMSCRIPTEN__
void Sim::loadFromConfigString(const std::string& jsonString) { // overloaded method for binding for web interface
    nlohmann::json parsedConfig = nlohmann::json::parse(jsonString); // parses that string
    loadFromJsonConfig(parsedConfig);
}
#endif

void Sim::loadFromConfig(const std::string& configPath) { // overloaded method for .json files
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        throw std::runtime_error("Could not open config file: " + configPath);
    }
    
    nlohmann::json config;
    configFile >> config;
    configFile.close();
    
    loadFromJsonConfig(config);
}

void Sim::loadFromJsonConfig(const nlohmann::json& config) {
    workspaces.clear();
    
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
        now = e.time;
        handleEvent(e);
    }
}

void Sim::initialize() {
    // clear any previous simulation state
    jobs.clear();
    totalTh = 0;
    now = 0;
    while (!timeline.empty()) {
        timeline.pop();
    }
    
    for (Workspace& ws : workspaces) {
        ws.wip.clear();
        // reset all machines to idle state with no job references
        for (Machine& m : ws.machines) {
            m.busy = false;
            m.jobId = -1;
        }
    }
    
    // schedule first policy check to start the simulation
    Event polCheck(0.0, EventType::POLICY_CHECK, -1, -1);
    schedule(polCheck);
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