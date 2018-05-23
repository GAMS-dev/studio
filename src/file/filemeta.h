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
