#ifndef FILEMETA_H
#define FILEMETA_H

#include <QObject>
#include <QDateTime>
#include <QTextDocument>
#include "syntax.h"

namespace gams {
namespace studio {

class FileMeta: QObject
{
    Q_OBJECT
public:
    FileMeta(QString location);
    QString location() const;
    FileKind kind();
    QString name();
    bool isModified() const;
    QTextDocument* document() const;

private:
    struct Data {
        Data(QString location);
        bool exist = false;
        qint64 size = 0;
        QDateTime created;
        QDateTime modified;
        FileType *type = nullptr;
    };

private:
    QString mLocation;
    QString mName;
    Data mData;
    QTextDocument* mDocument = nullptr;

    // TODO(JM): QTextBlock.userData  ->  TextMark
    // TODO(JM): TextChanged events
    // TODO(JM): FileChanged events
    // TODO(JM): Autosave
    // TODO(JM): Data-Reference ( QTextDocument / GDX / LST+LXI / ... )
    // TODO(JM): FileState (opened, closed, changed, removed, ...)
    // TODO(JM): FileType info

};

} // namespace studio
} // namespace gams

#endif // FILEMETA_H
