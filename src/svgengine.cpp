#include "svgengine.h"
#include "scheme.h"
#include <QPainter>

namespace gams {
namespace studio {

SvgEngine::SvgEngine(const QString &name) : mName(name)
{
    mController = Scheme::instance();
}

SvgEngine::SvgEngine(const SvgEngine &other)
{
    mController = other.mController;
    mName = other.mName;
}

SvgEngine::~SvgEngine()
{
    if (mController)
        mController->unbind(this);
}

QString SvgEngine::iconName() const
{
    //QIconEngine::IconNameHook
    return mName;
}

void SvgEngine::unbind()
{
    mController = nullptr;
}

void SvgEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)
    QByteArray &data = mController->data(mName);
    QSvgRenderer renderer(data);
    renderer.render(painter, rect);
}

QIconEngine *SvgEngine::clone() const
{
    return new SvgEngine(*this);
}

QPixmap SvgEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QImage img(size, QImage::Format_ARGB32); // TODO(JM) test this using Format_ARGB32_Premultiplied
    img.fill(qRgba(0, 0, 0, 0));
    QPixmap res = QPixmap::fromImage(img, Qt::NoFormatConversion);
    QPainter painter(&res);
    paint(&painter, QRect(0, 0, size.width(), size.height()), mode, state);
    return res;
}

} // namespace studio
} // namespace gams
