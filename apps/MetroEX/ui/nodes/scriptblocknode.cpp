#include "scriptblocknode.h"

void ScriptBlockDataModel::setMetaInfo(const std::string& name, const MetaInfo& info) {
    mName = name.c_str();
    mInput.reserve(info.input.size());
    for (auto el : info.input)
        mInput.emplace_back(el);
    mOutput.reserve(info.output.size());
    for (auto el : info.output)
        mOutput.emplace_back(el);
    mResults.resize(info.output.size());
}

void ScriptBlockDataModel::checkInputIndex(int index) {
    if (index < mInput.size())
        return;
    int curSize = mInput.size();
    int newSize = index + 1;
    mInput.resize(newSize);
    for (int i = curSize; i < newSize; i++)
        mInput[i] = QString("__in_%1__").arg(i);
}
void ScriptBlockDataModel::checkOutputIndex(int index) {
    if (index < mOutput.size())
        return;
    int curSize = mOutput.size();
    int newSize = index + 1;
    mOutput.resize(newSize);
    for (int i = curSize; i < newSize; i++)
        mOutput[i] = QString("__out_%1__").arg(i);

    mResults.resize(mOutput.size());
}

QString ScriptBlockDataModel::caption(void) const {
    return mName;
}

QString ScriptBlockDataModel::name() const {
    return mName;
}

unsigned int ScriptBlockDataModel::nPorts(QtNodes::PortType type) const {
    switch (type) {
    case QtNodes::PortType::In:
        return mInput.size();
    case QtNodes::PortType::Out:
        return mOutput.size();
    }
    return 0;
}

QString ScriptBlockDataModel::portCaption(QtNodes::PortType type, QtNodes::PortIndex index) const {
    switch (type) {
    case QtNodes::PortType::In:
        return mInput[index];
    case QtNodes::PortType::Out:
        return mOutput[index];
    }
    return QString {};
}

bool ScriptBlockDataModel::portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const {
    return true;
}

QtNodes::NodeDataType ScriptBlockDataModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const {
    return QtNodes::NodeDataType { "var", "var" };
}

void ScriptBlockDataModel::setInData(std::shared_ptr<QtNodes::NodeData>, QtNodes::PortIndex) {
}

std::shared_ptr<QtNodes::NodeData> ScriptBlockDataModel::outData(QtNodes::PortIndex index) {
    return mResults[index];
}

QWidget* ScriptBlockDataModel::embeddedWidget(void) {
    return nullptr;
}
