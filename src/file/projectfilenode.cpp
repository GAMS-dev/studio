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
#include "projectfilenode.h"
#include "projectgroupnode.h"
#include "projectrepo.h"
#include "exception.h"
#include "syntax/textmarkrepo.h"
#include "filemeta.h"
#include "editors/codeedit.h"
#include "logger.h"
#include <QScrollBar>
#include <QToolTip>
#include <QTextCodec>

namespace gams {
namespace studio {

ProjectFileNode::ProjectFileNode(FileMeta *fileMeta, ProjectGroupNode* group, NodeType type)
    : ProjectAbstractNode(fileMeta?fileMeta->name():"[NULL]", type), mFileMeta(fileMeta)
{
    if (!mFileMeta) EXCEPT() << "The assigned FileMeta must not be null.";
    if (group) setParentNode(group);
}

ProjectFileNode::~ProjectFileNode()
{}

void ProjectFileNode::setParentNode(ProjectGroupNode *parent)
{
    ProjectAbstractNode::setParentNode(parent);
    // TODO(JM) setRunId in FileMeta
}

QIcon ProjectFileNode::icon()
{
    ProjectGroupNode* par = parentNode();
    while (par && !par->toRunGroup()) par = par->parentNode();
    QString runMark = (par && file() == par->toRunGroup()->runnableGms()) ? "-run" : "";
    if (file()->kind() == FileKind::Gms)
        return QIcon(":/img/gams-w"+runMark);
    if (file()->kind() == FileKind::Gdx)
        return QIcon(":/img/database");
    return QIcon(":/img/file-alt"+runMark);
}

QString ProjectFileNode::name(NameModifier mod) const
{
    QString res = ProjectAbstractNode::name();
    switch (mod) {
    case NameModifier::editState:
        res += (isModified() ? "*" : "");
        break;
    default:
        break;
    }
    return res;
}

bool ProjectFileNode::isModified() const
{
    return mFileMeta->isModified();
}

QTextDocument *ProjectFileNode::document() const
{
    return mFileMeta->document();
}

FileMeta *ProjectFileNode::file() const
{
    return mFileMeta;
}

void ProjectFileNode::replaceFile(FileMeta *fileMeta)
{
    if (mFileMeta != fileMeta) {
        mFileMeta = fileMeta;
        emit changed(id());
    }
}

QString ProjectFileNode::location() const
{
    return mFileMeta->location();
}

QString ProjectFileNode::tooltip()
{
    return location();
}

FileId ProjectFileNode::runFileId() const
{
    ProjectGroupNode* group = parentNode();
    while (group && group->type() != NodeType::runGroup)
        group = group->parentNode();
    if (group && group->type() != NodeType::runGroup)
        return group->toRunGroup()->runFileId();
    return -1;
}

// TODO(JM) Refactor: TextMarks now have their own repository
void ProjectFileNode::showToolTip(const QVector<TextMark*> marks)
{
    if (mEditors.size() && marks.size() > 0) {
        QTextCursor cursor(marks.first()->textCursor());
        if (cursor.isNull()) return;
        AbstractEdit* edit = ProjectAbstractNode::toAbstractEdit(mEditors.first());
        if (!edit) return;
        cursor.setPosition(cursor.anchor());
        QPoint pos = edit->cursorRect(cursor).bottomLeft();
        QStringList tips;
        for (TextMark* mark: marks) {
            QString newTip = parentEntry()->lstErrorText(mark->value());
            if (!tips.contains(newTip))
                tips << newTip;
        }
//        QString tip = parentEntry()->lstErrorText(marks.first()->value());
        QToolTip::showText(edit->mapToGlobal(pos), tips.join("\n"), edit);
    }
}

// TODO(JM) enhance mark texts with lst-data
void ProjectFileNode::updateMarks()
{
    // TODO(JM) Perform a large-file-test if this should have an own thread
    if (!mMarks) return;
    mMarks->updateMarks();
    if (mMarksEnhanced) return;
    //                     0     1 2       3 4                    5               6           7       8
    //                            (    $nr               |        nr+description      |  descr.   |  any  )
    QRegularExpression rex("\\*{4}((\\s+)\\$(([0-9,]+).*)|\\s{1,3}([0-9]{1,3})\\s+(.*)|\\s\\s+(.*)|\\s(.*))");
    if (mMetrics.fileType() == FileType::Lst && document()) {
        QVector<int> lines;
        for (TextMark* mark: mMarks->marks()) {
            int lineNr = mark->line();
            if (!lines.contains(lineNr)) lines << lineNr;
        }
        for (int lineNr: lines) {
            QList<int> errNrs;
            QList<int> errTextNr;
            QStringList errNrsDeb;
            QTextBlock block = document()->findBlockByNumber(lineNr).next();
            QStringList errText;
            while (block.isValid()) {
                QRegularExpressionMatch match = rex.match(block.text());
                if (!match.hasMatch()) break;
                if (match.capturedLength(4)) { // first line with error numbers and indent
                    int ind = 0;
                    QString line(match.captured(3));
                    QRegularExpression xpNr("[^0-9]*([0-9]+)");
                    do {
                        QRegularExpressionMatch nrMatch = xpNr.match(line, ind);
                        if (!nrMatch.hasMatch()) break;
                        errNrsDeb << nrMatch.captured(1);
                        errNrs << nrMatch.captured(1).toInt();
                        ind = nrMatch.capturedEnd(1);
                    } while (true);
                } else if (match.capturedLength(5)) { // line with error number and description
                    errText << match.captured(5)+"\t"+match.captured(6);
                    errTextNr << match.captured(5).toInt();
                } else if (match.capturedLength(7)) { // indented follow-up line for error description
                    errText << "\t"+match.captured(7);
                    errTextNr << (errTextNr.isEmpty() ? -1 : errTextNr.last());
                } else if (match.capturedLength(8)) { // non-indented line for additional error description
                    errText << match.captured(8);
                    errTextNr << (errTextNr.isEmpty() ? -1 : errTextNr.last());
                }
                block = block.next();
            }
            QStringList orderedErrText;
            for (int nr = 0; nr < errNrs.size(); ++nr) {
                for (int errLn = 0; errLn < errTextNr.size(); ++errLn) {
                    if (errTextNr.at(errLn) == errNrs.at(nr)) orderedErrText << errText.at(errLn);
                }
            }
            parentEntry()->setLstErrorText(lineNr, orderedErrText.join("\n"));
        }
        mMarksEnhanced = true;
    }
}

// TODO(JM) Refactor: This was linked to the current editor. Move implementation to the AbstractEdit
bool ProjectFileNode::eventFilter(QObject* watched, QEvent* event)
{
    static QSet<QEvent::Type> evCheckMouse {QEvent::MouseButtonDblClick, QEvent::MouseButtonPress, QEvent::MouseButtonRelease, QEvent::MouseMove};
    static QSet<QEvent::Type> evCheckKey {QEvent::KeyPress, QEvent::KeyRelease};

//    if (!mEditors.size() || (watched != mEditors.first() && watched != mEditors.first()->viewport()))
//        return FileSystemContext::eventFilter(watched, event);

    // For events MouseButtonPress, MouseButtonRelease, MouseMove,
    // - when send by viewport -> content
    // - when send by CodeEdit -> lineNumberArea
    // For event ToolTip
    // - always two events occur: one for viewport and one for CodeEdit

    // TODO(JM) use updateLinkDisplay

    AbstractEdit* edit = ProjectFileNode::toAbstractEdit(mEditors.first());
    if (!edit) ProjectAbstractNode::eventFilter(watched, event);

    QMouseEvent* mouseEvent = (evCheckMouse.contains(event->type())) ? static_cast<QMouseEvent*>(event) : nullptr;
    QHelpEvent* helpEvent = (event->type() == QEvent::ToolTip)  ? static_cast<QHelpEvent*>(event) : nullptr;
    QKeyEvent *keyEvent = (evCheckKey.contains(event->type())) ? static_cast<QKeyEvent*>(event) : nullptr;

    if (mMetrics.fileType() == FileType::Log
        && (event->type() == QEvent::MouseButtonDblClick
            || (event->type() == QEvent::MouseButtonRelease && mouseEvent->modifiers()==Qt::ControlModifier)) ) {
        // TODO(JM) ---> to LogEdit
        QPoint pos = mouseEvent->pos();
        QTextCursor cursor = edit->cursorForPosition(pos);
        if (mMarks && (mMarks->marksForBlock(cursor.block(), TextMark::error).isEmpty()
                       || mouseEvent->modifiers()==Qt::ControlModifier)) {
            int line = cursor.blockNumber();
            TextMark* linkMark = nullptr;
            for (TextMark *mark: mMarks->marks()) {
                if (mark->type() == TextMark::link && mark->refFileKind() == FileType::Lst) {
                    if (mark->line() < line)
                        linkMark = mark;
                    else if (!linkMark)
                        linkMark = mark;
                    else if (line+1 < mark->line()+mark->spread())
                        break;
                    else if (qAbs(linkMark->line()-line) < qAbs(line-mark->line()))
                        break;
                    else {
                        linkMark = mark;
                        break;
                    }
                }
            }
            if (linkMark) {
                linkMark->jumpToRefMark(true);
                edit->setFocus();
            }
        }

    } else if (keyEvent) {
//        if (keyEvent->modifiers() & Qt::ControlModifier) {
//            edit->viewport()->setCursor(mMarksAtMouse.isEmpty() ? Qt::IBeamCursor : Qt::PointingHandCursor);
//        } else {
//            edit->viewport()->setCursor(Qt::IBeamCursor);
//        }
//        return ProjectAbstractNode::eventFilter(watched, event);

    } else if (mouseEvent || helpEvent) {
        static QPoint ttPos;

//        QPoint pos = mouseEvent ? mouseEvent->pos() : helpEvent->pos();
//        QTextCursor cursor = edit->cursorForPosition(pos);
//        CodeEdit* codeEdit = ProjectAbstractNode::toCodeEdit(edit);
//        mMarksAtMouse = mMarks ? mMarks->findMarks(cursor) : QVector<TextMark*>();
//        bool isValidLink = false;
//        if (QToolTip::isVisible() && (ttPos-pos).manhattanLength() > 3) {
//            QToolTip::hideText();
//            ttPos = QPoint();
//        }

        if (codeEdit && watched == codeEdit && event->type() != QEvent::ToolTip) {
//            // if in CodeEditors lineNumberArea
//            Qt::CursorShape shape = Qt::ArrowCursor;
//            if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->cursorShape(&shape, true);
//            codeEdit->lineNumberArea()->setCursor(shape);
//            isValidLink = mMarksAtMouse.isEmpty() ? false : mMarksAtMouse.first()->isValidLink(true);
        } else {
//            Qt::CursorShape shape = Qt::IBeamCursor;
//            if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->cursorShape(&shape);
//            edit->viewport()->setCursor(shape);
//            isValidLink = mMarksAtMouse.isEmpty() ? false : mMarksAtMouse.first()->isValidLink();
        }

        if (!mMarksAtMouse.isEmpty() && event->type() == QEvent::MouseButtonPress) {
//            mClickPos = pos;
        } else if (!mMarksAtMouse.isEmpty() && event->type() == QEvent::MouseButtonRelease) {
//            if ((mClickPos-pos).manhattanLength() < 4 && isValidLink) {
//                if (!mMarksAtMouse.isEmpty()) mMarksAtMouse.first()->jumpToRefMark();
//                return !mMarksAtMouse.isEmpty();
//            }
        } else if (event->type() == QEvent::ToolTip) {
            if (!mMarksAtMouse.isEmpty()) showToolTip(mMarksAtMouse);
            ttPos = pos;
            return !mMarksAtMouse.isEmpty();
        }
    }
    return ProjectAbstractNode::eventFilter(watched, event);
}


} // namespace studio
} // namespace gams
