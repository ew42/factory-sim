#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <queue>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include "types.h"
#include "events.h"
#include "factory.h"
#include "json.hpp"
#include "state_io.h"

struct Sim {
    /*
        Good thing to think about is that Workspace, Machine, or Job
        should NEVER control what actually gets loaded. All of the
        policy logic should be handled inside of sim.
            - Example of how that works in practice
                Workspace's startMachine method should take in a job rather than
                itself looking at all of its jobs
        
        Also, Sim shouldn't have to update jobs, machines, or workspaces as
        idle/active or move jobs between stations -- it should only ever
        query that information

    1) on SERVICE_END, what should it do?
        - move the finished job to the next workspace and check if it has idle machines, if so schedule new event
        - check if the current workspace has wip and load if it does, if so schedule new event
    2) on RELEASE_TICK, what should it do?
        - call policy ontick and schedule another release_tick
        
    methods:
        run -- loads the first material release, starts running the clock, and going through the heap
        schedule -- takes an event, puts it into the heap
        handleEvent -- takes in events and processes, see 1) above for SERVICE_END and 2) for RELEASE_TICK

    */
    int totalTh = 0; // total throughput
    double now = 0; // time var
    double delay = 0.5; // policy tick every 0.5 seconds
    double stepSize = 1;
    std::priority_queue<Event, std::vector<Event>, Earlier> timeline;
    std::vector<Workspace> workspaces;
    std::vector<Job> jobs;
    PolicyType policy = PolicyType::RUN_IMMEDIATELY;
    
    void run();
    void handleEvent(const Event& e);
    void schedule(const Event& e);
    void progressJob(int jId);
    void runPolicy();
    void loadFromConfigString(const std::string& jsonString); // web version
    void loadFromConfig(const std::string& configPath); // local version
    void stepUntil(double runUntil);
    std::vector<WorkspaceView> getWorkspaceView();
    MetricsView getMetricsView();

private:
    void loadFromJsonConfig(const nlohmann::json& config);
};

#endif // SIMULATION_H