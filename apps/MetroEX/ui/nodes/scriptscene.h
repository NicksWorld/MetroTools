#pragma once

#include <memory>

class MetroScript;
namespace QtNodes {
class FlowScene;
}

void                                SetNodeStyle();
std::unique_ptr<QtNodes::FlowScene> CreateScriptScene(MetroScript& script);
