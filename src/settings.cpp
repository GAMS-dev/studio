/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QSettings>
#include <QFile>
#include <QSize>
#include <QPoint>
#include <QFontDatabase>
#include "logger.h"
#include "settings.h"
#include "commonpaths.h"
#include "version.h"
#include "exception.h"
#include "file/dynamicfile.h"

namespace gams {
namespace studio {

// Increase mVersion only on MAJOR changes (change/remove existing field or add to array-element)
const int Settings::mVersion = 1;
Settings *Settings::mInstance = nullptr;

// ====== Some helper functions for string conversion ======

QString pointToString(QPoint p) {
    return QString("%1,%2").arg(p.x()).arg(p.y());
}
QString sizeToString(QSize s) {
    return QString("%1,%2").arg(s.width()).arg(s.height());
}
QList<int> toIntArray(QString s) {
    QList<int> res;
    for (QString v : s.split(',')) res << v.toInt();
    return res;
}
QPoint stringToPoint(QString s) {
    QList<int> a = toIntArray(s);
    if (a.size() == 2) return QPoint(a.at(0), a.at(1));
    return QPoint();
}
QSize stringToSize(QString s) {
    QList<int> a = toIntArray(s);
    if (a.size() == 2) return QSize(a.at(0), a.at(1));
    return QSize();
}

QString findFixedFont()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (font.fixedPitch())
        return font.family();
    QFontDatabase fdb;
    QStringList list = fdb.families();
    for (int i = 0; i < list.size(); ++i) {
        if (fdb.isPrivateFamily(list.at(i)))
            continue;
        if (fdb.isFixedPitch(list.at(i))) {
            return list.at(i);
        }
    }
    DEB() << "No fixed font found on system. Using " << font.family();
    return font.family();
}

// ====== reader and writer for JSON files ======

bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QJsonParseError parseResult;
    QJsonDocument json = QJsonDocument::fromJson(device.readAll(), &parseResult);
    if (parseResult.error) {
        DEB() << "JSON parse error at " << parseResult.offset << ": " << parseResult.errorString();
        return false;
    }
    map = json.object().toVariantMap();
    return true;
}

bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    device.write(QJsonDocument(QJsonObject::fromVariantMap(map)).toJson());
    return true;
}

// ====== Start of Settings methods ======

void Settings::createSettings(bool ignore, bool reset, bool resetView)
{
    if (mInstance) {
        DEB() << "Tried to create multiple settings, suppressed.";
    } else
        mInstance = new Settings(ignore, reset, resetView);
}

Settings *Settings::settings()
{
    return mInstance;
}

void Settings::releaseSettings()
{
    delete mInstance;
    mInstance = nullptr;
}

Settings::Settings(bool ignore, bool reset, bool resetView)
    : mCanWrite(!ignore),
      mCanRead(!ignore && !reset)
{
    // initialize json format and make it the default
    QSettings::Format jsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    QSettings::setDefaultFormat(jsonFormat);

    initKeys();

    initData(KAll);
    // QSettings only needed if we want to write
    if (mCanWrite || mCanRead) {
        QSettings *sUi = nullptr;
        // create basic non versionized application settings
        sUi = new QSettings(QSettings::defaultFormat(), QSettings::UserScope, GAMS_ORGANIZATION_STR, "uistates");
        if (sUi->status()) {
            // On Error -> create a backup and mark to reset this settings file
            QString uiFile = sUi->fileName();
            delete sUi;
            {
                DynamicFile file(uiFile, 2);
            }
            sUi = new QSettings(QSettings::defaultFormat(), QSettings::UserScope, GAMS_ORGANIZATION_STR, "uistates");
            if (sUi->status()) {
                mCanWrite = false;
                mCanRead = false;
                delete sUi;
                sUi = nullptr;
                DEB() << "Could not create settings files, switched to --ignore-settings";
            } else {
                DEB() << "Problems with reading uistates.json at first attempt. Created backup file.";
            }
        }
        if (sUi) {
            // only if the basic settings file has been created ...
            sUi->sync();
            mSettings.insert(KUi, sUi);
            // ... create versionized settings (may init from older version)
            createSettingFiles();
        }
    }
    if (resetView)
        resetViewSettings();

    QDir location(settingsPath());
    for (const QString &fileName: location.entryList({"*.lock"})) {
        QFile f(location.path() +  "/" + fileName);
        f.remove();
    }
}

