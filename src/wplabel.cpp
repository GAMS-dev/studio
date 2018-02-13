#include "wplabel.h"
#include <QDebug>

namespace gams {
namespace studio {

WpLabel::WpLabel(QWidget *parent) : QLabel(parent)
{
}

WpLabel::WpLabel(const QString &content, const QString &link, QWidget *parent)
    : QLabel(parent), mContent(content), mLink(link)
{
    QLabel::setText(mContent);
}

void WpLabel::enterEvent(QEvent *event)
{
    setFrameShape(QFrame::Box);
}

void WpLabel::leaveEvent(QEvent *event)
{
    setFrameShape(QFrame::StyledPanel);
}

void WpLabel::mousePressEvent(QMouseEvent *event)
{
    if (!mLink.isNull())
        QLabel::linkActivated(mLink);
}

//void WpLabel::linkActivated(const QString &link)
//{
//    QLabel::linkActivated(link);
//}

}
}
