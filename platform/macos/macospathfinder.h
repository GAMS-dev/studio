#ifndef MACOSPATHFINDER_H
#define MACOSPATHFINDER_H

#include <QString>

class MacOSPathFinder
{
private:
    MacOSPathFinder();

public:
    static QString systemDir();
    static QString bundlePath();

private:
    static QString searchApplications();

private:
    static const QString SubPath;
};

#endif // MACOSPATHFINDER_H