Settings::~Settings()
{
    QMap<Kind, QSettings*>::iterator si = mSettings.begin();
    while (si != mSettings.end()) {
        QSettings *set = si.value();
        si = mSettings.erase(si);
        delete set;
    }
}

void Settings::initKeys()
{
    // prepare storage
    mData.insert(KUi, Data());
    mData.insert(KSys, Data());
    mData.insert(KUser, Data());

    // versions for all settings files (KAll)
    mKeys.insert(_VersionSettings, KeyData(KAll, {"version","settings"}, mVersion));
    mKeys.insert(_VersionStudio, KeyData(KAll, {"version","studio"}, QString(GAMS_VERSION_STR)));

    // window settings
    mKeys.insert(_winSize, KeyData(KUi, {"window","size"}, QString("1024,768")));
    mKeys.insert(_winPos, KeyData(KUi, {"window","pos"}, QString("0,0")));
    mKeys.insert(_winState, KeyData(KUi, {"window","state"}, QByteArray("")));
    mKeys.insert(_winMaximized, KeyData(KUi, {"window","maximized"}, false));

    // view menu settings
    mKeys.insert(_viewProject, KeyData(KUi, {"viewMenu","project"}, true));
    mKeys.insert(_viewOutput, KeyData(KUi, {"viewMenu","output"}, true));
    mKeys.insert(_viewHelp, KeyData(KUi, {"viewMenu","help"}, false));
    mKeys.insert(_viewOption, KeyData(KUi, {"viewMenu","optionEdit"}, false));

    // general system settings
    mKeys.insert(_encodingMib, KeyData(KSys, {"encodingMIBs"}, QString("106,0,4,17,2025")));
    mKeys.insert(_projects, KeyData(KSys, {"projects"}, QJsonObject()));
    mKeys.insert(_tabs, KeyData(KSys, {"tabs"}, QJsonObject()));
    mKeys.insert(_history, KeyData(KSys, {"history"}, QJsonArray()));

    // settings of help page
    mKeys.insert(_hBookmarks, KeyData(KSys, {"help","bookmarks"}, QJsonArray()));
    mKeys.insert(_hZoomFactor, KeyData(KSys, {"help","bookmarks"}, 1.0));

    // search widget
    mKeys.insert(_searchUseRegex, KeyData(KSys, {"search", "regex"}, false));
    mKeys.insert(_searchCaseSens, KeyData(KSys, {"search", "caseSens"}, false));
    mKeys.insert(_searchWholeWords, KeyData(KSys, {"search", "wholeWords"}, false));
    mKeys.insert(_searchScope, KeyData(KSys, {"search", "scope"}, 0));

    // general settings page
    mKeys.insert(_defaultWorkspace, KeyData(KUser, {"defaultWorkspace"}, CommonPaths::defaultWorkingDir()));
    mKeys.insert(_skipWelcomePage, KeyData(KUser, {"skipWelcome"}, false));
    mKeys.insert(_restoreTabs, KeyData(KUser, {"restoreTabs"}, true));
    mKeys.insert(_autosaveOnRun, KeyData(KUser, {"autosaveOnRun"}, true));
    mKeys.insert(_openLst, KeyData(KUser, {"openLst"}, false));
    mKeys.insert(_jumpToError, KeyData(KUser, {"jumpToError"}, true));
    mKeys.insert(_foregroundOnDemand, KeyData(KUser, {"foregroundOnDemand"}, true));
    mKeys.insert(_historySize, KeyData(KUser, {"historySize"}, 12));

    // editor settings page
    mKeys.insert(_edFontFamily, KeyData(KUser, {"editor","fontFamily"}, findFixedFont()));
    mKeys.insert(_edFontSize, KeyData(KUser, {"editor","fontSize"}, 10));
    mKeys.insert(_edShowLineNr, KeyData(KUser, {"editor","showLineNr"}, true));
    mKeys.insert(_edTabSize, KeyData(KUser, {"editor","TabSize"}, 4));
    mKeys.insert(_edLineWrapEditor, KeyData(KUser, {"editor","lineWrapEditor"}, false));
    mKeys.insert(_edLineWrapProcess, KeyData(KUser, {"editor","lineWrapProcess"}, false));
    mKeys.insert(_edClearLog, KeyData(KUser, {"editor","clearLog"}, false));
    mKeys.insert(_edWordUnderCursor, KeyData(KUser, {"editor","wordUnderCursor"}, false));
    mKeys.insert(_edHighlightCurrentLine, KeyData(KUser, {"editor","highlightCurrentLine"}, false));
    mKeys.insert(_edAutoIndent, KeyData(KUser, {"editor","autoIndent"}, true));
    mKeys.insert(_edWriteLog, KeyData(KUser, {"editor","writeLog"}, true));
    mKeys.insert(_edLogBackupCount, KeyData(KUser, {"editor","logBackupCount"}, 3));
    mKeys.insert(_edAutoCloseBraces, KeyData(KUser, {"editor","autoCloseBraces"}, true));
    mKeys.insert(_edEditableMaxSizeMB, KeyData(KUser, {"editor","editableMaxSizeMB"}, 50));

    // MIRO settings page
    mKeys.insert(_miroInstallPath, KeyData(KUser, {"miro","installationLocation"}, QString()));

    // solver option editor settings
    mKeys.insert(_soOverrideExisting, KeyData(KUser, {"solverOption","overrideExisting"}, true));
    mKeys.insert(_soAddCommentAbove, KeyData(KUser, {"solverOption","addCommentAbove"}, false));
    mKeys.insert(_soAddEOLComment, KeyData(KUser, {"solverOption","addEOLComment"}, false));
    mKeys.insert(_soDeleteCommentsAbove, KeyData(KUser, {"solverOption","deleteCommentsAbove"}, false));

    // user model library directory
    mKeys.insert(_userModelLibraryDir, KeyData(KSys, {"userModelLibraryDir"}, CommonPaths::userModelLibraryDir()));

}

