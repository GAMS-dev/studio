#include "updatechecker.h"
#include "checkforupdatewrapper.h"

namespace gams {
namespace studio {
namespace support {

UpdateChecker::UpdateChecker(QObject *parent)
    : QThread(parent)
{

}

void UpdateChecker::run()
{
    QString message;
    CheckForUpdateWrapper c4uWrapper;
    if (c4uWrapper.isValid()) {
        message = c4uWrapper.checkForUpdate();
    } else {
        message = c4uWrapper.message();
    }
    emit messageAvailable(message);
}

}
}
}
