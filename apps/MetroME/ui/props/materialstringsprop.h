#pragma once

#include <QObject>

class MaterialStringsProp : public QObject {
public:
    explicit MaterialStringsProp(QObject* parent = nullptr) : QObject(parent) {}

private:
    Q_OBJECT
    Q_PROPERTY(QString Texture MEMBER texture NOTIFY propertyChanged)
    Q_PROPERTY(QString Shader MEMBER shader NOTIFY propertyChanged)
    Q_PROPERTY(QString Material MEMBER material NOTIFY propertyChanged)
    Q_PROPERTY(QString SrcMaterial MEMBER src_mat NOTIFY propertyChanged)

signals:
    void propertyChanged();

public:
    QString     texture;
    QString     shader;
    QString     material;
    QString     src_mat;
};

