#pragma once

#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QColor>

class MetroTextureInfoData : public QObject
{
public:
    explicit MetroTextureInfoData(QObject* parent = nullptr) : QObject(parent) {}

private:
    Q_OBJECT
    Q_PROPERTY(int Texture_type MEMBER type NOTIFY propertyChanged)
    Q_PROPERTY(bool IsAnimated MEMBER animated NOTIFY propertyChanged)
    Q_PROPERTY(int Format MEMBER fmt NOTIFY propertyChanged)
    Q_PROPERTY(int Width MEMBER width NOTIFY propertyChanged)
    Q_PROPERTY(int Height MEMBER height NOTIFY propertyChanged)
    Q_PROPERTY(QString Name MEMBER name NOTIFY propertyChanged)
    Q_PROPERTY(QString BumpName MEMBER bump_name NOTIFY propertyChanged)
    Q_PROPERTY(double BumpHeight MEMBER bump_height NOTIFY propertyChanged)
    Q_PROPERTY(int ParrHeight MEMBER parr_height NOTIFY propertyChanged)
    Q_PROPERTY(QString DetailName MEMBER det_name NOTIFY propertyChanged)
    Q_PROPERTY(double DetailHScale MEMBER det_u_scale NOTIFY propertyChanged)
    Q_PROPERTY(double DetailVScale MEMBER det_v_scale NOTIFY propertyChanged)
    Q_PROPERTY(double DetailInt MEMBER det_int NOTIFY propertyChanged)
    Q_PROPERTY(bool MipsEnabled MEMBER mip_enabled NOTIFY propertyChanged)
    Q_PROPERTY(bool IsStreamable MEMBER streamable NOTIFY propertyChanged)
    Q_PROPERTY(bool HighPriority MEMBER priority NOTIFY propertyChanged)
    Q_PROPERTY(QColor AverageColour MEMBER avg_color NOTIFY propertyChanged)

signals:
    void propertyChanged();

public:
    int         type;   // enum TextureType
    bool        animated;
    int         fmt;
    int         width;
    int         height;
    QString     name;
    QString     bump_name;
    double      bump_height;
    int         parr_height;
    QString     det_name;
    double      det_u_scale;
    double      det_v_scale;
    double      det_int;
    bool        mip_enabled;
    bool        streamable;
    bool        priority;
    QColor      avg_color;
};
