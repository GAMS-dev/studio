#ifndef FILEMETA_H
#define FILEMETA_H

#include <QObject>

namespace gams {
namespace studio {

class FileMeta: QObject
{
    Q_OBJECT
public:
    FileMeta(QString location);
    QString location() const;

private:
    QString mLocation;


};

} // namespace studio
} // namespace gams

#endif // FILEMETA_H
