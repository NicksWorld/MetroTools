#pragma once
#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui { class MdlInfoPanel; }
QT_END_NAMESPACE

class ModelInfoPanel : public QFrame {
    Q_OBJECT

public:
    ModelInfoPanel(QWidget* parent = nullptr);
    ~ModelInfoPanel();

    void    SetModelTypeText(const QString& text);
    void    SetNumVerticesText(const QString& text);
    void    SetNumTrianglesText(const QString& text);
    void    SetNumJointsText(const QString& text);
    void    SetNumAnimationsText(const QString& text);
    void    SetPlayStopButtonText(const QString& text);

    void    ClearMotionsList();
    void    AddMotionToList(const QString& name);

    void    ClearLodsList();
    void    AddLodIdxToList(const int lodIdx);
    void    SelectLod(const int lodIdx);

signals:
    void    playStopClicked();
    void    modelInfoClicked();
    void    exportMotionClicked();
    void    motionsListSelectionChanged(int idx);
    void    lodsListSelectionChanged(int idx);

private slots:
    void    on_btnMdlPropPlayStopAnim_clicked(bool checked);
    void    on_btnModelInfo_clicked(bool checked);
    void    on_btnModelExportMotion_clicked(bool checked);
    void    on_lstMdlPropMotions_itemSelectionChanged();
    void    on_lstLods_currentIndexChanged(int index);

private:
    Ui::MdlInfoPanel*   ui;
};

