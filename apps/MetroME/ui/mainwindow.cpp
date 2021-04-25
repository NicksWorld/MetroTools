#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "simpleribbon/SimpleRibbon.h"
#include "simpleribbon/SimpleRibbonTab.h"
#include "simpleribbon/SimpleRibbonGroup.h"
#include "simpleribbon/SimpleRibbonVBar.h"
#include "simpleribbon/SimpleRibbonButton.h"

#include <QToolButton>
#include <QMenu>
#include <QCheckBox>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SimpleRibbonTab* tabModel = ui->ribbon->AddRibbonTab(tr("Model"));
    SimpleRibbonTab* tabSkeleton = ui->ribbon->AddRibbonTab(tr("Skeleton"));
    SimpleRibbonTab* tabAnimation = ui->ribbon->AddRibbonTab(tr("Animation"));
    SimpleRibbonTab* tab3DView = ui->ribbon->AddRibbonTab(tr("3D View"));

    SimpleRibbonGroup* groupFile = tabModel->AddRibbonGroup(tr("File"));
    SimpleRibbonGroup* groupShit = tabModel->AddRibbonGroup(tr("Shit"));

    SimpleRibbonButton* importModelButton = new SimpleRibbonButton;
    importModelButton->SetText(tr("Import..."));
    importModelButton->SetTooltip(tr("Import model from file"));
    importModelButton->SetIcon(QIcon(":/imgs/ImportFile.png"));
    {
        QMenu* menu = new QMenu("");
        menu->addAction("Import from Metro model...");
        menu->addSeparator();
        menu->addAction("Import from OBJ model...");
        importModelButton->SetMenu(menu);
    }
    groupFile->AddWidget(importModelButton);

    // Add 'Export' button
    SimpleRibbonButton* exportModelButton = new SimpleRibbonButton;
    exportModelButton->SetText(tr("Export..."));
    exportModelButton->SetTooltip(tr("Export model to file"));
    exportModelButton->SetIcon(QIcon(":/imgs/ExportFile.png"));
    {
        QMenu* menu = new QMenu("");
        menu->addAction("Export to Metro model...");
        menu->addSeparator();
        menu->addAction("Export to OBJ model...");
        exportModelButton->SetMenu(menu);
    }
    groupFile->AddWidget(exportModelButton);

    SimpleRibbonVBar* vbar = new SimpleRibbonVBar();
    QCheckBox* chk1 = new QCheckBox();
    chk1->setText(tr("Show bounds"));
    QComboBox* combo1 = new QComboBox();
    combo1->addItem("Bounding Box");
    combo1->addItem("Bounding Sphere");
    combo1->setEditable(false);
    QCheckBox* chk2 = new QCheckBox();
    chk2->setText(tr("Submodels bounds"));
    vbar->AddWidget(chk1);
    vbar->AddWidget(combo1);
    vbar->AddWidget(chk2);
    groupShit->AddWidget(vbar);
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::onRibbonSizeChanged(int newHeight) {
    this->setContentsMargins(0, newHeight, 0, 0);
}

