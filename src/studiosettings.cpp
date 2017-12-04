#include "studiosettings.h"
#include "gamspaths.h"

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

    QMap<QString, QStringList> map(mMain->commandLineModel()->allHistory());
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

    mUserSettings->setValue("defaultWorkspace", defaultWorkspace());
    mUserSettings->setValue("skipWelcome", skipWelcomePage());
    mUserSettings->setValue("restoreTabs", restoreTabs());
    mUserSettings->setValue("autosaveOnRun", autosaveOnRun());
    mUserSettings->setValue("openLst", openLst());
    mUserSettings->setValue("jumpToError", jumpToError());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");

    mUserSettings->setValue("fontFamily", fontFamily());
    mUserSettings->setValue("fontSize", fontSize());
    mUserSettings->setValue("showLineNr", showLineNr());
    mUserSettings->setValue("replaceTabsWithSpaces", replaceTabsWithSpaces());
    mUserSettings->setValue("tabSize", tabSize());
    mUserSettings->setValue("lineWrap", lineWrap());

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
    mMain->commandLineModel()->setAllHistory(map);

    mAppSettings->endGroup();

    if (mUserSettings == nullptr)
        mUserSettings = new QSettings("GAMS", "Studio-User");

    mUserSettings->beginGroup("General");

    setDefaultWorkspace(mUserSettings->value("defaultWorkspace", GAMSPaths::defaultWorkingDir()).toString());
    setSkipWelcomePage(mUserSettings->value("skipWelcome", false).toBool());
    setRestoreTabs(mUserSettings->value("restoreTabs", true).toBool());
    setAutosaveOnRun(mUserSettings->value("autosaveOnRun", true).toBool());
    setOpenLst(mUserSettings->value("openLst", false).toBool());
    setJumpToError(mUserSettings->value("jumpToError", true).toBool());

    mUserSettings->endGroup();
    mUserSettings->beginGroup("Editor");

    QFont ff = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    setFontFamily(mUserSettings->value("fontFamily", ff.defaultFamily()).toString());
    setFontSize(mUserSettings->value("fontSize", 10).toInt());
    setShowLineNr(mUserSettings->value("showLineNr", true).toBool());
    setReplaceTabsWithSpaces(mUserSettings->value("replaceTabsWithSpaces", false).toBool());
    setTabSize(mUserSettings->value("tabSize", 4).toInt());
    setLineWrap(mUserSettings->value("lineWrap", true).toBool());

    mUserSettings->endGroup();

    qDebug() << "loading user settings from" << mUserSettings->fileName();

    // TODO: before adding list of open tabs/files, add functionality to remove them from ui

    // only after loading all settings tabs can be restored
    if(!restoreTabs())
        return;

    mAppSettings->beginGroup("fileHistory");
    size = mAppSettings->beginReadArray("openedTabs");
    for (int i = 0; i < size; i++) {
        mAppSettings->setArrayIndex(i);
        QString value = mAppSettings->value("location").toString();
        if(QFileInfo(value).exists())
            mMain->openFile(value);
    }
    mAppSettings->endArray();
    mAppSettings->endGroup();
}

QString StudioSettings::defaultWorkspace() const
{
    return mDefaultWorkspace;
}

void StudioSettings::setDefaultWorkspace(const QString &value)
{
    mDefaultWorkspace = value;
}

bool StudioSettings::skipWelcomePage() const
{
    return mSkipWelcomePage;
}

void StudioSettings::setSkipWelcomePage(bool value)
{
    mSkipWelcomePage = value;
}

bool StudioSettings::restoreTabs() const
{
    return mRestoreTabs;
}

void StudioSettings::setRestoreTabs(bool value)
{
    mRestoreTabs = value;
}

bool StudioSettings::autosaveOnRun() const
{
    return mAutosaveOnRun;
}

void StudioSettings::setAutosaveOnRun(bool value)
{
    mAutosaveOnRun = value;
}

bool StudioSettings::openLst() const
{
    return mOpenLst;
}

void StudioSettings::setOpenLst(bool value)
{
    mOpenLst = value;
}

bool StudioSettings::jumpToError() const
{
    return mJumpToError;
}

void StudioSettings::setJumpToError(bool value)
{
    mJumpToError = value;
}

int StudioSettings::fontSize() const
{
    return mFontSize;
}

void StudioSettings::setFontSize(int value)
{
    mFontSize = value;
}

bool StudioSettings::showLineNr() const
{
    return mShowLineNr;
}

void StudioSettings::setShowLineNr(bool value)
{
    mShowLineNr = value;
}

bool StudioSettings::replaceTabsWithSpaces() const
{
    return mReplaceTabsWithSpaces;
}

void StudioSettings::setReplaceTabsWithSpaces(bool value)
{
    mReplaceTabsWithSpaces = value;
}

int StudioSettings::tabSize() const
{
    return mTabSize;
}

void StudioSettings::setTabSize(int value)
{
    mTabSize = value;
}

bool StudioSettings::lineWrap() const
{
    return mLineWrap;
}

void StudioSettings::setLineWrap(bool value)
{
    mLineWrap = value;
}

QString StudioSettings::fontFamily() const
{
    return mFontFamily;
}

void StudioSettings::setFontFamily(const QString &value)
{
    mFontFamily = value;
}

void StudioSettings::updateEditors(QString fontFamily, int fontSize)
{
    QFont font(fontFamily, fontSize);
    foreach (QPlainTextEdit* edit, mMain->openEditors()) {
        edit->setFont(font);
    }
}

}
}
