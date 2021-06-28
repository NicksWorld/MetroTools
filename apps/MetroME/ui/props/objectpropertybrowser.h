#pragma once

#include <QObject>
#include <QPushButton>
#include <QMap>
#include <QMetaProperty>
#include <qteditorfactory.h>
#include <qttreepropertybrowser.h>
#include <qtpropertymanager.h>
#include <qtvariantproperty.h>

class ObjectPropertyBrowser : public QtTreePropertyBrowser {
    Q_OBJECT

public:
    ObjectPropertyBrowser(QWidget* parent);
    void setActiveObject(QObject* obj); // connect to QObject using this

private:
    QtVariantPropertyManager* variantManager;
    QObject* currentlyConnectedObject = nullptr;
    QMap<QtProperty*, const char*> propertyMap;

signals:
    void objectPropertyChanged();

private slots:
    void valueChanged(QtProperty* property, const QVariant& value);

public slots:
    void objectUpdated(); // call this whenever currentlyConnectedObject is updated
};
