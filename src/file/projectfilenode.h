/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
#ifndef PROJECTFILENODE_H
#define PROJECTFILENODE_H

#include <QFileSystemWatcher>
#include "projectabstractnode.h"
#include "filemetrics.h"
#include "syntax.h"

namespace gams {
namespace studio {

class CodeEdit;
class ProjectGroupNode;
class TextMark;
class FileMeta;
typedef QPair<int,QString> ErrorHint;

///
/// The <c>ProjectFileNode</c> class represents a file. It is derived from <c>ProjectAbstractNode</c>.
/// \see ProjectAbstractNode, ProjectGroupNode, ProjectLogNode
///
class ProjectFileNode : public ProjectAbstractNode
{
    Q_OBJECT
public:
    enum ExtractionState {
        Outside,
        Entering,
        Inside,
        Exiting,
        FollowupError,
    };

    ~ProjectFileNode() override;


    /// The icon for this file type.
    /// \return The icon for this file type.
    QIcon icon() override;

    /// The caption of this file, which is its extended display name.
    /// \return The caption of this node.
    QString name(NameModifier mod = NameModifier::raw) const override;
    bool isModified() const;
    QTextDocument* document() const;
    FileMeta* file() const;
    QString location() const;
    QString tooltip() override;
    FileId runFileId() const;

signals:
    void changed();

protected:
    friend class ProjectLogNode;
    friend class ProjectRepo;
    friend class FileMeta;

    ProjectFileNode(FileMeta* fileMeta, ProjectGroupNode *group = nullptr, NodeType type = NodeType::file);
    void setParentNode(ProjectGroupNode *parent) override;
    void replaceFile(FileMeta* fileMeta);
//    void showToolTip(const QVector<TextMark*> marks);

private:
    FileMeta* mFileMeta;
    ErrorHighlighter* mSyntaxHighlighter = nullptr;

//    void setParentEntry(ProjectGroupNode *parent) override;

//    /// The name of the current codec for this file.
//    /// \return The name of the codec.
//    int codecMib() const;

//    /// Changes the codec for this file.
//    /// \param codec The name of the new codec.
//    void setCodecMib(int mib);

//    /// Sets a new location (name and path) to the node. This sets the CRUD-state to "Create"
//    /// \param location The new location
//    void setLocation(const QString &_location) override;

//    /// Saves the file, if it is changed.
//    void save();

//    /// Saves the file to a new location.
//    /// \param filePath new location for file
//    void save(QString filePath);


//    /// Loads the file into the current QTextDocument.
//    /// \param codecMibs The codec-MIBs to try for loading the file.
//    /// \param keepMarks true, if the TextMarks should be kept.
//    void load(QList<int> codecMibs = QList<int>(), bool keepMarks = false);
//    void load(int codecMib, bool keepMarks = false);

//    /// Gets the list of assigned editors.
//    /// \return The list of assigned editors.
//    const QWidgetList editors() const;

//    /// Assigns a <c>CodeEditor</c> to this file. All editors assigned to a <c>ProjectFileNode</c> share the same
//    /// <c>QTextDocument</c>. If the editor is already assigned it is moved to top.
//    /// \param edit The additional <c>CodeEditor</c>
//    virtual void addEditor(QWidget* edit);

//    /// Moves the <c>CodeEditor</c> to the top of the editors-list of this file. (same behavior as <c>addEditor()</c>)
//    /// \param edit The <c>CodeEditor</c> to be moved to top.
//    void editToTop(QWidget* edit);

//    /// Removes an <c>CodeEditor</c> from the list.
//    /// \param edit The <c>CodeEditor</c> to be removed.
//    virtual void removeEditor(QWidget* edit);

//    /// Removes all <c>CodeEditor</c>s from the list.
//    /// \param edit The <c>CodeEditor</c> to be removed.
//    void removeAllEditors();

//    /// Tests, if a <c>QWidget</c> is assigned to this <c>FileContext</c>.
//    /// \param edit The <c>QWidget</c> to be find.
//    /// \return TRUE, if a <c>QWidget</c> is assigned to this <c>FileContext</c>.
//    bool hasEditor(QWidget* edit);

//    /// The current QTextDocument assigned to this file.
//    /// \return The current QTextDocument
//    virtual QTextDocument* document() const;

//    bool isReadOnly();

//    const FileMetrics& metrics() const;
//    void jumpTo(const QTextCursor& cursor, bool focus, int altLine = 0, int altColumn = 0);

//    void rehighlightBlock(QTextBlock block, QTextBlock endBlock = QTextBlock());
//    void updateMarks();
//    inline void clearMarksEnhanced() {mMarksEnhanced = false;}
//    TextMark* generateTextMark(TextMark::Type tmType, int value, int line, int column, int size = 0);
//    TextMark* generateTextMark(QString fileName, TextMark::Type tmType, int value, int line, int column, int size = 0);
//    int textMarkCount(QSet<TextMark::Type> tmTypes);
//    ErrorHighlighter* highlighter();

//    void removeTextMarks(TextMark::Type tmType, bool rehighlight = true);
//    void removeTextMarks(QSet<TextMark::Type> tmTypes, bool rehighlight = true);
//    void addFileWatcherForGdx();
//    void unwatch();

//    // TODO(JM) move handling to FileMetaRepo
//    TextMarkRepo* marks() const { return mMarks; }
//    void unbindMarks() { mMarks = nullptr; }


//signals:
//    /// Signal is emitted when the file has been modified externally.
//    /// \param fileId The file identifier
//    void modifiedExtern(FileId fileId);

//    /// Signal is emitted when the file has been deleted (or renamed) externally.
//    /// \param fileId The file identifier
//    void deletedExtern(FileId fileId);

//    void findFileNode(QString filePath, ProjectFileNode** fileContext, ProjectGroupNode* fileGroup = nullptr);
//    void findOrCreateFileNode(QString filePath, ProjectFileNode*& fileContext, ProjectGroupNode* fileGroup = nullptr);
//    void openFileNode(ProjectFileNode* fileContext, bool focus = true, int codecMib = -1);
//    void documentOpened();
//    void documentClosed();

//protected slots:
//    void onFileChangedExtern(QString filepath);
//    /// Slot to handle a change of the assigned Document
//    void modificationChanged(bool modiState);
//    void shareMarkHash(QHash<int, TextMark*>* marks, TextMark::Type filter);
//    void textMarkIconsEmpty(bool* marksEmpty);

//protected:
//    friend class ProjectLogNode;
//    friend class ProjectRepo;

//    QWidgetList& editorList();
//    bool eventFilter(QObject *watched, QEvent *event) override;
//    bool mouseOverLink();

//private:
//    QVector<QPoint> getEditPositions();
//    void setEditPositions(QVector<QPoint> edPositions);

//private:
//    QTextCodec *mCodec = nullptr;
//    ProjectFileNode *mLinkFile = nullptr;
//    QWidgetList mEditors;
//    QFileSystemWatcher *mWatcher = nullptr;
//    QVector<TextMark*> mMarksAtMouse;
//    QPoint mClickPos;
//    TextMarkRepo *mMarks = nullptr;

//    bool mMarksEnhanced = true;

};

} // namespace studio
} // namespace gams

#endif // PROJECTFILENODE_H
