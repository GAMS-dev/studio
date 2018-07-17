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

    QStringList checkForAutosaveFiles(QStringList list);

    void recoverAutosaveFiles(const QStringList &autosaveFiles);

    void saveChangedFiles();

    void clearAutosaveFiles(const QStringList &openTabs);

private:
    MainWindow *mMainWindow;
    const QString mAutosavedFileMarker = "~$";
};

} // namespace studio
} // namespace gams

#endif // AUTOSAVEHANDLER_H
