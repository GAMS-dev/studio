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
bool Settings::mUseRelocatedTestDir = false;

// ====== Some helper functions ======

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

int Settings::version()
{
    return mVersion;
}

void Settings::useRelocatedPathForTests()
{
    if (mInstance) {
        DEB() << "Can't relocate after instance has been created";
    } else {
        mUseRelocatedTestDir = true;
    }
}

Settings::Settings(bool ignore, bool reset, bool resetView)
    : mCanWrite(!ignore),
      mCanRead(!ignore && !reset),
      mKeys(generateKeys())
{
    // initialize storage
    mData.insert(scUi, Data());
    mData.insert(scSys, Data());
    mData.insert(scUser, Data());

    // initialize json format and make it the default
    QSettings::Format jsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    QSettings::setDefaultFormat(jsonFormat);
    if (mUseRelocatedTestDir)
        QSettings::setPath(jsonFormat, QSettings::UserScope, ".");


    initDefault(scAll);

    // QSettings only needed if we want to write
    if (mCanWrite || mCanRead) {
        QSettings *sUi = nullptr;
        // create basic non versionized application settings
        sUi = newQSettings("uistates");
        if (sUi->status() != QSettings::NoError) {
            if (sUi->status() == QSettings::FormatError) {
                QString uiFile = sUi->fileName();
                delete sUi;
                if (QFile(uiFile).exists()) {
                    DynamicFile(uiFile, 2); // creates backup
                    DEB() << " - created backup file.";
                }
                sUi = newQSettings("uistates");
            }
            if (sUi->status()) {
                delete sUi;
                sUi = nullptr;
            }
        }
        if (sUi) {
            // only if the basic settings file has been created ...
            sUi->sync();
            mSettings.insert(scUi, sUi);
            load(scUi);
//            saveFile(KUi);
            // ... create versionized settings (may init from older version)
            createSettingFiles();
            QDir location(settingsPath());
            for (const QString &fileName: location.entryList({"*.lock"})) {
                QFile f(location.path() +  "/" + fileName);
                f.remove();
            }
        } else {
            mCanWrite = false;
            mCanRead = false;
            DEB() << "Could not create settings files, switched to --ignore-settings";
        }
    }
    if (resetView)
        resetViewSettings();

}

Settings::~Settings()
{
    QMap<Scope, QSettings*>::iterator si = mSettings.begin();
    while (si != mSettings.end()) {
        QSettings *set = si.value();
        set->sync();
        si = mSettings.erase(si);
        delete set;
    }
}

QSettings *Settings::newQSettings(QString name)
{
    QSettings *res = new QSettings(QSettings::defaultFormat(), QSettings::UserScope, GAMS_ORGANIZATION_STR, name);
    if (res->status()) {
        DEB() << (res->status()==1 ? "Access-" : "Format-") << "error in file: " << res->fileName();
    }
    return res;
}

