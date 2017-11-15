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
#include "textmark.h"

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
    };

    ~FileContext() override;

    /// The name of the current codec for this file.
    /// \return The name of the codec.
    QString codec() const;

    /// Changes the codec for this file.
    /// \param codec The name of the new codec.
    void setCodec(const QString& codec);

    /// The caption of this file, which is its extended display name.
    /// \return The caption of this node.
    virtual const QString caption();

    bool isModified();

    /// Sets a new location (name and path) to the node. This sets the CRUD-state to "Create"
    /// \param location The new location
    void setLocation(const QString &_location);

    /// The icon for this file type.
    /// \return The icon for this file type.
    QIcon icon();

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
    const QList<QPlainTextEdit*> editors() const;

    /// Assigns a <c>CodeEditor</c> to this file. All editors assigned to a <c>FileContext</c> share the same
    /// <c>QTextDocument</c>. If the editor is already assigned it is moved to top.
    /// \param edit The additional <c>CodeEditor</c>
    void addEditor(QPlainTextEdit *edit);
    void addEditor(CodeEditor *edit);

    /// Moves the <c>CodeEditor</c> to the top of the editors-list of this file. (same behavior as <c>addEditor()</c>)
    /// \param edit The <c>CodeEditor</c> to be moved to top.
    void editToTop(QPlainTextEdit *edit);

    /// Removes an <c>CodeEditor</c> from the list.
    /// \param edit The <c>CodeEditor</c> to be removed.
    void removeEditor(QPlainTextEdit *edit);

    /// Removes all <c>CodeEditor</c>s from the list.
    /// \param edit The <c>CodeEditor</c> to be removed.
    void removeAllEditors();

    /// Tests, if a <c>QPlainTextEdit</c> is assigned to this <c>FileContext</c>.
    /// \param edit The <c>QPlainTextEdit</c> to be find.
    /// \return TRUE, if a <c>QPlainTextEdit</c> is assigned to this <c>FileContext</c>.
    bool hasEditor(QPlainTextEdit* edit);

    /// The current QTextDocument assigned to this file.
    /// \return The current QTextDocument
    QTextDocument* document();

    /// If set to TRUE, the document is kept even if the last editor is closed.
    /// \param keep Sets whether to keep the document.
    void setKeepDocument(bool keep = true);

    const FileMetrics& metrics();
    void jumpTo(const QTextCursor& cursor);
    void showToolTip(const TextMark& mark);

    void markOld();

signals:
    /// Signal is emitted when the file has been modified externally.
    /// \param fileId The file identifier
    void modifiedExtern(int fileId);

    /// Signal is emitted when the file has been deleted (or renamed) externally.
    /// \param fileId The file identifier
    void deletedExtern(int fileId);

    void requestContext(const QString &filePath, FileContext *&fileContext, FileGroupContext *group = nullptr);

    void findFileContext(QString filePath, FileContext** fileContext, FileGroupContext* fileGroup = nullptr);
    void createErrorHint(const int errCode, const QString &errText);
    void requestErrorHint(const int errCode, QString &errText);
    void openOrShow(FileContext* fileContext);
    void clearLineIcons();
    void setLineIcon(int line, const QIcon& icon);

public slots:
    void clearRecentMarks();
    void addProcessData(QProcess::ProcessChannel channel, QString text);

protected slots:
    void onFileChangedExtern(QString filepath);

    /// Slot to handle a change of the assigned Document
    void modificationChanged(bool modiState);

    void updateMarks();

protected:
    struct LinkData {
        TextMark* textMark = nullptr;
        int col = 0;
        int size = 1;
    };

    friend class FileRepository;
    FileContext(int id, QString name, QString location);

    QString extractError(QString text, ExtractionState &state, QList<LinkData>& marks);
    TextMark* generateTextMark(gams::studio::TextMark::Type tmType, int value, int line, int column, int size = 0);
    void markLink(TextMark* mark);
    void removeTextMarks(TextMark::Type tmType);
    void removeTextMarks(QSet<TextMark::Type> tmTypes);
    bool eventFilter(QObject *watched, QEvent *event);
    bool mouseOverLink();

private:
    FileMetrics mMetrics;
    QString mCodec = "UTF-8";
    FileContext *mLinkFile = nullptr;
    QList<QPlainTextEdit*> mEditors;
    QFileSystemWatcher *mWatcher = nullptr;
    QTextDocument *mDocument;
    bool mBeforeErrorExtraction = true;
    QSet<FileContext*> mMarkedContextList;
    QHash<int, TextMark*> mTextMarks;
    QPair<int, QString> mCurrentErrorHint;
    bool mMouseOverLink = false;

    static const QStringList mDefaulsCodecs;

};

} // namespace studio
} // namespace gams

#endif // FILECONTEXT_H
