# Factory Simulator

## Plan
After reading "The Goal" by Eliyahu Goldratt, I want visualize and simulate the process of a factory, then implement lessons from Theory of Constraints(TOC).

## Structure
factory-sim/
    main.cpp -- Loads config, creates and runs sim
    simulator.cpp -- Event loop and time live here
    policy.cpp -- different policies live here, gets called when release_tick
    model.cpp -- all the structs(event, job, station, ...) live here