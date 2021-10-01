#ifndef MOTIONSROLLOUT_H
#define MOTIONSROLLOUT_H

#include <QWidget>

#include "mycommon.h"

namespace Ui {
class MotionsRollout;
}

class MetroSkeleton;

class MotionsRollout : public QWidget {
    Q_OBJECT

public:
    explicit MotionsRollout(QWidget* parent = nullptr);
    ~MotionsRollout();

    void    FillForTheSkeleton(MetroSkeleton* skeleton);

private slots:
    void    on_textMotions_textChanged();

private:
    Ui::MotionsRollout* ui;
    MetroSkeleton*      mSkeleton;
};

#endif // MOTIONSROLLOUT_H
