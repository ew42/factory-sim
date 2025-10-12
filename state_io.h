#ifndef STATEIO 
#define STATEIO

struct WorkspaceView {
    int id;
    int m; // num of machines
    int busy; // num of busy machines
    int wip; // num of wip
};

struct MetricsView {
    double t;
    double avgTh;
    int wip;
};

#endif // STATEIO