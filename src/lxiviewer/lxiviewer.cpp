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

    if (QFileInfo(mLxiFile).exists()) {
        LxiTreeModel* model = new LxiTreeModel(LxiParser::parseFile(QDir::toNativeSeparators(mLxiFile)));
        ui->lxiTreeView->setModel(model);
        connect(ui->lxiTreeView, &QTreeView::doubleClicked, this, &LxiViewer::jumpToLine);
    }
}

LxiViewer::~LxiViewer()
{
    delete ui;
}

CodeEditor *LxiViewer::codeEditor() const
{
    return mCodeEditor;
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
        mFileContext->jumpTo(cursor, false);
    }
}

} // namespace lxiviewer
} // namespace studio
} // namespace gams