int Settings::checkVersion()
{
    int res = mVersion;
    QDir dir = settingsPath();

    // Find setting files of the highest version up to mVersion
    QStringList files = dir.entryList(QDir::Files);
    while (res) {
        if (files.contains(QString("systemsettings%1.json").arg(res))) break;
        if (files.contains(QString("usersettings%1.json").arg(res))) break;
        --res;
    }
    // nothing to do if no setting file found
    if (!res) res = mVersion;

    return res;
}

bool Settings::createSettingFiles()
{
    // look for latest setting files
    int version = checkVersion();

    // create setting files of found version to read from
    initSettingsFiles(version);
    load(KSys);
    load(KUser);
    if (version == mVersion) return true;

    // Need to upgrade from older version
    while (version < mVersion) {
        switch (version) {
        case 1:
            // On increasing version from 1 to 2 -> implement mData conversion HERE
            break;
        case 2:
            // On increasing version from 2 to 3 -> implement mData conversion HERE
            break;
        default:
            break;
        }
        ++version;
    }

    // write setting files in current version
    initSettingsFiles(version);
    save();
    return true;
}

void Settings::initSettingsFiles(int version)
{
    // initializes versionized setting files
    mSettings.insert(KSys, new QSettings(QSettings::defaultFormat(),  QSettings::UserScope,
                                         GAMS_ORGANIZATION_STR, QString("systemsettings%1").arg(version)));
    mSettings.insert(KUser, new QSettings(QSettings::defaultFormat(), QSettings::UserScope,
                                          GAMS_ORGANIZATION_STR, QString("usersettings%1").arg(version)));
    // TODO(JM) Handle studioscheme.json and syntaxscheme.json
}

void Settings::reset(Kind kind)
{
    for (Kind &k : mData.keys()) {
        if (kind == KAll || k == kind) mData[k].clear();
    }
    initData(kind);
}

void Settings::initData(Kind kind)
{
    QHash<SettingsKey, KeyData>::const_iterator di = mKeys.constBegin();
    for ( ; di != mKeys.constEnd() ; ++di) {
        KeyData dat = di.value();
        if (kind == KAll || kind == dat.kind)
            setValue(di.key(), dat.initial);
    }
    // create default values for current mVersion
//    if (kind == KAll)
//        Scheme::instance()->initDefault();
}

