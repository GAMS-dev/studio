#ifndef TOOL_H
#define TOOL_H

#include <QtCore>

namespace gams {
namespace studio {

class Tool
{
    Tool() {}
public:
    static int findAlphaNum(QString text, int start, bool back);
};

} // namespace studio
} // namespace gams

#endif // TOOL_H
