#include <random>
#include <vector>
#include <queue>
#include <algorithm>  // Add this include at the top if not already present

using namespace std;

enum class EventType {SERVICE_END, MATERIAL_TICK, POLICY_CHECK};
// SERVICE_END, a machine has finished -- account for that
// MATERIAL_TICK, after a fixed delay check if material should be released
// POLICY_CHECK, after a fixed delay check if workspaces should load wip into machines
enum class JobStatus {IDLE, ACTIVE, COMPLETED};
enum class PolicyType {RUN_IMMEDIATELY};

struct Event {
	double time;
	EventType type;
	int workspaceId; // -1 if MATERIAL_TICK
	int machineId; // -1 if MATERIAL_TICK

    Event(double t, EventType typ, int jId, int mId) : time(t), type(typ), jobId(jId), machineId(mId) {}
};

struct Earlier {
/*
* Assigns priortiy for priority queue
* What it is asking is:
* "Should a come after b?"
* Since I want earlier times to have higher priority,
* yes, if a.time > b.time
* no, if a.time < b.time
*/
	bool operator() (const Event& a, const Event& b) {
        return a.time > b.time;
	}
}

struct NormDist {
	/* 
	 * Average
	 * Standard Deviation
	 */
	mt19937_64 rng(1);
	double mean;
	double std;
	normal_distribution<double> dist;

	NormDist(double m, double s) 
	: mean(m), std(s), dist(m, s) {}

	double sample () {
		return dist(rng);
	}
};

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

	Job (unsigned int idNum, JobStatus stat, double release)
		: id(idNum), workspace(0), status(stat), releaseTime(release) {}




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
	 * 3) WIP at Workspace
     * 4) a Distribution
     * How do I track busy?
	 */
	int id;
	vector<Machine> machines; // Machine contains jobId(int) and busy(bool)
	vector<int> wip;
    NormDist dist;


    int finishMachine(int mId) {
        // return jId
        machines[mId].busy = false;
        int jId = machines[mId].jobId;
        machines[mId].jobId = -1;
        
        // remove job from wip
        auto it = find(wip.begin(), wip.end(), jId);
        if (it != wip.end()) {
            wip.erase(it);
        }
        
        return jId;
    }    
    
    Event startMachine(Job& job, double time) { // should only be called after calling anyIdle
        job.status = JobStatus::ACTIVE;
        int machineId = findIdle()
        Machine machine = machines[machineId];
        machine.busy = true;
        machine.jobId = job.jobId;
        
        double delay = NormDist.sample();
        
        return Event(time + delay, EventType::SERVICE_END, job.workspace, machineId)

    }
    
    bool anyIdle() {
        for (const Machine& machine : Machines) {
            if (!machine.busy) {
                return true;
            }
        }
        return false;
    }
    
    int findIdle() { // returns index of an idle machine, if none, return -1
			for (size_t i = 0; i < machines.size(); i++) {
					if (!machines[i].busy) {
							return static_cast<int>(i);
					}
			}
			return -1;
    }
		
    bool anyWIP() {
        if (wip.size() > 0) {
            return true;
        }
    }
    
    int findWIP() { // only call after anyWIP
        return wip.front();
    }

};

struct Sim {
	/*
		Good thing to thing about is that Workspace, Machine, or Job
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
	double now = 0; // time var
	double delay = 0.5; // material tick every 0.5 seconds
	priority_queue<Event, vector<Event>, Earlier> timeline;
    vector<Workspace> workspaces;
    vector<Job> jobs;
    PolicyType policy = PolicyType::RUN_IMMEDIATELY;
    
    void run() {
        Event matTick(0.0, EventType::MATERIAL_TICK, -1, -1); // first material tick event

        Event polCheck(0.0, EventType::POLICY_CHECK, -1, -1) // first policy check event

        schedule(matTick);
        schedule(polCheck);

        while (timeline.size > 0) { // Event Loop, 
            e = timeline.top(); // pop doesn't return the element popped
            timeline.pop();
            now = e.time;
            handle_event(e);
        }

    }
    
    void handleEvent(const Event& e) {
        /*
         * Question of, should handleEvent update all the Job stuff or should that be dealt
         * with by the methods of the workspace?
         *
         *
        */
        switch(e.type) {
            case EventType::SERVICE_END:
							/*
							 * finishes the machine, returns the jobId
							 * check the next station if it has any idle
							 * check the current station if it has any wip
							*/
                int station = workspaces[e.workspaceId];
                int jId = station.finishMachine(e.machineId);
                
                if (station.anyWIP()) { // this is where a policy check should go
                    Event newServiceEndEvent = station.startMachine(jobs[station.findWIP()], now); // returns an Event
                    timeline.push(newEvent);
                }                
                break;
            case EventType::MATERIAL_TICK:
                Workspace firstWS = workspaces[0];
                if (firstWS.anyIdle()) { // this is where a policy check would go
										/*
										 * right now this checks if there are any idle machines at the original workspace
										 * then, releases a job to it, schedules a serviceEnd, and inserts a release tick at now + delay
										*/
                    Job newJob = Job(static_cast<unsigned int>(jobs.size()), JobStatus::IDLE, now);
                    jobs.push_back(newJob);
                    Event new = firstWS.startMachine(jobs.back(), now);
                    schedule(newReleaseEvent); 
                }
								Event newServiceEnd = (now + delay, EventType:MATERIAL_TICK, -1, -1);
								schedule(newServiceEnd);
                break;
            case EventType::POLICY_CHECK:
                for (Workspace station : workspaces) {
                    
                }
            
                break;
        }
    }
    
    void schedule(const Event& e) {
        timeline.push(e);
    }
    
    void progressJob(int jId) {
        /*
         * called when a machine has finished working on a job
         * 
         * what should it do?
         * 1) either progress the job to the next station or account for it being complete
         *  - change status, workspace and machine to account for that
         *  - need to update the workspace it moves to's wip vector
         * 2) if (policyCheck()) then load it into the next machine
         *  - change status, workspace and machine to account for that
        */
        
    }

   bool policyCheck(PolicyType pt, Workspace station) {
       /*
        *
       */
       switch (pt) {
           case PolicyType::RUN_IMMEDIATELY:
               for (Workspace station: workspaces) {
                    if(station.anyIdle() && station.anyWIP) {
                        return true;
                    }
               }
               break;
       }
   } 
    
    
};