void Settings::reload()
{
    load(KAll);
}

void Settings::resetViewSettings()
{
    initData(KUi);
    QSettings *set = mSettings.value(KUi);
    if (set) set->sync();
}

void Settings::save()
{
    // ignore-settings argument -> no settings assigned
    if (!mCanWrite) return;
    for (const Kind &kind : mSettings.keys()) {
        saveFile(kind);
    }
}

QSize Settings::toSize(SettingsKey key) const
{
    return stringToSize(toString(key));
}

QPoint Settings::toPoint(SettingsKey key) const
{
    return stringToPoint(toString(key));
}

QJsonObject Settings::toJsonObject(SettingsKey key) const
{
    return value(key).toJsonObject();
}

QJsonArray Settings::toJsonArray(SettingsKey key) const
{
    return value(key).toJsonArray();
}

void Settings::setSize(SettingsKey key, const QSize &value)
{
    setValue(key, sizeToString(value));
}

void Settings::setPoint(SettingsKey key, const QPoint &value)
{
    setValue(key, pointToString(value));
}

bool Settings::isValidVersion(QString currentVersion)
{
    for (const QChar &c: currentVersion)
        if (c != '.' && (c < '0' || c > '9')) return false;
    QStringList verList = currentVersion.split('.');
    if (verList.size() < 2) return false;
    for (const QString &s: verList)
        if (s.isEmpty()) return false;
    return true;
}

int Settings::compareVersion(QString currentVersion, QString otherVersion)
{
    QStringList curntList = currentVersion.split('.');
    QStringList otherList = otherVersion.split('.');
    for (int i = 0; i < qMax(curntList.size(), otherList.size()); ++i) {
        if (i == curntList.size()) return -1;
        if (i == otherList.size()) return 1;
        bool a,b;
        int res = curntList.at(i).toInt(&a) - otherList.at(i).toInt(&b);
        if (a && !b) return 2;
        if (b && !a) return -2;
        if (res) return res/qAbs(res);
    }
    return 0;
}

QVariant Settings::value(SettingsKey key) const
{
    KeyData dat = mKeys.value(key);
    if (!mData.contains(dat.kind))
        return QVariant();
    if (dat.keys.length() == 1)
        return mData[dat.kind].value(dat.keys.at(0));
    if (dat.keys.length() == 2) {
        QJsonObject jo = mData[dat.kind].value(dat.keys.at(0)).toJsonObject();
        return jo.value(dat.keys.at(1));
    }
    return QVariant();
}

bool Settings::setValue(SettingsKey key, QVariant value)
{
    KeyData dat = mKeys.value(key);
    if (!mData.contains(dat.kind)) // ensure that entry for kind exists
        mData.insert(dat.kind, Data());
    if (dat.keys.length() == 1) {
        mData[dat.kind].insert(dat.keys.at(0), value);
        return true;
    }
    if (dat.keys.length() == 2) {
        // ensure that Data entry for group-key exists
        if (!mData[dat.kind].contains(dat.keys.at(0)))
            mData[dat.kind].insert(dat.keys.at(0), Data());
        QJsonObject jo = mData[dat.kind].value(dat.keys.at(0)).toJsonObject();
        switch (QMetaType::Type(value.type())) {
        case QMetaType::Double: jo[dat.keys.at(1)] = value.toDouble(); break;
        case QMetaType::ULongLong:
        case QMetaType::LongLong: jo[dat.keys.at(1)] = value.toLongLong(); break;
        case QMetaType::UInt:
        case QMetaType::Int: jo[dat.keys.at(1)] = value.toInt(); break;
        case QMetaType::Bool: jo[dat.keys.at(1)] = value.toBool(); break;
        case QMetaType::QByteArray:
        case QMetaType::QString: jo[dat.keys.at(1)] = value.toString(); break;
        case QMetaType::QJsonObject: jo[dat.keys.at(1)] = value.toJsonObject(); break;
        case QMetaType::QJsonArray: jo[dat.keys.at(1)] = value.toJsonArray(); break;
        default: return false;
        }
        mData[dat.kind].insert(dat.keys.at(0), jo);
        return true;
    }
    return false;
}

