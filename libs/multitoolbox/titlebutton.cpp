/*
 * Copyright (C) 2010 Alexei Sheplyakov
 * Distributed under the terms of the WTFPLv2 license.
 *
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 * 0. You just DO WHAT THE FUCK YOU WANT TO.
 */
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QLinearGradient>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <QtDebug>
#include "titlebutton.h"

TitleButton::TitleButton(QWidget* parent) : QToolButton(parent)
{
    m_branchOpen.load( "://images/branchopen.png" );
    m_branchClose.load( "://images/branchclose.png" );

    setCheckable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->setMinimumHeight(10);
}

void TitleButton::paintEvent(QPaintEvent* /* event */)
{
    QStylePainter p(this);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.state &= ~(QStyle::State_On | QStyle::State_Sunken);
    opt.state |= QStyle::State_Raised;

    QLinearGradient gradient;
    gradient.setStart(this->size().width()/2, 0);
    gradient.setFinalStop(this->size().width()/2, this->size().height());
    gradient.setColorAt(0.0, QColor::fromRgb(228, 228, 228));
    gradient.setColorAt(1.0, QColor::fromRgb(205, 205, 205));

    QBrush brush(gradient);

    QStyleOption branchOption;
    QRect r = opt.rect;
    static const int i = m_branchOpen.width();
    branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
    branchOption.state = QStyle::State_Children;
    if (isChecked())
        branchOption.state |= QStyle::State_Open;

    p.setBrush(brush);
    p.fillRect(this->rect(), brush);

    p.setPen(QColor::fromRgb(190, 190, 190));
    p.drawLine(this->rect().topLeft(), this->rect().topRight());
    p.drawLine(this->rect().bottomLeft(), this->rect().bottomRight());

    p.setPen(QColor::fromRgb(0, 0, 0));
    p.setFont(qApp->font());
    p.drawText(opt.rect, text(), QTextOption(Qt::AlignCenter));

    p.drawPrimitive(QStyle::PE_PanelStatusBar, opt);

    if (branchOption.state & QStyle::State_Open)
        p.drawItemPixmap(branchOption.rect, Qt::AlignCenter, m_branchOpen);
    else
        p.drawItemPixmap(branchOption.rect, Qt::AlignCenter, m_branchClose);
}

QSize TitleButton::sizeHint() const
{
    QFontMetrics metrics(qApp->font());
    QSize size = QSize(0, metrics.height() + metrics.height()/4);
    return size;
}
