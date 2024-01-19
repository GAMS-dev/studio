#ifndef GAMS_STUDIO_TEXTFILESAVER_H
#define GAMS_STUDIO_TEXTFILESAVER_H

#include <QObject>
#include <QFile>

namespace gams {
namespace studio {

class TextFileSaver : public QObject
{
    Q_OBJECT
public:
    explicit TextFileSaver(QObject *parent = nullptr);
    ~TextFileSaver();
    bool open(const QString &filename, const QString &tempMarker = QString(".~tmp"));
    qint64 write(const QByteArray &content);
    qint64 write(const char *content, qint64 len);
    qint64 write(const char* content);
    bool close();

private:
    QFile mTempFile;
    QString mFileName;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_TEXTFILESAVER_H
