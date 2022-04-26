#ifndef GAMS_STUDIO_HTMLCONVERTER_H
#define GAMS_STUDIO_HTMLCONVERTER_H

#include <QByteArray>
#include <QTextCursor>

namespace gams {
namespace studio {

class HtmlConverter
{
public:
    static QByteArray toHtml(QTextCursor cursor, QColor background = Qt::white);
    static QByteArray toHtml(QTextDocument *doc, QColor background = Qt::white);

private:
    HtmlConverter();
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_HTMLCONVERTER_H
