#ifndef GAMSHIGHLIGHT_H
#define GAMSHIGHLIGHT_H

#include <QObject>

namespace gams {
namespace studio {

class GamsHighlight : public QSyntaxHighlighter
{
public:
    GamsHighlight();
};

} // namespace studio
} // namespace gams

#endif // GAMSHIGHLIGHT_H