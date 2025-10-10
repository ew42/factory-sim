#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

struct Event {
    double time;
    EventType type;
    int workspaceId; // -1 if POLICY_CHECK
    int machineId; // -1 if POLICY_CHECK

    Event(double t, EventType typ, int wId, int mId)
     : time(t), type(typ), workspaceId(wId), machineId(mId) {}
};

struct Earlier {
/*
* Assigns priority for priority queue
* What it is asking is:
* "Should a come after b?"
* Since I want earlier times to have higher priority,
* yes, if a.time > b.time
* no, if a.time < b.time
*/
    bool operator() (const Event& a, const Event& b) {
        return a.time > b.time;
    }
};

#endif // EVENTS_H

