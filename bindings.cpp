#include <emscripten/bind.h>
#include "simulation.h"

EMSCRIPTEN_BINDINGS(factory_sim) {
    emscripten::value_object<WorkspaceView>("WorkspaceView")
        .field("id", &WorkspaceView::id)
        .field("m", &WorkspaceView::m)
        .field("busy", &WorkspaceView::busy)
        .field("wip", &WorkspaceView::wip);
        
    emscripten::value_object<MetricsView>("MetricsView")
        .field("t", &MetricsView::t)
        .field("avgTh", &MetricsView::avgTh)
        .field("wip", &MetricsView::wip);

    emscripten::register_vector<WorkspaceView>("WorkspaceViewVector");
    

    emscripten::class_<Sim>("Sim")
        .constructor<>()
        .function("stepUntil", &Sim::stepUntil)
        .function("loadFromConfig", &Sim::loadFromConfigString)
        .function("getWorkspaceView", &Sim::getWorkspaceView)
        .function("getMetricsView", &Sim::getMetricsView);

}