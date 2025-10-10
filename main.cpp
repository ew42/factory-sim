#include "simulation.h"
#include <iostream>

int main() {
    try {
        Sim s = Sim();
        s.loadFromConfig("config.json");
        
        std::cout << "Successfully loaded " << s.workspaces.size() << " workspaces from config.json" << std::endl;
        
        for (size_t i = 0; i < s.workspaces.size(); i++) {
            const auto& ws = s.workspaces[i];
            std::cout << "Workspace " << ws.id << ": " << ws.machines.size() 
                      << " machines, mean=" << ws.dist.mean << ", stdev=" << ws.dist.std << std::endl;
        }
        
        s.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}