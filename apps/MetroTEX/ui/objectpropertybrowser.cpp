#include "objectpropertybrowser.h"
#include <QDebug>


static QString sEnumNames[] = {
    QLatin1String("Diffuse texture"),
    QLatin1String("Detail texture"),
    QLatin1String("Cubemap texture"),
    QLatin1String("Cubemap HDR texture (unused in 2033)"),
    QLatin1String("Terrain texture"),
    QLatin1String("Bump texture"),
    QLatin1String("Diffuse VA texture (mostly monsters hair)")
};


ObjectPropertyBrowser::ObjectPropertyBrowser(QWidget* parent) : QtTreePropertyBrowser(parent) {
    variantManager = new QtVariantPropertyManager(this);
    setFactoryForManager(variantManager, new QtVariantEditorFactory);
}

void ObjectPropertyBrowser::setActiveObject(QObject* obj) {
    clear();
    variantManager->clear();
    propertyMap.clear();
    if (currentlyConnectedObject) currentlyConnectedObject->disconnect(this);
    currentlyConnectedObject = obj;
    if (!obj) return;

    for (int i = 1; i < obj->metaObject()->propertyCount(); ++i) {
        QMetaProperty mp = obj->metaObject()->property(i);
        const QVariant::Type typ = (1 == i) ? static_cast<QVariant::Type>(QtVariantPropertyManager::enumTypeId()) : mp.type();
        QtVariantProperty* property = variantManager->addProperty(typ, mp.name());

        if (1 == i) {
            QStringList enumNames;
            for (size_t j = 0; j < std::size(sEnumNames); ++j) {
                enumNames.push_back(sEnumNames[j]);
            }
            property->setAttribute(QLatin1String("enumNames"), enumNames);
        }

        property->setEnabled(mp.isWritable());
        propertyMap[property] = mp.name();
        addProperty(property);
    }
    connect(obj, SIGNAL(propertyChanged()), this, SLOT(objectUpdated()));
    objectUpdated();
}

void ObjectPropertyBrowser::valueChanged(QtProperty* property, const QVariant& value) {
    currentlyConnectedObject->setProperty(propertyMap[property], value);
    objectUpdated();

    emit objectPropertyChanged();
}

void ObjectPropertyBrowser::objectUpdated() {
    disconnect(variantManager, SIGNAL(valueChanged(QtProperty*, QVariant)), this, SLOT(valueChanged(QtProperty*, QVariant)));
    QMapIterator<QtProperty*, const char*> i(propertyMap);
    while (i.hasNext()) {
        i.next();
        variantManager->setValue(i.key(), currentlyConnectedObject->property(i.value()));
    }
    connect(variantManager, SIGNAL(valueChanged(QtProperty*, QVariant)), this, SLOT(valueChanged(QtProperty*, QVariant)));
}