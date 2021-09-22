#ifndef BONESLISTROLLOUT_H
#define BONESLISTROLLOUT_H

#include <QWidget>
#include <QTreeWidgetItem>

namespace Ui {
class BonesListRollout;
}

class MetroSkeleton;

class BonesListRollout : public QWidget {
    Q_OBJECT

public:
    explicit BonesListRollout(QWidget* parent = nullptr);
    ~BonesListRollout();

    void    FillForTheSkeleton(const MetroSkeleton* skeleton);

private slots:
    void    on_bonesTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private:
    Ui::BonesListRollout *ui;
};

#endif // BONESLISTROLLOUT_H
