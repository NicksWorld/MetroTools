#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include <QWidget>
#include <QAction>
#include "mycommon.h"

namespace Ui {
class MainToolbar;
}

class MainToolbar : public QWidget {
    Q_OBJECT

public:
    explicit MainToolbar(QWidget *parent = nullptr);
    ~MainToolbar();

    void    SetOpenArchiveHistory(const WStringArray& history);
    void    SetOpenGameFolderHistory(const WStringArray& history);

private slots:
    void    on_tbtnOpenArchive_triggered(QAction* action);
    void    on_tbtnOpenGameFolder_triggered(QAction* action);
    void    on_tbtnShowTransparency_toggled(bool checked);
    void    on_tbtnSettings_clicked();
    void    on_tbtnAbout_clicked();

signals:
    void    OnOpenArchiveTriggered(QString);
    void    OnOpenGameFolderTriggered(QString);
    void    OnShowTransparencyTriggered(bool);
    void    OnSettingsTriggered();
    void    OnAboutTriggered();

private:
    Ui::MainToolbar*    ui;
    QAction             mOpenArchiveEmptyAction;
    QAction             mOpenGameFolderEmptyAction;
};

#endif // MAINTOOLBAR_H
