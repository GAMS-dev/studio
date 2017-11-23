#include "studiosettings.h"

namespace gams {
namespace studio {


StudioSettings::StudioSettings(MainWindow *main)
    : mMain(main)
{
}

void StudioSettings::saveSettings()
{
    if (mAppSettings == nullptr) {
        qDebug() << "ERROR: settings file missing.";
        return;
    }
    // Main Application Settings
    qDebug() << "Saving settings";
    // window
    mAppSettings->beginGroup("mainWindow");
    mAppSettings->setValue("size", mMain->size());
    mAppSettings->setValue("pos", mMain->pos());
    mAppSettings->setValue("windowState", mMain->saveState());

    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    mAppSettings->setValue("projectView", mMain->projectViewVisibility());
    mAppSettings->setValue("outputView", mMain->outputViewVisibility());

    mAppSettings->endGroup();

    // history
    mAppSettings->beginGroup("fileHistory");
    mAppSettings->beginWriteArray("lastOpenedFiles");
    for (int i = 0; i < mMain->history()->lastOpenedFiles.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("file", mMain->history()->lastOpenedFiles.at(i));
    }
    mAppSettings->endArray();

    QMap<QString, QStringList> map(mMain->commandLineModel()->allOptions());
    mAppSettings->beginWriteArray("commandLineOptions");
    for (int i = 0; i < map.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("path", map.keys().at(i));
        mAppSettings->setValue("opt", map.values().at(i));
    }
    mAppSettings->endArray();

    mAppSettings->beginWriteArray("openedTabs");
    QList<QPlainTextEdit*> editList = mMain->fileRepository()->editors();
    for (int i = 0; i < editList.size(); i++) {
        mAppSettings->setArrayIndex(i);
        FileContext *fc = mMain->fileRepository()->fileContext(editList.at(i));
        mAppSettings->setValue("location", fc->location());
    }

    mAppSettings->endArray();
    mAppSettings->endGroup();

    // User Settings
    mUserSettings->beginGroup("General");
    // todo
    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");
    // todo
    mUserSettings->endGroup();

    mAppSettings->sync();
}

void StudioSettings::loadSettings()
{
    if (mAppSettings == nullptr)
        mAppSettings = new QSettings("GAMS", "Studio");

    qDebug() << "loading settings from" << mAppSettings->fileName();

    // window
    mAppSettings->beginGroup("mainWindow");
    mMain->resize(mAppSettings->value("size", QSize(1024, 768)).toSize());
    mMain->move(mAppSettings->value("pos", QPoint(100, 100)).toPoint());
    mMain->restoreState(mAppSettings->value("windowState").toByteArray());

    mAppSettings->endGroup();

    // tool-/menubar
    mAppSettings->beginGroup("viewMenu");
    mMain->setProjectViewVisibility(mAppSettings->value("projectView").toBool());
    mMain->setOutputViewVisibility(mAppSettings->value("outputView").toBool());

    mAppSettings->endGroup();

    // history
    mAppSettings->beginGroup("fileHistory");
    mAppSettings->beginReadArray("lastOpenedFiles");
    for (int i = 0; i < mMain->history()->MAX_FILE_HISTORY; i++) {
        mAppSettings->setArrayIndex(i);
        mMain->history()->lastOpenedFiles.append(mAppSettings->value("file").toString());
    }
    mAppSettings->endArray();

    QMap<QString, QStringList> map;
    int size = mAppSettings->beginReadArray("commandLineOptions");
    for (int i = 0; i < size; i++) {
        mAppSettings->setArrayIndex(i);
        map.insert(mAppSettings->value("path").toString(),
                   mAppSettings->value("opt").toStringList());
    }
    mAppSettings->endArray();
    mMain->commandLineModel()->setAllOptions(map);

    size = mAppSettings->beginReadArray("openedTabs");
    for (int i = 0; i < size; i++) {
        mAppSettings->setArrayIndex(i);
        mMain->openFile(mAppSettings->value("location").toString());
    }

    mAppSettings->endArray();
    mAppSettings->endGroup();

    if (mUserSettings == nullptr)
        mUserSettings = new QSettings("GAMS", "Studio-User");

    mUserSettings->beginGroup("General");
    // todo
    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");
    // todo
    mUserSettings->endGroup();

    qDebug() << "loading user settings from" << mUserSettings->fileName();

    // TODO: before adding list of open tabs/files, add functionality to remove them from ui
}

}
}
