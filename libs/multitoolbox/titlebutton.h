/*
 * Copyright (C) 2010 Alexei Sheplyakov
 * Distributed under the terms of the WTFPLv2 license.
 *
 *             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 * 0. You just DO WHAT THE FUCK YOU WANT TO.
 */
#ifndef TITLEBUTTON_H
#define TITLEBUTTON_H

#include <QToolButton>
class QPaintEvent;

class TitleButton : public QToolButton
{
Q_OBJECT
public:
    TitleButton(QWidget* parent = 0);

private:
    QPixmap m_branchOpen;
    QPixmap m_branchClose;

protected:
    void paintEvent(QPaintEvent* evt);
    QSize sizeHint() const;
};

#endif /* TITLEBUTTON_H */

