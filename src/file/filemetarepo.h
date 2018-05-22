#ifndef FILEMETAREPO_H
#define FILEMETAREPO_H

#include <QObject>

namespace gams {
namespace studio {

class FileMetaRepo : public QObject
{
    Q_OBJECT
public:
    FileMetaRepo(QObject* parent);
};

} // namespace studio
} // namespace gams

#endif // FILEMETAREPO_H