bool Settings::setJsonObject(SettingsKey key, QJsonObject value)
{
    return setValue(key, value);
}

bool Settings::setJsonArray(SettingsKey key, QJsonArray value)
{
    return setValue(key, value);
}


QStringList Settings::fileHistory()
{
    QStringList res;
    QJsonArray joLastOpenedFiles = mData[KSys].value("lastOpenedFiles").toJsonArray();
    for (QJsonValue jRef: joLastOpenedFiles) {
        res << jRef["name"].toString();
    }
    return res;
}

QString Settings::settingsPath()
{
    if (QSettings *settings = mSettings[KUi]) {
        DEB() << "UiSettings: " << settings->fileName();
        return QFileInfo(settings->fileName()).path();
    }
    DEB() << "ERROR: Settings file must be initialized before using settingsPath()";
    return QString();
}

void Settings::saveFile(Kind kind)
{
    if (!mSettings.contains(kind)) return;

    // Store values that are repeated in ALL settings, like the versions
    Data::const_iterator it = mData[KAll].constBegin();
    for ( ; it != mData[kind].constEnd() ; ++it) {
        mSettings[kind]->setValue(it.key(), it.value());
    }

    // store individual settings
    it = mData[kind].constBegin();
    for ( ; it != mData[kind].constEnd() ; ++it) {
        mSettings[kind]->setValue(it.key(), it.value());
    }
    mSettings[kind]->sync();
}

QVariant Settings::read(SettingsKey key, Settings::Kind kind)
{
    KeyData dat = keyData(key);
    if (kind == KAll) kind = dat.kind;
    QSettings *qs = mSettings.value(kind, nullptr);
    if (!qs) return QVariant();
    if (dat.keys.size() == 1) return  qs->value(dat.keys[0]);
    if (dat.keys.size() == 2) {
        QJsonObject jo = qs->value(dat.keys[0]).toJsonObject();
        return jo[dat.keys[1]].toVariant();
    }
    return QVariant();
}

void Settings::load(Kind kind)
{
    if (mCanRead) {
        // sync all settings
        QMap<Kind, QSettings*>::const_iterator si = mSettings.constBegin();
        for ( ; si != mSettings.constEnd() ; ++si) {
            if (si.value()) si.value()->sync();
        }
        // write settings content into mData
        QHash<SettingsKey, KeyData>::const_iterator di = mKeys.constBegin();
        for ( ; di != mKeys.constEnd() ; ++di) {
            KeyData dat = di.value();

            if (dat.kind == KAll) {
                // --- special handling of entries for all files (like version)
                if (kind == KAll) {
                    // Update shared entry over all files
                    for (const Kind &k : mData.keys()) {
                        if (k == KAll) continue;
                        QVariant var = read(di.key(), k);
                        if (var.isValid()) setValue(di.key(), var);
                    }
                } else {
                    // Only update given kind
                    QVariant var = read(di.key());
                    if (var.isValid()) setValue(di.key(), var);
                }

            } else if (kind == KAll || kind == dat.kind) {
                // --- common update of entry
                QVariant var = read(di.key());
                if (var.isValid()) setValue(di.key(), var);
            }
        }
    }
//    Scheme::instance()->initDefault();


    // the location for user model libraries is not modifyable right now
    // anyhow, it is part of StudioSettings since it might become modifyable in the future
    setString(_userModelLibraryDir, CommonPaths::userModelLibraryDir());
}

void Settings::importSettings(const QString &path)
{
    if (!mSettings.value(KUser)) return;
    QFile backupFile(path);
    QFile settingsFile(mSettings.value(KUser)->fileName());
    settingsFile.remove(); // remove old file
    backupFile.copy(settingsFile.fileName()); // import new file
    reload();
    load(KUser);
}

void Settings::exportSettings(const QString &path)
{
    if (!mSettings.value(KUser)) return;
    QFile originFile(mSettings.value(KUser)->fileName());
    originFile.copy(path);
}

}
}
