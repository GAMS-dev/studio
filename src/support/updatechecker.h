#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QThread>

namespace gams {
namespace studio {
namespace support {

class UpdateChecker : public QThread
{
    Q_OBJECT

public:
    UpdateChecker(QObject *parent = nullptr);

    void run() override;

signals:
    void messageAvailable(const QString &message);
};

}
}
}

#endif // UPDATECHECKER_H
