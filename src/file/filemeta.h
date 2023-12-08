/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
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
#ifndef FILEMETA_H
#define FILEMETA_H

#include <QWidget>
#include <QDateTime>
#include <QTextDocument>
#include <QTableView>
#include "syntax.h"
#include "editors/codeedit.h"
#include "editors/textview.h"
#include "gdxviewer/gdxviewer.h"
#include "lxiviewer/lxiviewer.h"
#include "reference/referenceviewer.h"

class QTabWidget;

namespace gams {
namespace studio {

class PExProjectNode;

class FileMeta: public QObject
{
    Q_OBJECT
public:
    enum FileDifference {
        FdEqual = 0x00,
        FdMissing = 0x01,
        FdTime = 0x02,
        FdSize = 0x04,
        FdName = 0x08,
        FdType = 0x10,
    };
    Q_ENUM(FileDifference)
    typedef QFlags<FileDifference> FileDifferences;

public:
    ~FileMeta() override;
    FileId id() const;
    QString location() const;
    QStringList suffix() const;
    void setKind(FileKind kind);
    FileKind kind() const;
    QString kindAsStr() const;
    QString name(NameModifier mod = NameModifier::raw);
    FontGroup fontGroup();
    QTextDocument* document() const;
    int codecMib() const;
    void setCodecMib(int mib);
    QTextCodec *codec() const;
    void setCodec(QTextCodec *codec);
    bool exists(bool ckeckNow = false) const;
    bool isOpen() const;
    bool isModified() const;
    bool isReadOnly() const;
    bool isAutoReload() const;
    void setAutoReload();
    void resetTempReloadState();
    void setModified(bool modified=true);
    bool isPinnable();
    void updateTabName(QTabWidget *tabWidget, int index);
    void updateBreakpoints();

    QWidget *createEdit(QWidget *parent, PExProjectNode *project, int codecMib = -1, bool forcedAsTextEdit = false);
    int addToTab(QTabWidget *tabWidget, QWidget *edit, int codecMib = -1, NewTabStrategy tabStrategy = tabAfterCurrent);
    const QWidgetList editors() const;
    QWidget* topEditor() const;
    void editToTop(QWidget* edit);
    void removeEditor(QWidget* edit);
    void deleteEditor(QWidget* edit);
    bool hasEditor(QWidget * const &edit) const;
    void load(int codecMib, bool init = true);
    void save(const QString& newLocation = "");
    void renameToBackup();
    FileDifferences compare(QString fileName = QString());
    bool refreshMetaData();

    void jumpTo(NodeId groupId, bool focus, int line = 0, int column = 0, int length = 0);
    void rehighlight(int line);
    syntax::SyntaxHighlighter *highlighter() const;
    void marksChanged(QSet<int> lines = QSet<int>());
    void reloadDelayed();
    void setLocation(QString location);

    static bool hasExistingFile(QList<QUrl> urls);
    static bool hasExistingFolder(QList<QUrl> urls);
    static QStringList pathList(QList<QUrl> urls);
    void invalidate();
    const NodeId &projectId() const;
    void setProjectId(const NodeId &newProjectId);

public slots:
    void reload();
    void updateView();
    void invalidateTheme();

signals:
    void changed(gams::studio::FileId fileId);
    void modifiedChanged(gams::studio::FileId fileId, bool modified);
    void documentOpened();
    void documentClosed();
    void editableFileSizeCheck(const QFile &file, bool &canOpen);
    void fontChangeRequest(gams::studio::FileMeta *fileMeta, QFont f);
    void projectTabRenamed(gams::studio::FileMeta *fileMeta);
    void adjustBreakpoint(int &line);

protected:
    bool eventFilter(QObject*sender, QEvent* event) override;

private slots:
    void modificationChanged(bool modiState);
    void contentsChange(int from, int charsRemoved, int charsAdded);
    void blockCountChanged(int newBlockCount);
    void updateMarks();
    void zoomRequest(qreal delta);

private:
    struct Data {
        Data(QString location, FileType *knownType = nullptr);
        FileDifferences compare(Data other);
        bool exist = false;
        qint64 size = 0;
        QDateTime created;
        QDateTime modified;
        FileType *type = nullptr;
    };

    friend class FileMetaRepo;
    FileMeta(FileMetaRepo* fileRepo, FileId id, QString location, FileType *knownType = nullptr);
    QVector<QPoint> getEditPositions();
    void setEditPositions(QVector<QPoint> edPositions);
    bool checkActivelySavedAndReset();
    void updateEditsCompleter();
    void linkDocument(QTextDocument *doc = nullptr);
    void unlinkAndFreeDocument();
    void refreshType();
    void updateSyntaxColors();
    void initEditorColors();
    void updateEditorColors();
    void addEditor(QWidget* edit);

private:
    FileId mId;
    NodeId mProjectId;
    FileMetaRepo* mFileRepo;
    QString mLocation;
    QString mName;
    Data mData;
    bool mAutoReload = false;
    bool mActivelySaved = false;
    QWidgetList mEditors;
    QTextCodec *mCodec = nullptr;
    QTextDocument* mDocument = nullptr;
    syntax::SyntaxHighlighter* mHighlighter = nullptr;
    int mLineCount = 0;
    int mChangedLine = 0;
    bool mLoading = false;
    QTimer mTempAutoReloadTimer;
    QTimer mReloadTimer;
    QTimer mDirtyLinesUpdater;
    QSet<int> mDirtyLines;
    QMutex mDirtyLinesMutex;
};

} // namespace studio
} // namespace gams

#endif // FILEMETA_H
