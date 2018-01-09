#include "searchwidget.h"
#include "studiosettings.h"
#include "syntax.h"
#include "ui_searchwidget.h"
#include <QDebug>

namespace gams {
namespace studio {

SearchWidget::SearchWidget(StudioSettings *settings, RecentData &rec, FileRepository &repo, MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::SearchWidget), mSettings(settings), mRecent(rec), mRepo(repo), mMain(parent)
{
    ui->setupUi(this);
    ui->cb_regex->setChecked(mSettings->searchUseRegex());
    ui->cb_caseSens->setChecked(mSettings->searchCaseSens());
    ui->cb_wholeWords->setChecked(mSettings->searchWholeWords());
    ui->combo_scope->setCurrentIndex(mSettings->selectedScopeIndex());
    ui->lbl_nrResults->setText("");
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

bool SearchWidget::regex()
{
    return ui->cb_regex->isChecked();
}

bool SearchWidget::caseSens()
{
    return ui->cb_caseSens->isChecked();
}

bool SearchWidget::wholeWords()
{
    return ui->cb_wholeWords->isChecked();
}

QString SearchWidget::searchTerm()
{
    return ui->txt_search->text();
}

int SearchWidget::selectedScope()
{
    return ui->combo_scope->currentIndex();
}

void SearchWidget::setSelectedScope(int index)
{
    ui->combo_scope->setCurrentIndex(index);
}

void SearchWidget::on_btn_Find_clicked()
{
    find();
}

void SearchWidget::on_btn_FindAll_clicked()
{
    QList<Result> res;

    switch (ui->combo_scope->currentIndex()) {
    case 0: // this file
        findAndHighlight();
        break;
    case 1: // this group
        res = findInGroup();
        mMain->showResults(res);
        break;
    case 2: // open files

        break;
    case 3: // all files/groups

        break;
    default:
        break;
    }
}

QList<Result> SearchWidget::findInGroup(FileSystemContext *fsc)
{
    QList<Result> matches;

    FileGroupContext *fgc;
    if (fsc == nullptr) {
        FileContext* fc = mMain->fileRepository()->fileContext(mRecent.editor);
        fgc = (fc ? fc->parentEntry() : nullptr);

        if (!fgc)
            return QList<Result>();
    }

    for (int i = 0; i < fgc->childCount(); i++) {
        matches.append(findInFile(fgc->childEntry(i)));
    }
    return matches;

    // dummy content
    Result r1(11, "somefile.gms", "Full Line of Text with some extra words added at the");
    Result r2(5, "readme.txt", "this is a readme file");
    Result r3(1001, "extrafile.gms", "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.");

    QList<Result> ll;
    ll.append(r1);
    ll.append(r2);
    ll.append(r3);

}

QList<Result> SearchWidget::findInFile(FileSystemContext *fsc)
{
    if (fsc->type()) // is group
        findInGroup(fsc); // TESTME studio does not support this case yet

    FileContext *fc(static_cast<FileContext*>(fsc));

    qDebug() << "fsc->name():" << fsc->name() << "type():" << fsc->type();

    Result r(0, fc->location(), "hi");

    return QList<Result>() << r;
}

void SearchWidget::findAndHighlight(QPlainTextEdit* edit)
{
    if (edit == nullptr)
        edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    QString searchTerm = ui->txt_search->text();

    QTextCursor item;
    QTextCursor lastItem;
    FileContext *fc = mRepo.fileContext(mRecent.editor);
    int hits = 0;

    fc->removeTextMarks(TextMark::result);
    QTextCursor tmpCurs = edit->textCursor();
    tmpCurs.clearSelection();
    edit->setTextCursor(tmpCurs);

    do {
        item = edit->document()->find(searchTerm, lastItem, getFlags());
        lastItem = item;
        if (!item.isNull()) {
            int length = item.selectionEnd() - item.selectionStart();
            mAllTextMarks.append(fc->generateTextMark(TextMark::result, 0, item.blockNumber(),
                                                      item.columnNumber() - length, length));
            hits++;
        }
    } while (!item.isNull());

    if (fc->highlighter()) fc->highlighter()->rehighlight();
    if (hits == 1)
        ui->lbl_nrResults->setText(QString::number(hits) + " match");
    else
        ui->lbl_nrResults->setText(QString::number(hits) + " matches");
}

void SearchWidget::find(bool backwards)
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    bool useRegex = ui->cb_regex->isChecked();
    QString searchTerm = ui->txt_search->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();
    if (backwards)
        searchFlags.setFlag(QTextDocument::FindFlag::FindBackward);

    QRegularExpression searchRegex;
    if (useRegex) searchRegex.setPattern(searchTerm);

    mLastSelection = (!mSelection.isNull() ? mSelection : edit->textCursor());
    if (useRegex) {
        mSelection = edit->document()->find(searchRegex, mLastSelection, searchFlags);
    } else {
        mSelection = edit->document()->find(searchTerm, mLastSelection, searchFlags);
    }

    // nothing found, try to start over
    if (mSelection.isNull()) {
        if (useRegex) {
            mSelection = edit->document()->find(searchRegex, 0, searchFlags);
        } else {
            mSelection = edit->document()->find(searchTerm, 0, searchFlags);
        }
    }

    // on hit
    if (!mSelection.isNull()) {
        edit->setTextCursor(mSelection);
        ui->btn_Replace->setEnabled(true);
    }
}

void SearchWidget::on_btn_Replace_clicked()
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    QString replaceTerm = ui->txt_replace->text();
    if (edit->textCursor().hasSelection())
        edit->textCursor().insertText(replaceTerm);

