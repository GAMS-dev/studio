/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include <QtWidgets>
#include "filesystemcontext.h"
#include "filemetrics.h"
#include "syntax.h"
#include "gdxviewer/gdxviewer.h"

namespace gams {
namespace studio {

class CodeEditor;
class FileGroupContext;
class TextMark;
typedef QPair<int,QString> ErrorHint;

///
/// The <c>FileContext</c> class represents context data for a text-file. It is derived from <c>FileSystemContext</c>.
/// \see FileSystemContext, FileGroupContext, FileActionContext
///
class FileContext : public FileSystemContext
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

    ~FileContext() override;

    void setParentEntry(FileGroupContext *parent) override;

    /// The name of the current codec for this file.
    /// \return The name of the codec.
    QString codec() const;

    /// Changes the codec for this file.
    /// \param codec The name of the new codec.
    void setCodec(const QString& codec);

    /// The caption of this file, which is its extended display name.
    /// \return The caption of this node.
    virtual const QString caption() override;

    bool isModified();

    /// Sets a new location (name and path) to the node. This sets the CRUD-state to "Create"
    /// \param location The new location
    void setLocation(const QString &_location) override;

    /// The icon for this file type.
    /// \return The icon for this file type.
    QIcon icon() override;

    /// Saves the file, if it is changed.
    void save();

    /// Saves the file to a new location.
    /// \param filePath new location for file
    void save(QString filePath);

    /// Loads the file into the current QTextDocument.
    /// \param codecName The text-codec to use.
    void load(QString codecName = QString());

    /// Gets the list of assigned editors.
    /// \return The list of assigned editors.
    const QWidgetList editors() const;

    /// Assigns a <c>CodeEditor</c> to this file. All editors assigned to a <c>FileContext</c> share the same
    /// <c>QTextDocument</c>. If the editor is already assigned it is moved to top.
    /// \param edit The additional <c>CodeEditor</c>
    virtual void addEditor(QWidget* edit);

    /// Moves the <c>CodeEditor</c> to the top of the editors-list of this file. (same behavior as <c>addEditor()</c>)
    /// \param edit The <c>CodeEditor</c> to be moved to top.
    void editToTop(QWidget* edit);

    /// Removes an <c>CodeEditor</c> from the list.
    /// \param edit The <c>CodeEditor</c> to be removed.
    virtual void removeEditor(QWidget* edit);

    /// Removes all <c>CodeEditor</c>s from the list.
    /// \param edit The <c>CodeEditor</c> to be removed.
    void removeAllEditors();

    /// Tests, if a <c>QPlainTextEdit</c> is assigned to this <c>FileContext</c>.
    /// \param edit The <c>QPlainTextEdit</c> to be find.
    /// \return TRUE, if a <c>QPlainTextEdit</c> is assigned to this <c>FileContext</c>.
    bool hasEditor(QWidget* edit);

    /// The current QTextDocument assigned to this file.
    /// \return The current QTextDocument
    virtual QTextDocument* document();

    const FileMetrics& metrics() const;
    void jumpTo(const QTextCursor& cursor, bool focus, int altLine = 0, int altColumn = 0);
    void showToolTip(const QList<TextMark*> marks);

    void rehighlightAt(int pos);
    void updateMarks();
    inline void clearMarksEnhanced() {mMarksEnhanced = false;}
    TextMark* generateTextMark(TextMark::Type tmType, int value, int line, int column, int size = 0);
    TextMark* generateTextMark(QString fileName, TextMark::Type tmType, int value, int line, int column, int size = 0);
    int textMarkCount(QSet<TextMark::Type> tmTypes);
    ErrorHighlighter* highlighter();

    void removeTextMarks(TextMark::Type tmType);
    void removeTextMarks(QSet<TextMark::Type> tmTypes);

signals:
    /// Signal is emitted when the file has been modified externally.
    /// \param fileId The file identifier
    void modifiedExtern(FileId fileId);

    /// Signal is emitted when the file has been deleted (or renamed) externally.
    /// \param fileId The file identifier
    void deletedExtern(FileId fileId);

    void findFileContext(QString filePath, FileContext** fileContext, FileGroupContext* fileGroup = nullptr);
    void findOrCreateFileContext(QString filePath, FileContext** fileContext, FileGroupContext* fileGroup = nullptr);
    void openFileContext(FileContext* fileContext, bool focus = true);

protected slots:
    void onFileChangedExtern(QString filepath);
    /// Slot to handle a change of the assigned Document
    void modificationChanged(bool modiState);

protected:
    friend class LogContext;
    friend class FileRepository;
    FileContext(FileId fileId, QString name, QString location, ContextType type = FileSystemContext::File);

    QWidgetList& editorList();
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool mouseOverLink();

private:
    FileMetrics mMetrics;
    QString mCodec = "UTF-8";
    FileContext *mLinkFile = nullptr;
    QWidgetList mEditors;
    QFileSystemWatcher *mWatcher = nullptr;
    QList<TextMark*> mMarksAtMouse;
    QPoint mClickPos;
    TextMarkList *mMarks = nullptr;
    ErrorHighlighter* mSyntaxHighlighter = nullptr;
    bool mMarksEnhanced = true;

    static const QStringList mDefaulsCodecs;

};

} // namespace studio
} // namespace gams

#endif // FILECONTEXT_H