QHash<SettingsKey, Settings::KeyData> Settings::generateKeys()
{
    QHash<SettingsKey, Settings::KeyData> res;

    // versions for all settings files (KAll)
    res.insert(skVersionSettings, KeyData(scAll, {"version","settings"}, mVersion));
    res.insert(skVersionStudio, KeyData(scAll, {"version","studio"}, QString(GAMS_VERSION_STR)));

    // window settings
    res.insert(skWinSize, KeyData(scUi, {"window","size"}, QString("1024,768")));
    res.insert(skWinPos, KeyData(scUi, {"window","pos"}, QString("0,0")));
    res.insert(skWinState, KeyData(scUi, {"window","state"}, QByteArray("")));
    res.insert(skWinMaximized, KeyData(scUi, {"window","maximized"}, false));

    // view menu settings
    res.insert(skViewProject, KeyData(scUi, {"viewMenu","project"}, true));
    res.insert(skViewOutput, KeyData(scUi, {"viewMenu","output"}, true));
    res.insert(skViewHelp, KeyData(scUi, {"viewMenu","help"}, false));
    res.insert(skViewOption, KeyData(scUi, {"viewMenu","optionEdit"}, false));

    // general system settings
    res.insert(skDefaultCodecMib, KeyData(scSys, {"defaultCodecMib"}, 106));
    res.insert(skEncodingMib, KeyData(scSys, {"encodingMIBs"}, QString("106,0,4,17,2025")));
    res.insert(skProjects, KeyData(scSys, {"projects"}, QJsonObject()));
    res.insert(skTabs, KeyData(scSys, {"tabs"}, QJsonObject()));
    res.insert(skHistory, KeyData(scSys, {"history"}, QJsonArray()));

    // settings of help page
    res.insert(skHelpBookmarks, KeyData(scSys, {"help","bookmarks"}, QJsonArray()));
    res.insert(skHelpZoomFactor, KeyData(scSys, {"help","zoom"}, 1.0));

    // search widget
    res.insert(skSearchUseRegex, KeyData(scSys, {"search", "regex"}, false));
    res.insert(skSearchCaseSens, KeyData(scSys, {"search", "caseSens"}, false));
    res.insert(skSearchWholeWords, KeyData(scSys, {"search", "wholeWords"}, false));
    res.insert(skSearchScope, KeyData(scSys, {"search", "scope"}, 0));

    // general settings page
    res.insert(skDefaultWorkspace, KeyData(scUser, {"defaultWorkspace"}, CommonPaths::defaultWorkingDir()));
    res.insert(skSkipWelcomePage, KeyData(scUser, {"skipWelcome"}, false));
    res.insert(skRestoreTabs, KeyData(scUser, {"restoreTabs"}, true));
    res.insert(skAutosaveOnRun, KeyData(scUser, {"autosaveOnRun"}, true));
    res.insert(skOpenLst, KeyData(scUser, {"openLst"}, false));
    res.insert(skJumpToError, KeyData(scUser, {"jumpToErrors"}, true));
    res.insert(skForegroundOnDemand, KeyData(scUser, {"foregroundOnDemand"}, true));
    res.insert(skHistorySize, KeyData(scUser, {"historySize"}, 12));

    // editor settings page
    res.insert(skEdColorSchemeIndex, KeyData(scUser, {"editor","colorSchemeIndex"}, 0));
    res.insert(skEdFontFamily, KeyData(scUser, {"editor","fontFamily"}, findFixedFont()));
    res.insert(skEdFontSize, KeyData(scUser, {"editor","fontSize"}, 10));
    res.insert(skEdShowLineNr, KeyData(scUser, {"editor","showLineNr"}, true));
    res.insert(skEdTabSize, KeyData(scUser, {"editor","TabSize"}, 4));
    res.insert(skEdLineWrapEditor, KeyData(scUser, {"editor","lineWrapEditor"}, false));
    res.insert(skEdLineWrapProcess, KeyData(scUser, {"editor","lineWrapProcess"}, false));
    res.insert(skEdClearLog, KeyData(scUser, {"editor","clearLog"}, false));
    res.insert(skEdWordUnderCursor, KeyData(scUser, {"editor","wordUnderCursor"}, false));
    res.insert(skEdHighlightCurrentLine, KeyData(scUser, {"editor","highlightCurrentLine"}, false));
    res.insert(skEdAutoIndent, KeyData(scUser, {"editor","autoIndent"}, true));
    res.insert(skEdWriteLog, KeyData(scUser, {"editor","writeLog"}, true));
    res.insert(skEdLogBackupCount, KeyData(scUser, {"editor","logBackupCount"}, 3));
    res.insert(skEdAutoCloseBraces, KeyData(scUser, {"editor","autoCloseBraces"}, true));
    res.insert(skEdEditableMaxSizeMB, KeyData(scUser, {"editor","editableMaxSizeMB"}, 50));

    // MIRO settings page
    res.insert(skMiroInstallPath, KeyData(scUser, {"miro","installationLocation"}, QString()));

    // solver option editor settings
    res.insert(skSoOverrideExisting, KeyData(scUser, {"solverOption","overrideExisting"}, true));
    res.insert(skSoAddCommentAbove, KeyData(scUser, {"solverOption","addCommentAbove"}, false));
    res.insert(skSoAddEOLComment, KeyData(scUser, {"solverOption","addEOLComment"}, false));
    res.insert(skSoDeleteCommentsAbove, KeyData(scUser, {"solverOption","deleteCommentsAbove"}, false));

    // user model library directory
    res.insert(skUserModelLibraryDir, KeyData(scSys, {"userModelLibraryDir"}, CommonPaths::userModelLibraryDir()));
    return res;
}

//void Settings::initGroups()
//{
//    for (QHash<SettingsKey, KeyData>::const_iterator it = mKeys.constBegin(); it == mKeys.constEnd(); ++it) {
//        if (it.value().keys.size() != 2) continue;
//        if (!mGroups.contains(it.value().keys.first()))
//            mGroups.insert(it.value().keys.first(), QList<SettingsKey>());
//        mGroups[it.value().keys.first()] << it.key();
//    }
//}

int Settings::checkVersion()
{
    int res = mVersion;
    QDir dir(settingsPath());

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
    load(scSys);
    load(scUser);
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
//    save();
    return true;
}

