#ifndef AUTOSAVEHANDLER_H
#define AUTOSAVEHANDLER_H

#include <QStringList>

namespace gams {
namespace studio {

class MainWindow;

class AutosaveHandler
{
public:
    AutosaveHandler(MainWindow *mainWindow);

    QStringList checkForAutosaveFiles();

    void recoverAutosaveFiles(const QStringList &autosaveFiles);

private:
    MainWindow *mMainWindow;
    const QString mAutosavedFileMarker = "~$";
};

} // namespace studio
} // namespace gams

#endif // AUTOSAVEHANDLER_H
