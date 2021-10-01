#ifndef FACEFXROLLOUT_H
#define FACEFXROLLOUT_H

#include <QWidget>

#include "mycommon.h"

namespace Ui {
class FaceFXRollout;
}

class MetroSkeleton;

class FaceFXRollout : public QWidget {
    Q_OBJECT

public:
    explicit FaceFXRollout(QWidget* parent = nullptr);
    ~FaceFXRollout();

    void    FillForTheSkeleton(MetroSkeleton* skeleton);

private slots:
    void    on_txtFaceFX_textEdited(const QString& text);

private:
    Ui::FaceFXRollout*  ui;
    MetroSkeleton*      mSkeleton;
};

#endif // FACEFXROLLOUT_H
