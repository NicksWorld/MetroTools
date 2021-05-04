#pragma once

#include <metro/scripts/MetroBlockMeta.h>
#include <nodes/NodeDataModel>
#include <string>

class ScriptBlockDataModel : public QtNodes::NodeDataModel {
    Q_OBJECT

public:
    QString                            caption(void) const override;
    QString                            name() const override;
    unsigned int                       nPorts(QtNodes::PortType) const override;
    QString                            portCaption(QtNodes::PortType, QtNodes::PortIndex) const override;
    bool                               portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override;
    QtNodes::NodeDataType              dataType(QtNodes::PortType, QtNodes::PortIndex) const override;
    void                               setInData(std::shared_ptr<QtNodes::NodeData>, QtNodes::PortIndex) override;
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex) override;
    QWidget*                           embeddedWidget(void) override;

    void setMetaInfo(const std::string& name, const MetaInfo& info);
    void checkInputIndex(int index);
    void checkOutputIndex(int index);

private:
    QString                                         mName;
    std::vector<QString>                            mInput;
    std::vector<QString>                            mOutput;
    std::vector<std::shared_ptr<QtNodes::NodeData>> mResults;
};
