#include "filecontext.h"
#include <QTextStream>
#include <QDebug>

namespace ide {

FileContext::FileContext(QString fileName)
{
    mFileInfo = QFileInfo(fileName);
    QString suffix = mFileInfo.suffix();
    QString pattern = ".gms";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftGms;
    pattern = ".txt.text";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftTxt;
    pattern = ".inc";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftInc;
    pattern = ".log";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftLog;
    pattern = ".lst";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftLst;
    pattern = ".lxi";
    if (pattern.indexOf(suffix, Qt::CaseInsensitive)>=0) mFileType = FileType::ftLxi;
}

bool FileContext::isEmpty()
{
    return mFileInfo.fileName().isEmpty();
}

CodeEditor*FileContext::createEditor(QTabWidget* tabWidget)
{
    if (!tabWidget) {
        throw std::exception("error: missing tabWidget to create CodeEditor");
    }
    mTextEdit = new ide::CodeEditor(tabWidget);
    mTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    mTabIndex = tabWidget->addTab(mTextEdit, mFileInfo.baseName());
    tabWidget->setTabToolTip(mTabIndex, mFileInfo.filePath());
    tabWidget->setCurrentIndex(mTabIndex);
    return mTextEdit;
}

bool FileContext::exist()
{
    return !mFileInfo.fileName().isEmpty() && mFileInfo.exists();
}

bool FileContext::load()
{
    QFile file(mFileInfo.filePath());

    if (file.exists()) {
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        mTextEdit->setPlainText(stream.readAll());
        file.close();
        return true;
    }
    qDebug() << "file not opened";
    return false;
}

} // namespace ide
