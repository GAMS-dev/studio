#include "filemeta.h"
#include "filemetarepo.h"
#include "filetype.h"
#include "editors/codeeditor.h"
#include "exception.h"
#include "studiosettings.h"
#include "commonpaths.h"
//#include "lxiviewer/lxiviewer.h"
#include <QFileInfo>
#include <QPlainTextDocumentLayout>
#include <QTextCodec>
#include <QScrollBar>

namespace gams {
namespace studio {

FileMeta::FileMeta(FileMetaRepo *fileRepo, FileId id, QString location)
    : mId(id), mFileRepo(fileRepo), mLocation(location), mData(Data(location))
{
    if (!mFileRepo) EXCEPT() << "FileMetaRepo  must not be null";
    bool symbolic = mLocation.startsWith('[');
    int braceEnd = mLocation.indexOf(']');
    if (braceEnd <= 0) braceEnd = mLocation.size();
    mName = symbolic ? mLocation.left(braceEnd) : QFileInfo(mLocation).fileName();
    mDocument = new QTextDocument(this);
    mDocument->setDocumentLayout(new QPlainTextDocumentLayout(mDocument));
    mDocument->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

inline FileId FileMeta::id() const
{
    return mId;
}

QString FileMeta::location() const
{
    return mLocation;
}

FileKind FileMeta::kind()
{
    return mData.type->kind();
}

QString FileMeta::name()
{
    return mName;
}

QWidgetList FileMeta::editors() const
{
    return mEditors;
}

QWidget *FileMeta::topEditor() const
{
    return isOpen() ? mEditors.first() : nullptr;
}

void FileMeta::modificationChanged(bool modiState)
{
    Q_UNUSED(modiState);
    emit changed(id());
}

void FileMeta::updateMarks()
{
    // This gathers additional error information from lst-content

    // TODO(JM) Perform a large-file-test if this should have an own thread

//    if (!mMarks) return;
//    mMarks->updateMarks();
//    if (mMarksEnhanced) return;
//    QRegularExpression rex("\\*{4}((\\s+)\\$([0-9,]+)(.*)|\\s{1,3}([0-9]{1,3})\\s+(.*)|\\s\\s+(.*)|\\s(.*))");
//    if (mMetrics.fileType() == FileType::Lst && document()) {
//        for (TextMark* mark: mMarks->marks()) {
//            QList<int> errNrs;
//            int lineNr = mark->line();
//            QTextBlock block = document()->findBlockByNumber(lineNr).next();
//            QStringList errText;
//            while (block.isValid()) {
//                QRegularExpressionMatch match = rex.match(block.text());
//                if (!match.hasMatch()) break;
//                if (match.capturedLength(3)) { // first line with error numbers and indent
//                    for (QString nrText: match.captured(3).split(",")) errNrs << nrText.toInt();
//                    if (match.capturedLength(4)) errText << match.captured(4);
//                } else if (match.capturedLength(5)) { // line with error number and description
//                    errText << match.captured(5)+"\t"+match.captured(6);
//                } else if (match.capturedLength(7)) { // indented follow-up line for error description
//                    errText << "\t"+match.captured(7);
//                } else if (match.capturedLength(8)) { // non-indented line for additional error description
//                    errText << match.captured(8);
//                }
//                block = block.next();
//            }
//            parentEntry()->setLstErrorText(lineNr, errText.join("\n"));
//        }
//        mMarksEnhanced = true;
//    }

}

void FileMeta::addEditor(QWidget *edit)
{
    if (!edit) return;
    if (mEditors.contains(edit)) {
        mEditors.move(mEditors.indexOf(edit), 0);
        return;
    }
    if (editorType(edit) == EditorType::undefined)
        EXCEPT() << "Type assignment missing for this editor/viewer";
    bool newlyOpen = !document();
    mEditors.prepend(edit);
    AbstractEditor* ptEdit = toAbstractEdit(edit);
    CodeEditor* scEdit = toCodeEdit(edit);

    if (mEditors.size() == 1) {
        if (ptEdit) {
            ptEdit->document()->setParent(this);
            connect(document(), &QTextDocument::modificationChanged, this, &FileMeta::modificationChanged, Qt::UniqueConnection);
            if (mHighlighter && mHighlighter->document() != document()) {
                mHighlighter->setDocument(document());
                if (scEdit) connect(scEdit, &CodeEditor::requestSyntaxState, mHighlighter, &ErrorHighlighter::syntaxState);
            }
            if (newlyOpen) emit documentOpened();
            QTimer::singleShot(50, this, &FileMeta::updateMarks);
        }
    } else if (ptEdit) {
        ptEdit->setDocument(document());
    }
    // TODO(JM) getMouseMove and -click for editor to enable link-clicking
    if (ptEdit) {
        if (!ptEdit->viewport()->hasMouseTracking()) {
            ptEdit->viewport()->setMouseTracking(true);
        }
        ptEdit->viewport()->installEventFilter(this);
        ptEdit->installEventFilter(this);
    }
//    if (scEdit && mMarks) {
//        // TODO(JM) Should be bound directly to a sublist in TextMarkRepo
//        connect(scEdit, &CodeEditor::requestMarkHash, this, &ProjectFileNode::shareMarkHash);
//        connect(scEdit, &CodeEditor::requestMarksEmpty, this, &ProjectFileNode::textMarkIconsEmpty);
//        connect(scEdit->document(), &QTextDocument::contentsChange, scEdit, &CodeEditor::afterContentsChanged);
//    }
}

void FileMeta::editToTop(QWidget *edit)
{
    addEditor(edit);
}

void FileMeta::removeEditor(QWidget *edit)
{
    int i = mEditors.indexOf(edit);
    if (i < 0)
        return;
    bool wasModified = isModified();
    AbstractEditor* ptEdit = toAbstractEdit(edit);
    CodeEditor* scEdit = toCodeEdit(edit);

    if (ptEdit && mEditors.size() == 1) {
        emit documentClosed();
        // On removing last editor: paste document-parency back to editor
        ptEdit->document()->setParent(ptEdit);
        disconnect(ptEdit->document(), &QTextDocument::modificationChanged, this, &FileMeta::modificationChanged);
    }
    mEditors.removeAt(i);
    if (mEditors.isEmpty()) {
        if (!document()) emit documentClosed();
        if (wasModified) emit changed(id());
    } else if (ptEdit) {
        ptEdit->setDocument(document()->clone(ptEdit));
    }
    if (ptEdit) {
        ptEdit->viewport()->removeEventFilter(this);
        ptEdit->removeEventFilter(this);
    }
    if (scEdit && mHighlighter) {
        disconnect(scEdit, &CodeEditor::requestSyntaxState, mHighlighter, &ErrorHighlighter::syntaxState);
    }
}

void FileMeta::removeAllEditors()
{
    auto editors = mEditors;
    for (auto editor : editors) {
        removeEditor(editor);
    }
    mEditors = editors;
}

bool FileMeta::hasEditor(QWidget *edit)
{
    return mEditors.contains(edit);
}

void FileMeta::jumpTo(const QTextCursor &cursor, FileId runId, bool focus, int altLine, int altColumn)
{
    emit openFile(this, focus, runId, codecMib());
    if (mEditors.size()) {
        AbstractEditor* edit = toAbstractEdit(mEditors.first());
        if (!edit) return;

        QTextCursor tc;
        if (cursor.isNull()) {
            if (edit->document()->blockCount()-1 < altLine) return;
            tc = QTextCursor(edit->document()->findBlockByNumber(altLine));
        } else {
            tc = cursor;
        }

        if (cursor.isNull()) tc.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, altColumn);
        tc.clearSelection();
        edit->setTextCursor(tc);
        // center line vertically
        qreal lines = qreal(edit->rect().height()) / edit->cursorRect().height();
        qreal line = qreal(edit->cursorRect().bottom()) / edit->cursorRect().height();
        int mv = line - lines/2;
        if (qAbs(mv) > lines/3)
            edit->verticalScrollBar()->setValue(edit->verticalScrollBar()->value()+mv);
    }
}

bool FileMeta::isModified() const
{
    return mDocument->isModified();
}

QTextDocument *FileMeta::document() const
{
    return mDocument;
}

int FileMeta::codecMib() const
{
    return mCodec ? mCodec->mibEnum() : -1;
}

bool FileMeta::exists() const
{
    return mData.exist;
}

bool FileMeta::isOpen() const
{
    return !mEditors.isEmpty();
}

QWidget* FileMeta::createEdit(QTabWidget *tabWidget, bool focus, ProjectRunGroupNode *runGroup, int codecMip)
{
    QWidget* res = nullptr;
    if (kind() != FileKind::Gdx) {
        CodeEditor *codeEdit = new CodeEditor(mFileRepo->settings(), tabWidget);
        codeEdit->setTabChangesFocus(false);
        codeEdit->setRunFileId(runGroup ? runGroup->runFileId() : -1);
        initEditorType(codeEdit);
        codeEdit->setFont(QFont(mFileRepo->settings()->fontFamily(), mFileRepo->settings()->fontSize()));
        QFontMetrics metric(codeEdit->font());
        codeEdit->setTabStopDistance(8*metric.width(' '));
        res = codeEdit;
        if (kind() == FileKind::Lst) {
            lxiviewer::LxiViewer* lxiViewer = new lxiviewer::LxiViewer(codeEdit, this, runGroup, tabWidget);
            initEditorType(lxiViewer);
            res = lxiViewer;
        }

        if (codecMip == -1)
            load(encodingMIBs(), true);
        else
            load(codecMip, true);

        if (kind() == FileKind::Log ||
                kind() == FileKind::Lst ||
                kind() == FileKind::Ref) {

            codeEdit->setReadOnly(true);
            codeEdit->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        }
    } else {
        gdxviewer::GdxViewer* gdxView = new gdxviewer::GdxViewer(location(), CommonPaths::systemDir(), tabWidget);
        initEditorType(gdxView);
        res = gdxView;
        addFileWatcherForGdx();
    }
    addEditor(res);
    return res;
}

FileMeta::Data::Data(QString location)
{
    if (location.startsWith('[')) {
        int len = location.indexOf(']')-2;
        type = (len > 0) ? &FileType::from(location.mid(1, len)) : &FileType::from("");
    } else {
        QFileInfo fi(location);
        exist = fi.exists();
        size = fi.size();
        created = fi.created();
        modified = fi.lastModified();
        type = &FileType::from(fi.suffix());
    }
}

} // namespace studio
} // namespace gams
