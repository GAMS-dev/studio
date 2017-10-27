#ifndef FILEMETRICS_H
#define FILEMETRICS_H

#include <QtCore>
#include "filetype.h"

namespace gams {
namespace studio {

///
/// The FileMetrics class stores current metrics of a file
///
class FileMetrics
{
    bool mExists;
    qint64 mSize;
    QDateTime mCreated;
    QDateTime mModified;
    FileType *mType;

public:
    enum ChangeKind {ckSkip, ckUnchanged, /* ckRenamed, */ ckNotFound, ckModified};

    FileMetrics();
    explicit FileMetrics(QFileInfo fileInfo);
    FileMetrics(const FileMetrics &other);
    FileMetrics &operator=(const FileMetrics& other);

    const FileType& fileType() const;

    ChangeKind check(QFileInfo fileInfo);

};

} // namespace studio
} // namespace gams

#endif // FILEMETRICS_H
