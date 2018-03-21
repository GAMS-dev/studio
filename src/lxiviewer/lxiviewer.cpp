#include "filecontext.h"
#include "filegroupcontext.h"
#include "gamsprocess.h"
#include "lxiviewer.h"
#include "lxiparser.h"
#include "lxitreemodel.h"
#include "ui_lxiviewer.h"

#include "logger.h"

namespace gams {
namespace studio {
namespace lxiviewer {

LxiViewer::LxiViewer(CodeEditor *codeEditor, FileContext *fc, QWidget *parent):
    QWidget(parent), mCodeEditor(codeEditor), mFileContext(fc),
    ui(new Ui::LxiViewer)
{
    ui->setupUi(this);

    mLstFile = mFileContext->location();

    ui->splitter->addWidget(mCodeEditor);

    QFileInfo info(mLstFile);
    mLxiFile = info.path() + "/" + info.baseName() + ".lxi";

    loadLxiFile();
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    connect(ui->lxiTreeView, &QTreeView::doubleClicked, this, &LxiViewer::jumpToLine);
    connect(mCodeEditor, &CodeEditor::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
    connect(mFileContext->parentEntry(), &FileGroupContext::gamsProcessStateChanged, this, &LxiViewer::loadLxiFile);


    //connect(mCodeEditor, &CodeEditor::textChanged, this, &LxiViewer::loadLxiFile);
}

LxiViewer::~LxiViewer()
{
    LxiTreeModel* oldModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
    if (oldModel)
        delete oldModel;
    delete ui;
}

CodeEditor *LxiViewer::codeEditor() const
{
    return mCodeEditor;
}

void LxiViewer::loadLxiFile()
{
    if (QProcess::NotRunning == mFileContext->parentEntry()->gamsProcessState()) {
        if (QFileInfo(mLxiFile).exists()) {
            ui->splitter->widget(0)->show();
            LxiTreeModel* model = LxiParser::parseFile(QDir::toNativeSeparators(mLxiFile));
            LxiTreeModel* oldModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
            ui->lxiTreeView->setModel(model);
            if (oldModel)
                delete oldModel;
        }
        else
            ui->splitter->widget(0)->hide();
    }
}

void LxiViewer::jumpToTreeItem()
{
    int lineNr  = mCodeEditor->textCursor().block().blockNumber();
    LxiTreeModel* lxiTreeModel = static_cast<LxiTreeModel*>(ui->lxiTreeView->model());
    int itemIdx = 0;

    if (lineNr >= lxiTreeModel->lineNrs().first()) {
        itemIdx=1;
        while (lxiTreeModel->lineNrs().size()>itemIdx && lineNr >= lxiTreeModel->lineNrs()[itemIdx])
            itemIdx++;
        itemIdx--;
    }

    LxiTreeItem* treeItem = lxiTreeModel->treeItems()[itemIdx];
    if (!ui->lxiTreeView->isExpanded(treeItem->parentItem()->modelIndex()))
        ui->lxiTreeView->expand(treeItem->parentItem()->modelIndex());
    ui->lxiTreeView->selectionModel()->select(treeItem->modelIndex(), QItemSelectionModel::SelectCurrent);
    ui->lxiTreeView->scrollTo(treeItem->modelIndex());
}

void LxiViewer::jumpToLine(QModelIndex modelIndex)
{

    LxiTreeItem* selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
    int lineNr = selectedItem->lineNr();

    //jump to first child for virtual nodes
    if(lineNr == -1) {
        if (!ui->lxiTreeView->isExpanded(modelIndex)) {
            modelIndex = modelIndex.child(0,0);
            selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
            lineNr = selectedItem->lineNr();
        }
        else
            return;
    }

    QTextBlock tb = mCodeEditor->document()->findBlockByNumber(lineNr);
    while (tb.isValid() && tb.text().isEmpty()) {
        tb = tb.next();
    }
    lineNr  = tb.blockNumber();
    QTextCursor cursor = mCodeEditor->textCursor();
    cursor.setPosition(tb.position());

    disconnect(mCodeEditor, &CodeEditor::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
    mFileContext->jumpTo(cursor, true);
    connect(mCodeEditor, &CodeEditor::cursorPositionChanged, this, &LxiViewer::jumpToTreeItem);
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
