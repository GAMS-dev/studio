#include "filecontext.h"
#include "lxiviewer.h"
#include "lxiparser.h"
#include "lxitreemodel.h"
#include "ui_lxiviewer.h"

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
    connect(mCodeEditor, &CodeEditor::textChanged, this, &LxiViewer::loadLxiFile);
}

LxiViewer::~LxiViewer()
{
    delete ui;
}

CodeEditor *LxiViewer::codeEditor() const
{
    return mCodeEditor;
}

void LxiViewer::loadLxiFile()
{
    if (QFileInfo(mLxiFile).exists()) {
        ui->splitter->widget(0)->show();
        LxiTreeModel* model = new LxiTreeModel(LxiParser::parseFile(QDir::toNativeSeparators(mLxiFile)));
        ui->lxiTreeView->setModel(model);
    }
    else
        ui->splitter->widget(0)->hide();
}

void LxiViewer::jumpToLine(QModelIndex modelIndex)
{
    LxiTreeItem* selectedItem = static_cast<LxiTreeItem*>(modelIndex.internalPointer());
    int lineNr = selectedItem->lineNr();

    if (selectedItem && lineNr>0) {
        QTextBlock tb = mCodeEditor->document()->findBlockByNumber(lineNr);
        while (tb.text().isEmpty())
        {
            lineNr++;
            tb = mCodeEditor->document()->findBlockByNumber(lineNr);
        }
        QTextCursor cursor = mCodeEditor->textCursor();
        cursor.setPosition(tb.position());
        mFileContext->jumpTo(cursor, true);
    }
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
