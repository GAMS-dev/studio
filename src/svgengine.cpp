#include "svgengine.h"
#include "theme.h"
#include "logger.h"
#include <QPainter>
#include <QStyleOption>
#include <QGuiApplication>
#include <QApplication>

namespace gams {
namespace studio {

SvgEngine::SvgEngine(const QString &name)
    : QIconEngine(), mName(name), mNameD(name)
{
    mController = Theme::instance();
}

SvgEngine::SvgEngine(const QString &name, const QString &disabledName)
    : QIconEngine(), mName(name), mNameD(disabledName)
{
    mController = Theme::instance();
}

SvgEngine::SvgEngine(const SvgEngine &other) : QIconEngine()
{
    mController = other.mController;
    mForceSquare = other.mForceSquare;
    mScope = other.mScope;
    mName = other.mName;
    mNameD = other.mNameD;
    mNormalMode = other.mNormalMode;
}

SvgEngine::~SvgEngine()
{
    if (mController)
        mController->unbind(this);
}

void SvgEngine::setScope(int scope)
{
    if (mController->isValidScope(scope))
        mScope = scope;
}

QString SvgEngine::iconName() const
{
    //QIconEngine::IconNameHook
    return mName;
}

void SvgEngine::replaceNormalMode(QIcon::Mode mode)
{
    mNormalMode = mode;
}

void SvgEngine::forceSquare(bool force)
{
    mForceSquare = force;
}

void SvgEngine::unbind()
{
    mController = nullptr;
}

void SvgEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)
    if (mode == QIcon::Normal) mode = mNormalMode;
    const QString &name = (mode == QIcon::Disabled ? mNameD : mName);
    QByteArray &data = mController->data(name, Theme::Scope(mScope), mode);
    QSvgRenderer renderer(data);
    QRect pRect = rect;
    if (mForceSquare) pRect.setWidth(pRect.height());
    renderer.render(painter, pRect);
}

QIconEngine *SvgEngine::clone() const
{
    return new SvgEngine(*this);
}

QPixmap SvgEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter painter(&img);
    paint(&painter, QRect(0, 0, size.width(), size.height()), mode, state);
    painter.end();
    QPixmap res = QPixmap::fromImage(img, Qt::NoFormatConversion);
//    if (mode == QIcon::Disabled) {
//        QStyleOption opt(0);
//        opt.palette = QGuiApplication::palette();
//        return QApplication::style()->generatedIconPixmap(mode, res, &opt);
//    }
    return  res;
}

} // namespace studio
} // namespace gams
