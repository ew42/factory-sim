#ifndef STATEIO 
#define STATEIO

struct WorkspaceView {
    int id;
    int m; // num of machines
    int busy; // num of busy machines
    int queue; // jobs waiting for machines
    int wipTotal; // total WIP at station (queue + active jobs)
};

struct MetricsView {
    double t;
    double avgTh;
    int wip;
};

#endif // STATEIO