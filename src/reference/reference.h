/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#ifndef REFERENCE_H
#define REFERENCE_H

#include <QString>
#include <QMap>
#include <QDir>

//#include "referencedatatype.h"
#include "filereferenceitem.h"
#include "symbolreferenceitem.h"

namespace gams {
namespace studio {
namespace reference {

/// \brief The Reference class is used to parse the GAMS reference file and
///        store reference file information.
class Reference : public QObject
{
    Q_OBJECT
public:
    /// \brief This enum describes the different states of Reference object.
    enum ReferenceState {
        Initializing, ///< The Reference object is initializing.
        Loading,      ///< The Reference object is loading from the reference file.
        SuccessfullyLoaded,       ///< The Reference object has been successfully loaded from the reference file.
        UnsuccessfullyLoaded      ///< The Reference object has NOT been successfully loaded from the reference file.
    };
    Q_ENUM(ReferenceState)

    ///
    /// \brief Constructs a Reference object with the given reference file and parent.
    /// \param referenceFile the absolute file path of the reference file.
    /// \param parent the parent object.
    ///
    Reference(const QString &referenceFile, const QString &encodingName, QObject* parent = Q_NULLPTR);

    ///
    /// \brief Destructs the Reference object, i.e., cleaning up internal memory.
    ///
    ~Reference();

    ///
    /// \brief Find the SymbolReferenceItem of the symbol by type.
    /// \param type Symbol type.
    /// \return <c>List of SymbolReferenceItem of the symbols of the given type</c> if found;
    ///         otherwise <c>List of SymbolReferenceItem of all symbols</c>.
    ///
    QList<SymbolReferenceItem *> findReferenceFromType(SymbolDataType::SymbolType type);

    ///
    /// \brief Find the SymbolReferenceItem of the symbol by id.
    ///        Found if there is no problem parsing the reference file and
    ///        symbol of the given id exists in the reference object.
    /// \param id Symbol id.
    /// \return <c>SymbolReferenceItem of the symbol</c> if found; otherwise <c>nullptr</c>.
    ///
    SymbolReferenceItem* findReferenceFromId(SymbolId symbolid);

    ///
    /// \brief Find the SymbolReferenceItem of the symbol by name.
    ///        Found if there is no problem parsing the reference file and
    ///        symbol of the given name exists in the reference object.
    /// \param symbolName Symbol name.
    /// \return <c>SymbolReferenceItem of the symbol</c> if found; otherwise <c>nullptr</c>.
    ///
    SymbolReferenceItem* findReferenceFromName(const QString &symbolName);

    ///
    /// \brief Get the FileReferenceItem by id.
    /// \param File reference id.
    /// \return <c>FileReferenceItem.
    ///
    FileReferenceItem* getFileUsedReference(FileReferenceId id);

    ///
    /// \brief Get the number of file reference in the reference object.
    /// \return Returns the number of file references.
    int getNumberOfFileUsed() const;

    ///
    /// \brief Checks if symbol of the given id exists in the reference object.
    /// \param id Symbol id.
    /// \return <c>true</c> if the the reference object contains a symbol of the given id; otherwise <c>false</c>.
    ///
    bool contains(SymbolId id) const;

    ///
    /// \brief Checks if symbol of the given name exists in the reference object.
    /// \param symbolName Symbol name.
    /// \return <c>true</c> if the the reference object contains a symbol of the given name; otherwise <c>false</c>.
    ///
    bool contains(const QString &symbolName) const;

    ///
    /// \brief Checks if there is no symbol in the reference object.
    /// \return <c>true</c> if there is no symbol in the reference object; otherwise <c>false</c>.
    ///
    bool isEmpty() const;

    ///
    /// \brief Checks if there is a problem parsing the reference file.
    /// \return <c>true</c> if there is no problem parsing the reference file; otherwise <c>false</c>.
    ///
    bool isValid() const;

    ///
    /// \brief Get the number of symbols in the reference object.
    /// \return Returns the number of symbols.
    ///
    int size() const;

    ///
    /// \brief Get the list of all files used in the reference object.
    /// \return Returns the list of all files used in the reference object.
    ///
    QStringList getFileUsed() const;

    ///
    /// \brief Get the list of all symbol ids.
    /// \return Returns the list of all symbol ids.
    ///
    QList<SymbolId> symbolIDList() const;

    ///
    /// \brief Get the list of all symbol names.
    /// \return Returns the list of all symbol names.
    ///
    QList<QString> symbolNameList() const;

    ///
    /// \brief Get location of the reference file.
    /// \return Returns location of the reference file.
    ///
    QString getFileLocation() const;

    ///
    /// \brief Get state of the reference object.
    /// \return Returns state of the reference object.
    /// \see ::ReferenceState
    ///
    Reference::ReferenceState state() const;

    ///
    /// \brief Get the line number last read when encountering an error
    /// \return Returns the line number last read if there is a read error, -1 otherwise.
    ///
    int errorLine() const;

signals:
    ///
    /// \brief Signal emitted when loading the reference file has just been started.
    ///
    void loadStarted();

    ///
    /// \brief Signal emitted when loading the reference file has just been finished.
    /// \param loadStatus Finish load status.
    /// \param pendingRelaod if reload is pending.
    /// \see ::LoadStatus
    ///
    void loadFinished(bool loadStatus, bool pendignReload);

public slots:
    ///
    /// \brief Load the reference object from the reference file.
    /// \param encodingName pointer to codec to be loaded
    /// \param triggerReload True, if reloading. False, otherwise
    ///
    void loadReferenceFile(const QString &encodingName, bool triggerReload=true);

private:
    qint64 parseFile(const QString &referenceFile, const QString &encodingName);
    void addReferenceInfo(SymbolReferenceItem* ref, const QString &referenceType, int lineNumber, int columnNumber, const QString &location);
    void addSymbolReferenceItem();
    void clear();

    QString mReferenceFile;
    ReferenceState mState = Initializing;
    int mLastErrorLine = -1;
    qint64 mLastReadFileSize = 0;
    bool mPendingReload = false;

    QStringList mFileUsed;

    QList<SymbolReferenceItem *> mSetReference;
    QList<SymbolReferenceItem *> mAcronymReference;
    QList<SymbolReferenceItem *> mParReference;
    QList<SymbolReferenceItem *> mVarReference;
    QList<SymbolReferenceItem *> mEquReference;
    QList<SymbolReferenceItem *> mFileReference;
    QList<SymbolReferenceItem *> mModelReference;
    QList<SymbolReferenceItem *> mFunctionReference;
    QList<SymbolReferenceItem *> mMacroReference;
    QList<SymbolReferenceItem *> mUnusedReference;

    QMap<QString, SymbolId> mSymbolNameMap;
    QMap<FileReferenceId, FileReferenceItem*> mFileUsedReference;
    QMap<SymbolId, SymbolReferenceItem*> mReference;

    static QRegularExpression mRexSplit;
};

} // namespace reference
} // namespace studio
} // namespace gams

#endif // REFERENCE_H
