#include "wplabel.h"
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

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
    Q_UNUSED(event);
    setFrameShape(QFrame::Box);
}

void WpLabel::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    setFrameShape(QFrame::StyledPanel);
}

void WpLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (!mLink.isNull()) { // programatically added, for file history
        QLabel::linkActivated(mLink);
    } else {
        QString link = this->property("link").toString(); // added via designer, only web links for now
        QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
    }
}

}
}
