#ifndef TYPES_H
#define TYPES_H

enum class EventType {
    SERVICE_END, 
    POLICY_CHECK
};
// SERVICE_END, a machine has finished -- account for that
// POLICY_CHECK, after a fixed delay check if workspaces should load wip into machines
//               or if material should be relased

enum class JobStatus {
    IDLE, 
    ACTIVE, 
    COMPLETED
};

enum class PolicyType {
    RUN_IMMEDIATELY
};

#endif // TYPES_H

