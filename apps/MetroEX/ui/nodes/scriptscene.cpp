#include "scriptscene.h"
#include "scriptblocknode.h"
#include <metro/scripts/MetroScript.h>
#include <nodes/ConnectionStyle>
#include <nodes/DataModelRegistry>
#include <nodes/FlowScene>
#include <nodes/FlowViewStyle>
#include <nodes/Node>
#include <nodes/NodeStyle>

std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels();

std::unique_ptr<QtNodes::FlowScene> CreateScriptScene(MetroScript& script) {
    std::vector<std::unique_ptr<ScriptBlockDataModel>> models;
    models.reserve(script.blocks.blocks.size());
    for (const auto& block : script.blocks.blocks) {
        auto dataModel = new ScriptBlockDataModel();
        dataModel->setMetaInfo(block.name, *block.meta);
        models.emplace_back(dataModel);
    }
    for (const auto& link : script.links) {
        models[link.z]->checkInputIndex(link.w);
        models[link.x]->checkOutputIndex(link.y);
    }

    auto                        result = std::make_unique<QtNodes::FlowScene>(registerDataModels());
    std::vector<QtNodes::Node*> temp;
    temp.reserve(script.blocks.blocks.size());
    size_t i = 0;
    for (const auto& block : script.blocks.blocks) {
        auto& node = result->createNode(std::move(models[i]));
        node.nodeGraphicsObject().setPos(QPointF { (qreal)block.posx, (qreal)block.posy });
        temp.push_back(&node);
        i++;
    }

    for (const auto& link : script.links) {
        result->createConnection(*temp[link.z], link.w, *temp[link.x], link.y);
    }
    return result;
}

void SetNodeStyle() {
    QtNodes::FlowViewStyle::setStyle(
        R"(
  {
    "FlowViewStyle": {
      "BackgroundColor": [255, 255, 240],
      "FineGridColor": [245, 245, 230],
      "CoarseGridColor": [235, 235, 220]
    }
  }
  )");

    QtNodes::NodeStyle::setNodeStyle(
        R"(
  {
    "NodeStyle": {
      "NormalBoundaryColor": "darkgray",
      "SelectedBoundaryColor": "deepskyblue",
      "GradientColor0": "mintcream",
      "GradientColor1": "mintcream",
      "GradientColor2": "mintcream",
      "GradientColor3": "mintcream",
      "ShadowColor": [200, 200, 200],
      "FontColor": [10, 10, 10],
      "FontColorFaded": [100, 100, 100],
      "ConnectionPointColor": "white",
      "PenWidth": 2.0,
      "HoveredPenWidth": 2.5,
      "ConnectionPointDiameter": 10.0,
      "Opacity": 1.0
    }
  }
  )");

    QtNodes::ConnectionStyle::setConnectionStyle(
        R"(
  {
    "ConnectionStyle": {
      "ConstructionColor": "gray",
      "NormalColor": "black",
      "SelectedColor": "gray",
      "SelectedHaloColor": "deepskyblue",
      "HoveredColor": "deepskyblue",

      "LineWidth": 3.0,
      "ConstructionLineWidth": 2.0,
      "PointDiameter": 10.0,

      "UseDataDefinedColors": false
    }
  }
  )");
}

std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels() {
    auto ret = std::make_shared<QtNodes::DataModelRegistry>();
    ret->registerModel<ScriptBlockDataModel>("block");
    return ret;
}
