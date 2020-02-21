#ifndef GAMS_STUDIO_SVGENGINE_H
#define GAMS_STUDIO_SVGENGINE_H

#include <QIconEngine>
#include <QSvgRenderer>
#include <QIconEnginePlugin>

namespace gams {
namespace studio {

class Scheme;

class SvgEngine : public QIconEngine
{
public:
    SvgEngine(const QString &name);
    SvgEngine(const SvgEngine &other);
    ~SvgEngine() override;
    QString iconName() const override;
    void replaceNormalMode(QIcon::Mode mode);
    void forceSquare(bool force);
    void unbind();
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine * clone() const override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
private:
    Scheme *mController = nullptr;
    bool mForceSquare = true;
    QString mName;
    QIcon::Mode mNormalMode = QIcon::Normal;

};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_SVGENGINE_H
