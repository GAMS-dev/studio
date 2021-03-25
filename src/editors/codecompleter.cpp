#include "codecompleter.h"
#include "editors/codeedit.h"
#include "syntaxdata.h"
#include "logger.h"

#include <QSortFilterProxyModel>

namespace gams {
namespace studio {


// ----------- Model ---------------

CodeCompleterModel::CodeCompleterModel(QObject *parent): QAbstractListModel(parent)
{
    // DCOs
    QList<QPair<QString, QString>> src = syntax::SyntaxData::directives();
    QList<QPair<QString, QString>>::ConstIterator it = src.constBegin();
    while (it != src.constEnd()) {
        mData << '$' + it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccDco1);
    int i = mData.indexOf("$offText");
    if (i >= 0) {
        mType.insert(i, ccDco2);
        mType.insert(i+1, ccDco1);
    }

    // declarations
    src = syntax::SyntaxData::declaration();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccRes1);

    // reserved
    src = syntax::SyntaxData::declaration();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccRes2);

    // options
    src = syntax::SyntaxData::options();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccOpt);

    // models
    src = syntax::SyntaxData::modelTypes();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccMod);

}

int CodeCompleterModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mData.size();
}

QVariant CodeCompleterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        return mData.at(index.row());
    case Qt::ToolTipRole:
        return mDescription.at(index.row());
    case Qt::UserRole:
        return mType.lowerBound(index.row()).value();
    }
    return QVariant();
}


// ----------- Filter ---------------

QVariant FilterCompleterModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::EditRole) {
        bool ok;
        int type = QSortFilterProxyModel::data(index, Qt::UserRole).toInt(&ok);
        if (!ok || (type & mTypeFilter)) return '?' + QSortFilterProxyModel::data(index, Qt::DisplayRole).toString();
        return QSortFilterProxyModel::data(index, Qt::DisplayRole);
    }
    return QSortFilterProxyModel::data(index, role);
}

void FilterCompleterModel::setFilter(int completerTypeFilter)
{
    mTypeFilter = completerTypeFilter;
}


// ----------- Completer ---------------

CodeCompleter::CodeCompleter(CodeEdit *parent) :
    QListView(nullptr),
    mEdit(parent),
    mModel(new CodeCompleterModel(parent)),
    mFilterModel(new FilterCompleterModel(parent))
{
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setFilterRole(Qt::EditRole);
    mFilterModel->setSourceModel(mModel);
    setModel(mFilterModel);
    setWindowFlag(Qt::FramelessWindowHint);
}

CodeCompleter::~CodeCompleter()
{
}

bool CodeCompleter::event(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if (!this->isActiveWindow())
            hide();
    }
    return QListView::event(event);
}

void CodeCompleter::showEvent(QShowEvent *event)
{
    updateFilter();
    if (mFilterModel->rowCount()) {
        setCurrentIndex(mFilterModel->index(0,0));
        QListView::showEvent(event);
        setFocus();
    } else {
        mEdit->setFocus();
        hide();
    }
}

void CodeCompleter::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    hide();
}

void CodeCompleter::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
        hide();
        break;
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Home:
    case Qt::Key_End: {
        QListView::keyPressEvent(e);
        e->accept();
    }   break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab: {
        e->accept();
        insertCurrent();
    }   break;
    default: {
        mEdit->keyPressEvent(e);
        updateFilter();
    }
    }
}

void CodeCompleter::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab:
            break;
    default:
            mEdit->keyPressEvent(e);
    }
}

void CodeCompleter::focusOutEvent(QFocusEvent *event)
{
    QListView::focusOutEvent(event);
    hide();
}

enum CharGroup {
    clBreak,
    clAlpha,
    clNum,
    clFix,
};

CharGroup group(const QChar &c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') return clAlpha;
    if ((c >= '0' && c <= '9') || c == ':' || c == '.') return clNum;
    if (c == '$') return clFix;
    return clBreak;
}

void CodeCompleter::updateFilter()
{
    QTextCursor cur = mEdit->textCursor();
    QString line = cur.block().text();
    int peekStart = cur.positionInBlock();
    int validStart = peekStart;
    while (peekStart > 0) {
        --peekStart;
        CharGroup cg = group(line.at(peekStart));
        if (cg == clBreak) break;
        if (cg == clAlpha) validStart = peekStart;
        if (cg == clFix) {
            validStart = peekStart;
            break;
        }
    }
    int len = cur.positionInBlock() - validStart;
    if (!len) {
        mFilterText = "";
    } else {
        mFilterText = line.mid(validStart, len);
    }
    // TODO(JM) get matching filter from syntax type
//    mFilterModel->setFilter(xxx);
    if (mFilterText.startsWith('$'))
        mFilterModel->setFilterRegExp("^\\"+mFilterText+".*");
    else
        mFilterModel->setFilterRegExp('^'+mFilterText+".*");

    if (!currentIndex().isValid())
        setCurrentIndex(mFilterModel->index(0,0));
}

void CodeCompleter::updateDynamicData(QStringList symbols)
{
    Q_UNUSED(symbols)
    DEB() << "CodeCompleter doesn't support dynamic data yet";
}

int CodeCompleter::rowCount()
{
    return mFilterModel->rowCount();
}

void CodeCompleter::insertCurrent()
{
    if (currentIndex().isValid()) {
        QTextCursor cur = mEdit->textCursor();
        QString line = cur.block().text();
        cur.beginEditBlock();
        if (mFilterText.length())
            cur.setPosition(cur.position()-mFilterText.length());
        int start = cur.positionInBlock();
        QString res = model()->data(currentIndex()).toString();
        int i = mFilterText.length();
        for ( ; i < res.length() && start+i < line.length(); ++i) {
            if (line.at(start+i).toLower() != res.at(i).toLower()) break;
        }
        if (i > 0)
            cur.setPosition(cur.position() + i, QTextCursor::KeepAnchor);

        cur.insertText(res);
        cur.endEditBlock();
        mEdit->setTextCursor(cur);
    }
    mEdit->setFocus();
    hide();
}

} // namespace studio
} // namespace gams
