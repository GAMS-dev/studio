#ifndef TOOL_H
#define TOOL_H

#include <QString>

namespace gams {
namespace studio {

class Tool
{
private:
    Tool() {}

public:
    static int findAlphaNum(const QString &text, int start, bool back);
};

} // namespace studio
} // namespace gams

#endif // TOOL_H
