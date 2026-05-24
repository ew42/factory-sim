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
            workspaces[job.workspace].queue.push_back(jId);
            workspaces[job.workspace].wipSize++;
        }
    }
}

void Sim::runPolicy() {
    /*
     * Go through each workspace and check if it should be loaded
     *  - including workspace 0, aka a material release
    */
    switch(policy) { // switch statement currently only controls when material is released
        case PolicyType::RUN_IMMEDIATELY:
            // create one new job for workspace 0 if it has capacity and we're under job limit
            if (workspaces.size() > 0 && workspaces[0].anyIdle() && !workspaces[0].anyQueue() && jobs.size() < 1000) {
                jobs.emplace_back(static_cast<int>(jobs.size()), JobStatus::IDLE, now);
                workspaces[0].queue.push_back(jobs.back().id);
                workspaces[0].wipSize++;
            }
            break;
		case PolicyType::DRUM_BUFFER_ROPE:
			/*
			 * DBR logic should be pretty simple,
			 * but I need to update structs in order to get the data needed to properly release
			 * 		- need to know which workspace is the bottleneck
			 * 		- need the average time it takes for a job to get from release -> bottleneck queue
			 * 		- buffer multiplier in Sim
			 *
			 *
			 * Little's Law: WIP = Throughput * Average Time in System
			 * Modified for DBR: WIP = Throughput of Bottleneck Workspace * (Average Lead Time pre-Bottleneck * Buffer Multiplier)
			 * We then target that calculated WIP, if pre-bottleneck WIP < target WIP -> release material
			 * More explicitly: Target WIP = (number of machines at bottleneck * 1 job/machine mean completion time) * (sum of workspace means pre-bottleneck * buffer multiplier)
			 *
			 * Drum-Buffer-Rope's goal is to minimize work in progress inventory while keeping a bottleneck process constantly stocked
			 * The Drum is the throughput of the bottleneck, the processes in the system should **march** to its beat
			 * The Rope is that the material release/non-bottlenecks should be tied to the speed of the drum
			 * The Buffer is that material should be released with enough time such that it arrives early, in spite of variance
			 * 		ex: release if it takes 1 unit of time for material to reach to bottleneck, release it 1.5/2 units before bottleneck completion
			 *
			 * Outline of code implementation:
			 * 		1) Each workspace has a bool, isDrum | only one workspace should have this set as true
			 * 		2) Sim has a pre-drum WIP
			 * 			- if == -1, sum all wipSize of workspaces before the drum
			 * 				- set to -1 at startup
			 * 		3) Sim has an int target WIP,
			 * 			- if -1, sum the means of all workspaces prior to the drum
			 * 				- -1 is a value that means "my value isn't meaningful, you must calculate"
			 * 				- will be set to -1 at startup/if NormDist or the Machines vector fields are changed in any workspaces
			 * 			- if pre-drum WIP < targetWIP
			 * 				- releaseMaterial
			 *
			 *		Current questions:
			 *			- what happens if bottleneck is workspace[0]?
			 *				- currently that means that mean = 0, and targetWIP = 0
			 *				- 
			 *		Follow up:
			 *			- right now, you can't change any features of the factory while it's running,
			 *			  you can only restart and change things
			 *			- if I want to enable changing during operations, I'd need to make sure that I set pre-drum WIP and target WIP to -1 whenever changes occur
			 */
			preDrumWIP = 0;
			for (Workspace& ws: workspaces) {
				if (!ws.isDrum()) { // getWIPCount includes jobs currently inside machines -- we don't count WIP inside drum as preDrumWIP
					preDrumWIP += ws.getWIPCount();
				}
				else {
					preDrumWIP += ws.getQueueSize();
					break;
				}
			}
			if (targetWIP == -1) {
				double avgPreDrumLeadTime = 0; // here's where I should deal with bottleneck being workspace[0]
				double bottleneckThroughput = -1;
				for (Workspace& ws : workspaces) {
					if (ws.isDrum()) {
						bottleneckThroughput = static_cast<double>(ws.getMachineCount()) * (1/ws.getMean());
						break;
					}
					else avgPreDrumLeadTime += ws.getMean();
				}

				targetWIP = static_cast<int>(std::ceil(bottleneckThroughput * (avgPreDrumLeadTime * bufferMultiplier)));
				std::cout << "Target WIP: " << targetWIP << std::endl;
			}

			if (preDrumWIP < targetWIP) {
				if (workspaces.size() > 0) {
					jobs.emplace_back(static_cast<int>(jobs.size()), JobStatus::IDLE, now);
					workspaces[0].queue.push_back(jobs.back().id);
					workspaces[0].wipSize++;
				}
			}
			else if (targetWIP == 0) { // in the case that the bottleneck is workspaces[0]
				if (workspaces[0].anyIdle()) {
					jobs.emplace_back(static_cast<int>(jobs.size()), JobStatus::IDLE, now);
					workspaces[0].queue.push_back(jobs.back().id);
					workspaces[0].wipSize++;
				}
			}

			break;
    }
            
	// start machines with available WIP -- outside of switch statement
	for (Workspace& ws : workspaces) {
		while (ws.anyIdle() && ws.anyQueue()) {
			int queueJobId = ws.findQueue();
			// check job ID is valid before accessing
			if (queueJobId >= 0 && queueJobId < static_cast<int>(jobs.size())) {
				Event newServiceEnd = ws.startMachine(jobs[queueJobId], now);
				schedule(newServiceEnd);
			}
			else {
				break; // invalid job ID
			}
		}
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
    
	bool seenDrum = false;

    const auto& stations = config["stations"];
    for (size_t i = 0; i < stations.size(); ++i) {
        const auto& station = stations[i];
        
        int wsId = static_cast<int>(i);
        int machineNum = station["m"];
        double mean = station["mean"];
        double stdev = station["stdev"];
        bool isDrum = station["isDrum_"];
		if (isDrum && seenDrum) { // there cannot be 2 drums in the same line, check for that here
			throw std::runtime_error("Cannot be more than one drum workspace");
		}
		else if (isDrum) {
			seenDrum = true;
		}
        
        workspaces.emplace_back(wsId, mean, stdev, machineNum, isDrum);
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
        ws.queue.clear();
        ws.wipSize = 0;
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
        int queue = static_cast<int>(station.queue.size());
        int wipTotal = station.wipSize;
        ret.push_back(WorkspaceView{id, m, busy, queue, wipTotal});
    }
    return ret;
}

MetricsView Sim::getMetricsView() {
    double time = now;
    double avgTh = (time > 0) ? static_cast<double>(totalTh) / time : 0.0; // need to control for divide by 0
    int wip = 0;
    
    for (const Workspace& station : workspaces) {
        wip += station.wipSize;
    }
    
    return MetricsView{time, avgTh, wip};

}