    find();
}

void SearchWidget::on_btn_ReplaceAll_clicked()
{
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    QString searchTerm = ui->txt_search->text();
    QString replaceTerm = ui->txt_replace->text();
    QFlags<QTextDocument::FindFlag> searchFlags = getFlags();

    QList<QTextCursor> hits;
    QTextCursor item;
    QTextCursor lastItem;

    do {
        item = edit->document()->find(searchTerm, lastItem, searchFlags);
        lastItem = item;
        if (!item.isNull()) {
            hits.append(item);
        }
    } while (!item.isNull());

    QMessageBox msgBox;
    msgBox.setText("Replacing " + QString::number(hits.length()) + " occurrences of '" +
                   searchTerm + "' with '" + replaceTerm + "'. Are you sure?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int answer = msgBox.exec();

    if (answer == QMessageBox::Ok) {
        edit->textCursor().beginEditBlock();
        foreach (QTextCursor tc, hits) {
            tc.insertText(replaceTerm);
        }
        edit->textCursor().endEditBlock();
    }
}

void SearchWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    QPlainTextEdit* edit = FileSystemContext::toPlainEdit(mRecent.editor);
    if (!edit) return;

    ui->txt_search->setFocus();
    if (edit->textCursor().hasSelection())
        ui->txt_search->setText(edit->textCursor().selection().toPlainText());
    else
        ui->txt_search->setText("");
}

void SearchWidget::keyPressEvent(QKeyEvent* event)
{
    if (isVisible() && ( event->key() == Qt::Key_Escape
                         || (event->modifiers() & Qt::ControlModifier && (event->key() == Qt::Key_F)) )) {
        hide();
        mRecent.editor->setFocus();
    }
    if (isVisible() && event->modifiers() & Qt::ShiftModifier && event->key() == Qt::Key_F3) {
        find(true);
    } else if (isVisible() && event->key() == Qt::Key_F3) {
        find();
    }
}

void SearchWidget::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event);
    FileContext *fc = mRepo.fileContext(mRecent.editor);
    if (fc)
        fc->removeTextMarks(TextMark::result);

    ui->lbl_nrResults->setText("");
}


void SearchWidget::on_txt_search_returnPressed()
{
    on_btn_Find_clicked();
}

QFlags<QTextDocument::FindFlag> SearchWidget::getFlags()
{
    QFlags<QTextDocument::FindFlag> searchFlags;
    searchFlags.setFlag(QTextDocument::FindCaseSensitively, ui->cb_caseSens->isChecked());
    searchFlags.setFlag(QTextDocument::FindWholeWords, ui->cb_wholeWords->isChecked());

    return searchFlags;
}

void SearchWidget::on_btn_close_clicked()
{
    SearchWidget::close();
}

int Result::locLineNr() const
{
    return mLocLineNr;
}

QString Result::locFile() const
{
    return mLocFile;
}

QString Result::context() const
{
    return mContext;
}

Result::Result(int locLineNr, QString locFile, QString context) :
    mLocLineNr(locLineNr), mLocFile(locFile), mContext(context)
{
    qDebug() << "created result at" << mLocFile << ":" << locLineNr;
}

}
}
