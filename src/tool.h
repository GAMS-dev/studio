#ifndef TOOL_H
#define TOOL_H

#include <QString>

namespace gams {
namespace studio {

class Version
{// TODO keep this?
private:
    Version();

public:
    ///
    /// \brief Converts the STUDIO_VERSION into an number.
    /// \return The STUDIO_VERSION as number.
    /// \remark Used to check for updates.
    ///
    static int versionToNumber();

    ///
    /// \brief Get GAMS Distribution version number.
    /// \param version Version string buffer.
    /// \param length Length of the version string buffer.
    /// \return The GAMS Distribution version number as string. The
    ///         same as the <c>version</c> argument.
    ///
    static char* distribVersion(char* version, size_t length=16);
};

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
