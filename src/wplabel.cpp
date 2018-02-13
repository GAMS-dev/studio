#include "wplabel.h"
#include <QDebug>

namespace gams {
namespace studio {

WpLabel::WpLabel(const QString &content)
    : mContent(content)
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

}

//void WpLabel::linkActivated(const QString &link)
//{
//    QLabel::linkActivated(link);
//}

}
}
