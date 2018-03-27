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
    static QString absolutePath(QString path);
    static QString getPath(const QString &file);
    // TODO(AF) rename tool and document this function
    static int findAlphaNum(const QString &text, int start, bool back);
  
};

} // namespace studio
} // namespace gams

#endif // TOOL_H
