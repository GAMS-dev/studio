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

    ///
    /// \brief Get the file path even if the file does not exists.
    /// \param path Path to the file.
    /// \return Returns the canonical path if the file exists;
    ///         otherwise the absolute path.
    ///
    static QString absolutePath(const QString &path);
};

} // namespace studio
} // namespace gams

#endif // TOOL_H
