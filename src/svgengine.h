#ifndef GAMS_STUDIO_SVGENGINE_H
#define GAMS_STUDIO_SVGENGINE_H

#include <QIconEngine>
#include <QSvgRenderer>
#include <QIconEnginePlugin>

namespace gams {
namespace studio {

class Theme;

class SvgEngine : public QIconEngine
{
public:
    SvgEngine(const QString &name);
    SvgEngine(const QString &name, const QString &disabledName);
    SvgEngine(const SvgEngine &other);
    ~SvgEngine() override;
    void setScope(int scope);
    QString iconName() const override;
    void replaceNormalMode(QIcon::Mode mode);
    void forceSquare(bool force);
    void unbind();
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine * clone() const override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
private:
    Theme *mController = nullptr;
    bool mForceSquare = true;
    int mScope = 0;
    QString mName;
    QString mNameD;
    QIcon::Mode mNormalMode = QIcon::Normal;

};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_SVGENGINE_H
