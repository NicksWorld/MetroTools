/*
 * Copyright (C) 2010 Alexei Sheplyakov
 * Distributed under the terms of the WTFPLv2 license.
 *
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 * 0. You just DO WHAT THE FUCK YOU WANT TO.
 */
#include <QEvent>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QAbstractButton>
#include <QScrollArea>
#include "multitoolbox.h"
#include "titlebutton.h"
#include "ui_multitoolbox.h"

class UpdatesDisabler
{
    QPointer<QWidget> m_widget;
public:
    explicit UpdatesDisabler(QWidget* widget) : m_widget(widget)
    {
        Q_ASSERT(widget);
        m_widget->setUpdatesEnabled(false);
    }
    ~UpdatesDisabler()
    {
        if (!m_widget.isNull())
            m_widget->setUpdatesEnabled(true);
    }
};

MultiToolBox::MultiToolBox(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    ui = new Ui_MultiToolBox;
    ui->setupUi(this);
    ui->retranslateUi(this);

#ifdef Q_WS_WIN
    QPalette pal = ui->widget->palette();;
    ui->scrollArea->setPalette( pal );
#endif

    m_wrapper = ui->widget;
    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
//	m_layout->setSpacing(1);
    m_layout->setSpacing(0);
    m_layout->setSizeConstraint(QLayout::SetMinimumSize);
    m_layout->addStretch();
    m_wrapper->setLayout(m_layout);

    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, &QSignalMapper::mappedObject, this, &MultiToolBox::showHide);
}

void MultiToolBox::addWidget(QWidget* w, bool visible)
{
    UpdatesDisabler dontBother(this);
    if (!w)
        return;
    foreach (QPointer<QWidget> mW, m_widgets) {
        if (w == mW)
            return; // it's already here
    }

    TitleButton* titleButton = new TitleButton; // N.B. layout will take the ownership
    titleButton->setChecked(visible);
    titleButton->setText(w->windowTitle());
    m_signalMapper->setMapping(static_cast<QObject*>(titleButton), static_cast<QObject*>(w));

    using MapSlot = void(QSignalMapper::*)();
    QObject::connect(titleButton, &TitleButton::clicked, m_signalMapper, static_cast<MapSlot>(&QSignalMapper::map));

    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    // Make sure the button caption is updated whenever w's title changes
    w->installEventFilter(this);

    Q_ASSERT(m_layout);
    const int wIdx = m_layout->count() - 1;
    Q_ASSERT(wIdx >= 0);

    m_layout->insertWidget(wIdx, titleButton, 0, Qt::AlignTop);
    m_layout->insertWidget(wIdx + 1, w, 0, Qt::AlignTop);

    m_widgets.append(w);
    m_headers.append(titleButton);

    titleButton->setVisible(true);
    w->setVisible(visible);
}

bool MultiToolBox::eventFilter(QObject* obj, QEvent* event)
{
    Q_ASSERT(event);
    if ( (QEvent::WindowTitleChange != event->type()) &&
            (QEvent::Show != event->type()) &&
            (QEvent::Hide != event->type()) )
        return false;

    QWidget* w = qobject_cast<QWidget*>(obj);
    if (!w)
        return false;

    const int wIdx = m_widgets.indexOf(w);
    if (wIdx < 0)
        return false;

    QPointer<QAbstractButton> currentHeader = m_headers.at(wIdx);
    Q_ASSERT(!currentHeader.isNull());

    if ( QEvent::Hide == event->type() ) {
        currentHeader->setChecked(false);
        return false;
    }

    if ( QEvent::Show == event->type() ) {
        currentHeader->setChecked(true);
        return false;
    }

    if ( QEvent::WindowTitleChange == event->type() ) {
        currentHeader->setText( w->windowTitle() );
        return false;
    }

    return false;
}

void MultiToolBox::showHide(QObject* w)
{
    Q_ASSERT(w);
    static_cast<QWidget*>(w)->setVisible(!static_cast<QWidget*>(w)->isVisible());
}

QList<QPointer<QWidget> > MultiToolBox::widgets() const
{
    return m_widgets;
}

MultiToolBox::~MultiToolBox()
{
    if (ui)
        delete ui;
}


