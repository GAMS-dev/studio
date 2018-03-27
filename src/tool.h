#ifndef TOOL_H
#define TOOL_H

class QString;

namespace gams {
namespace studio {

class Tool
{
private:
    Tool() {}

public:

    // TODO(AF) rename tool and document this function
    static int findAlphaNum(const QString &text, int start, bool back);
};

} // namespace studio
} // namespace gams

#endif // TOOL_H
