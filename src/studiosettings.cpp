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
    mAppSettings->beginGroup("mainWindow");

    // window
    mAppSettings->setValue("size", mMain->size());
    mAppSettings->setValue("pos", mMain->pos());
    mAppSettings->setValue("windowState", mMain->saveState());

    mAppSettings->endGroup();
    mAppSettings->beginGroup("viewMenu");

    // tool-/menubar
    mAppSettings->setValue("projectView", mMain->projectViewVisibility());
    mAppSettings->setValue("outputView", mMain->outputViewVisibility());

    mAppSettings->endGroup();
    mAppSettings->beginGroup("history");

    // history
    mAppSettings->beginWriteArray("lastOpenedFiles");
    for (int i = 0; i < mMain->history()->lastOpenedFiles.size(); i++) {
        mAppSettings->setArrayIndex(i);
        mAppSettings->setValue("file", mMain->history()->lastOpenedFiles.at(i));
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

    mAppSettings->beginGroup("mainWindow");

    // window
    mMain->resize(mAppSettings->value("size", QSize(1024, 768)).toSize());
    mMain->move(mAppSettings->value("pos", QPoint(100, 100)).toPoint());
    mMain->restoreState(mAppSettings->value("windowState").toByteArray());

    mAppSettings->endGroup();
    mAppSettings->beginGroup("viewMenu");

    // tool-/menubar
    mMain->setProjectViewVisibility(mAppSettings->value("projectView").toBool());
    mMain->setOutputViewVisibility(mAppSettings->value("outputView").toBool());

    mAppSettings->endGroup();
    mAppSettings->beginGroup("history");

    // history
    mAppSettings->beginReadArray("lastOpenedFiles");
    for (int i = 0; i < mMain->history()->MAX_FILE_HISTORY; i++) {
        mAppSettings->setArrayIndex(i);
        mMain->history()->lastOpenedFiles.append(mAppSettings->value("file").toString());
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
