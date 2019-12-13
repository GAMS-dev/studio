#ifndef MIROPATHS_H
#define MIROPATHS_H

#include <QString>

// TODO (AF) tests

namespace gams {
namespace studio {

class MiroPaths
{
public:
    MiroPaths(const QString &miroPath);

    QString error() const;

    QString path();

private:
    bool exists(const QString &miro);
    QString searchLocations(const QStringList &locations);
    QStringList standardLocations();
    QString verifyPath(const QString &path, const QStringList &searched);

private:
    QString mConfiguredMiroPath;
    QString mError;

};

}
}

#endif // MIROPATHS_H
