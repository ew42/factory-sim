# Factory Simulator

Check out the live [demo](https://ew42.github.io/factory-sim/)!

## Plan
After reading "The Goal" by Eliyahu Goldratt, I wanted to visualize and simulate factory operations, so I could implement lessons and experiment with concepts/policies from Theory of Constraints(TOC).

## Structure

**Key Architecture Notes**
    - **Discrete Event Simulation**: Uses a priority queue (timeline) to process events chronologically
    - **Theory of Constraints Focus**: Designed to simulate and visualize factory constraints and bottlenecks
    - **Dual Interface**: Both native C++ executable and web-based visualization via WebAssembly
    - **Policy-Driven**: Simulation behavior controlled by a configurable policies
    - **Metrics Collection**: Tracks throughput, WIP, and workspace utilization for TOC

### Core Simulation Engine

factory-sim/
    main.cpp -- Loads config(.json file) and runs simulation
    simulation.h/.cpp -- Main simulation engine with discrete event loop
    factory.h/.cpp -- Core data structures (Job, Machine, Workspace)
    events.h/.cpp -- Event system for scheduling SERVICE_END and POLICY_CHECK events
    types.h -- Core enums (EventType, JobStatus, PolicyType)
    utils.h/.cpp -- Utility functions (currently only normal distribution sampling)
    state_io.h -- Data structures for UI state (WorkspaceView, MetricsView)

### Configuration and Data

    config.json -- Simulation parameters (station, machines, distribution)
    
### Web Interface (WebAssembly)

    bindings.cpp -- Emscripten binmdings to expose C++ structs to JavaScript
    docs/
        index.html -- Web-based visualization interface
        factory-sim.js -- WASM JavaScript code (for web)
        factory-sim.wasm -- WASM binary (for web)
        
## Compilation

### Prereqs
- C++17 compatible compiler
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) (for WASM builds)
- `json.hpp` header (included in project)

### Native Build
```bash

g++ -std=c+17 *.cpp -o factory-sim
```

### WebAssembly Build
```bash
# Make sure Emscripten is active
source /path/to/emsdk/emsdk_env.sh

# Basic WebAssembly build
em++ -std=c++17 --bind -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME="'Module'" simulation.cpp factory.cpp events.cpp utils.cpp bindings.cpp -o docs/factory-sim.js
```

### Running
- **Native**: `./factory-sim` (uses `config.json`)
- **Web**: Open `docs/index.html` in a web browser
- **Config**: Edit `config.json` to modify simulation parameters (stations, machines, distribution)