#include <random>
#include <vector>
#include <queue>

using namespace std;

enum class EventType {ARRIVAL, JOB_END, MATERIAL_RELEASE};
enum class JobStatus {IDLE, ACTIVE, TRAVELING, COMPLETED};
enum class MachineStatus {IDLE, ACTIVE};

struct Event {
	unsigned int jobId;
	unsigned int machineId;
	double time;
	EventType type;
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
	unsigned int id;
	unsigned int workspace; // if job completed workspace = -1(aka 4294967295), if not released = 0
	JobStatus status;
	double releaseTime;

	Job (unsigned int idNum, JobStatus stat, double release)
		: id(idNum), workspace(1), status(stat), releaseTime(release) {}

	void arrival() { // workspace - 1 ----job----> workspace
		status = JobStatus::IDLE;
	}

	void stageFinished() {
		workspace++;
		status = JobStatus::Traveling;
	}

	void jobCompleted() {
		workspace = -1; //aka 4294967295
	}



};

struct Machine {
	/*
	 * Data I want:
	 * 1) Workspace
	 * 2) Distribution (mean, std dev)
	 * 3) Status
	 * 		ex: idle, active
	 * 4) id?
	 *
	 * Methods I want:
	 * startJob
	 * changeStatus
	 * finishJob?
	 */
	unsigned int workspace;
	NormDist dist;
	MachineStatus status;
	// unsigned int id;
	Machine(unsigned int idNum, NormDist distribution)
		: workspace(1), dist(distribution), status(MachineStatus::IDLE) {}
	
	void startJob() {
		/*
		* Machine needs to change status to active
		* 
		*/
		status = MachineStatus::ACTIVE;
	}

	void finishJob() {
		status = MachineStatus::IDLE;
	}
};

struct Workspace {
	/*
	 * Data I want:
	 * 1) id (in order, i.e. 1 -> 2 -> -> 3)
	 * 2) Vec of machines
	 * 3) WIP at Workspace
	 */
	unsigned int id;
	vector<Machine> machines;
	unsigned int WIP;

};

struct Sim {
	/*
	* Need to figure out the event loop

	*/
	double now = 0; // time var
	priority_queue<Event, vector<Event>, Earlier> timeline;
};