void Settings::initSettingsFiles(int version)
{
    // initializes versionized setting files
    mSettings.insert(scSys, newQSettings(QString("systemsettings%1").arg(version)));
    mSettings.insert(scUser, newQSettings(QString("usersettings%1").arg(version)));
    // TODO(JM) Handle studioscheme.json and syntaxscheme.json
}

void Settings::reset(Scope scope)
{
    for (Scope &sc : mData.keys()) {
        if (scope == scAll || sc == scope) mData[sc].clear();
    }
    initDefault(scope);
}

void Settings::initDefault(Scope scope)
{
    QHash<SettingsKey, KeyData>::const_iterator di = mKeys.constBegin();
    for ( ; di != mKeys.constEnd() ; ++di) {
        KeyData dat = di.value();
        if (scope == scAll || scope == dat.scope)
            setValue(di.key(), dat.initial);
    }
}

void Settings::reload()
{
    load(scAll);
}

void Settings::resetViewSettings()
{
    initDefault(scUi);
    QSettings *set = mSettings.value(scUi);
    if (set) set->sync();
}

void Settings::save()
{
    // ignore-settings argument -> no settings assigned
    if (!canWrite()) return;
    for (const Scope &scope : mSettings.keys()) {
        saveFile(scope);
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

QByteArray Settings::toByteArray(SettingsKey key) const
{
    QString str = value(key).toString();
    return QByteArray::fromHex(str.toLatin1());
}

QVariantMap Settings::toVariantMap(SettingsKey key) const
{
    return value(key).toMap();
}

QVariantList Settings::toList(SettingsKey key) const
{
    return value(key).toList();
}

void Settings::setSize(SettingsKey key, const QSize &value)
{
    setValue(key, sizeToString(value));
}

void Settings::setPoint(SettingsKey key, const QPoint &value)
{
    setValue(key, pointToString(value));
}

bool Settings::setMap(SettingsKey key, QVariantMap value)
{
    return setValue(key, value);
}

bool Settings::setList(SettingsKey key, QVariantList value)
{
    return setValue(key, value);
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
    if (!mData.contains(dat.scope))
        return QVariant();
    if (dat.keys.length() == 1)
        return mData[dat.scope].value(dat.keys.at(0));
    if (dat.keys.length() == 2) {
        QJsonObject jo = mData[dat.scope].value(dat.keys.at(0)).toJsonObject();
        return jo.value(dat.keys.at(1));
    }
    return QVariant();
}

bool Settings::setValue(SettingsKey key, QVariant value)
{
    KeyData dat = mKeys.value(key);
    if (dat.keys.length() == 1)
        return setDirectValue(dat.scope, dat.keys.at(0), value);
    if (dat.keys.length() == 2)
        return setDirectValue(dat.scope, dat.keys.at(0), dat.keys.at(1), value);
    return false;
}

QVariant Settings::directValue(const Scope &scope, const QString &key) const
{
    if (!mData.contains(scope)) return QVariant();
    return mData[scope].value(key);
}

QVariant Settings::directValue(const Settings::Scope &scope, const QString &group, const QString &key) const
{
    if (!mData.contains(scope)) return QVariant();
    QJsonObject jo = mData[scope].value(group).toJsonObject();
    return jo.value(key);
}

bool Settings::setDirectValue(const Settings::Scope &scope, const QString &key, QVariant value)
{
    if (!mData.contains(scope)) // ensure that entry for the scope exists
        mData.insert(scope, Data());
    mData[scope].insert(key, value);
    return true;
}

bool Settings::setDirectValue(const Settings::Scope &scope, const QString &group, const QString &key, QVariant value)
{
    if (!mData.contains(scope)) // ensure that entry for the scope exists
        mData.insert(scope, Data());
    if (!mData[scope].contains(group))
        mData[scope].insert(group, QJsonObject());
    QVariant varGroup = mData[scope].value(group);
    if (!varGroup.canConvert<QJsonObject>() && !varGroup.canConvert<QVariantMap>())
        return false;
    QVariantMap jo = varGroup.toMap();
    if (!addToMap(jo, key, value)) return false;
    mData[scope].insert(group, jo);
    return true;
}

bool Settings::addToMap(QVariantMap &group, const QString &key, QVariant value)
{
    switch (QMetaType::Type(value.type())) {
    case QMetaType::Double: group[key] = value.toDouble(); break;
    case QMetaType::ULongLong:
    case QMetaType::LongLong: group[key] = value.toLongLong(); break;
    case QMetaType::UInt:
    case QMetaType::Int: group[key] = value.toInt(); break;
    case QMetaType::Bool: group[key] = value.toBool(); break;
    case QMetaType::QByteArray: group[key] = QString(value.toByteArray().toHex()); break;
    case QMetaType::QString: group[key] = value.toString(); break;
    case QMetaType::QVariantMap: group[key] = value.toJsonObject(); break;
    case QMetaType::QVariantList: group[key] = value.toJsonArray(); break;
    case QMetaType::QJsonObject: group[key] = value.toJsonObject(); break;
    case QMetaType::QJsonArray: group[key] = value.toJsonArray(); break;
    default: return false;
    }
    return true;
}

QString Settings::settingsPath()
{
    if (QSettings *settings = mSettings[scUi]) {
        return QFileInfo(settings->fileName()).path();
    }
    DEB() << "ERROR: Settings file must be initialized before using settingsPath()";
    return QString();
}

void Settings::saveFile(Scope scope)
{
    if (!canWrite()) return;
    if (!mSettings.contains(scope)) return;

    // Store values that are repeated in ALL settings, like the versions
    Data::const_iterator it = mData[scAll].constBegin();
    for ( ; it != mData[scAll].constEnd() ; ++it) {
        mSettings[scope]->setValue(it.key(), it.value());
    }

    // store individual settings
    it = mData[scope].constBegin();
    for ( ; it != mData[scope].constEnd() ; ++it) {
        mSettings[scope]->setValue(it.key(), it.value());
    }
    mSettings[scope]->sync();
}

QVariant Settings::read(SettingsKey key, Settings::Scope scope)
{
    KeyData dat = keyData(key);
    if (scope == scAll) scope = dat.scope;
    QSettings *qs = mSettings.value(scope, nullptr);
    if (!qs) return QVariant();
    if (dat.keys.size() == 1) return  qs->value(dat.keys[0]);
    if (dat.keys.size() == 2) {
        QJsonObject jo = qs->value(dat.keys[0]).toJsonObject();
        return jo[dat.keys[1]].toVariant();
    }
    return QVariant();
}

void Settings::load(Scope scope)
{
    if (mCanRead) {

        // load settings content into mData
        for (QMap<Scope, QSettings*>::const_iterator si = mSettings.constBegin() ; si != mSettings.constEnd() ; ++si) {
            if (!si.value() || (scope != scAll && scope != si.key())) continue;

            // sync settings of this scope
            si.value()->sync();

            // iterate over the settings content
            for (const QString &key : si.value()->allKeys()) {
                QVariant var = si.value()->value(key);
                if (var.isNull()) continue;
                if (var.canConvert<QJsonObject>() || var.canConvert<QVariantMap>()) {
                    // copy all elements
                    QJsonObject joSrc = var.toJsonObject();
                    QJsonObject joDest = mData.value(si.key()).value(key).toJsonObject();
                    for (const QString &joKey : joSrc.keys()) {
                        joDest[joKey] = joSrc[joKey];
                    }
                    var = joDest;
                }
                setDirectValue(si.key(), key, var);
            }
        }



        QHash<SettingsKey, KeyData>::const_iterator di = mKeys.constBegin();


        for ( ; di != mKeys.constEnd() ; ++di) {
            KeyData dat = di.value();

            if (dat.scope == scAll) {
                // --- special handling of entries for all files (like version)
                if (scope == scAll) {
                    // Update shared entry over all files
                    for (const Scope &k : mData.keys()) {
                        if (k == scAll) continue;
                        QVariant var = read(di.key(), k);
                        if (var.isValid()) setValue(di.key(), var);
                    }
                } else {
                    // Only update given scope
                    QVariant var = read(di.key());
                    if (var.isValid()) setValue(di.key(), var);
                }

            } else if (scope == scAll || scope == dat.scope) {
                // --- common update of entry
                QVariant var = read(di.key());
                if (var.isValid()) setValue(di.key(), var);
            }
        }
    }
//    Scheme::instance()->initDefault();


    // the location for user model libraries is not modifyable right now
    // anyhow, it is part of StudioSettings since it might become modifyable in the future
    setString(skUserModelLibraryDir, CommonPaths::userModelLibraryDir());
}

void Settings::importSettings(const QString &path)
{
    if (!mSettings.value(scUser)) return;
    QFile backupFile(path);
    QFile settingsFile(mSettings.value(scUser)->fileName());
    settingsFile.remove(); // remove old file
    backupFile.copy(settingsFile.fileName()); // import new file
    reload();
    load(scUser);
}

void Settings::exportSettings(const QString &path)
{
    if (!mSettings.value(scUser)) return;
    QFile originFile(mSettings.value(scUser)->fileName());
    originFile.copy(path);
}

}
}
