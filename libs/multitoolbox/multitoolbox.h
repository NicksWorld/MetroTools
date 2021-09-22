/*
 * Copyright (C) 2010 Alexei Sheplyakov
 * Distributed under the terms of the WTFPLv2 license.
 *
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 * 0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#ifndef MULTITOOLBOX_H
#define MULTITOOLBOX_H

#include <QWidget>
#include <QPointer>

class Ui_MultiToolBox;
class QAbstractButton;
class QEvent;
class QSignalMapper;
class QVBoxLayout;

class MultiToolBox : public QWidget
{
Q_OBJECT
public:
    MultiToolBox(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~MultiToolBox();
    // Add a widget to the toolbox. Takes the ownership of the widget.
    void addWidget(QWidget* w, bool visible = true);
    QList<QPointer<QWidget> > widgets() const;
protected:
    // Updates the button caption whenever the corresponding widget's
    // title changes. Also updates the indicator style when the widget
    // gets hidden/shown in some way (except clicking the title button).
    bool eventFilter(QObject* obj, QEvent* event);
private slots:
    void showHide(QWidget* w);
private:
    QList<QPointer<QWidget> >		m_widgets;
    QList<QPointer<QAbstractButton> >	m_headers;
    QSignalMapper*				m_signalMapper;
    QWidget*				m_wrapper;
    QVBoxLayout*				m_layout;
    Ui_MultiToolBox*			ui;
};

#endif /* MULTITOOLBOX_H */

