#ifndef CHECKFORUPDATEWRAPPER_H
#define CHECKFORUPDATEWRAPPER_H

#include "c4umcc.h"

#include <QStringList>

namespace gams {
namespace studio {

class CheckForUpdateWrapper
{// TODO html?
public:
    CheckForUpdateWrapper();
    ~CheckForUpdateWrapper();

    bool isValid() const;
    QString message() const;

    QString checkForUpdate();

private:
    void getMessages(int &messageIndex, char *buffer);

private:
    bool mValid = true;
    c4uHandle_t mC4UHandle;
    QStringList mMessages;
};

}
}

#endif // CHECKFORUPDATEWRAPPER_H
