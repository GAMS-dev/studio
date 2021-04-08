#include "codecompleter.h"
#include "editors/codeedit.h"
#include "syntaxdata.h"
#include "syntax/syntaxformats.h"
#include "logger.h"

#include <QSortFilterProxyModel>
#include <QGuiApplication>
#include <QScreen>

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
        if (i > 0) mType.insert(i-1, ccDco1);
        mType.insert(i, ccDco2);
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

    // declaration additions for "variable" and "set"
    src = syntax::SyntaxData::declaration4Var();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    src = syntax::SyntaxData::declaration4Set();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccRes2);

    // reserved
    src = syntax::SyntaxData::reserved();
    it = src.constBegin();
    while (it != src.constEnd()) {
        if (it->first == "ord") {
            mData << "option " << "options ";
            mDescription << "" << "";
        }
        if (it->first == "sum") {
            mData << "solve ";
            mDescription << "";
        }
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    // embedded
    src = syntax::SyntaxData::embedded();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccRes3);

    // embedded end
    src = syntax::SyntaxData::embeddedEnd();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccRes4);

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

    // models
    src = syntax::SyntaxData::extendableKey();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ' << it->second;
        mDescription << it->second << "";
        ++it;
    }
    mType.insert(mData.size()-1, ccSolve);

    // execute
    src = syntax::SyntaxData::execute();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccExec);
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

bool FilterCompleterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    int type = sourceModel()->data(index, Qt::UserRole).toInt();
    if (!(type & mTypeFilter)) return false;
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

void FilterCompleterModel::setTypeFilter(int completerTypeFilter)
{
    mTypeFilter = completerTypeFilter;
    invalidateFilter();
}


// ----------- Completer ---------------

CodeCompleter::CodeCompleter(CodeEdit *parent) :
    QListView(nullptr),
    mEdit(parent),
    mModel(new CodeCompleterModel(parent)),
    mFilterModel(new FilterCompleterModel(parent))
{
    if (mEdit) setFont(mEdit->font());
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
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
    setCurrentIndex(mFilterModel->index(0,0));
    QListView::showEvent(event);
    setFocus();
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
        if (e->key() == Qt::Key_Up && currentIndex().row() == 0) {
            QModelIndex mi = model()->index(rowCount()-1, 0);
            if (mi.isValid()) setCurrentIndex(mi);
        } else if (e->key() == Qt::Key_Down && currentIndex().row() == rowCount()-1) {
            QModelIndex mi = model()->index(0, 0);
            if (mi.isValid()) setCurrentIndex(mi);
        } else
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

    mFilterModel->setTypeFilter(getFilterFromSyntax());
    if (mFilterText.startsWith('$'))
        mFilterModel->setFilterRegularExpression("^\\"+mFilterText+".*");
    else
        mFilterModel->setFilterRegularExpression('^'+mFilterText+".*");

    if (!mFilterModel->rowCount()) hide();

    if (!currentIndex().isValid())
        setCurrentIndex(mFilterModel->index(0,0));

    // adapt size
    cur.setPosition(cur.position() - mFilterText.length());
    QPoint pos = mEdit->cursorRect(cur).bottomLeft()
            + QPoint(mEdit->viewportMargins().left(), mEdit->viewportMargins().top());

    QRect rect = QRect(mEdit->mapToGlobal(pos), geometry().size());
    int hei = sizeHintForRow(0) * qMin(10, rowCount());
    QScreen *screen = qApp->screenAt(rect.topLeft());
    while (hei > sizeHintForRow(0) && rect.top() + hei > screen->availableVirtualGeometry().bottom())
        hei -= sizeHintForRow(0);

    int wid = 0;
    for (int row = 0; row < rowCount(); ++row)
        wid = qMax(wid, sizeHintForColumn(row));

    rect.setHeight(hei + 4);
    rect.setWidth(wid + 25);
    setGeometry(rect);
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

void CodeCompleter::ShowIfData()
{
    updateFilter();
    if (rowCount()) {
        show();
    }
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

int CodeCompleter::getFilterFromSyntax()
{
    int res = ccAll;
    QTextCursor cur = mEdit->textCursor();
    int syntaxKind = 0;
    int syntaxFlavor = 0;

    QMap<int,QPair<int, int>> blockSyntax;
    emit mEdit->scanSyntax(cur.block(), blockSyntax);

    int start = cur.positionInBlock() - mFilterText.length();
    for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
        syntaxKind = it.value().first;
        syntaxFlavor = it.value().second;
        if (it.key() >= start) break;
    }

    // for analysis
    DEB() << "--- Line: " << cur.block().text();
    for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
        DEB() << "pos: " << it.key() << " = " << syntax::SyntaxKind(it.value().first) << ":" << it.value().second;
    }

    switch (syntax::SyntaxKind(syntaxKind)) {
    case syntax::SyntaxKind::Standard:
    case syntax::SyntaxKind::Formula:
    case syntax::SyntaxKind::Assignment:
    case syntax::SyntaxKind::Call:
    case syntax::SyntaxKind::IgnoredHead:
    case syntax::SyntaxKind::IgnoredBlock:
    case syntax::SyntaxKind::Semicolon:
    case syntax::SyntaxKind::CommaIdent:
    case syntax::SyntaxKind::DeclarationSetType:
    case syntax::SyntaxKind::DeclarationVariableType:
    case syntax::SyntaxKind::Declaration:
        res = ccStart; break;

    case syntax::SyntaxKind::Directive:
        res = ccDco; break;

    case syntax::SyntaxKind::DirectiveBody:
    case syntax::SyntaxKind::DirectiveComment:
    case syntax::SyntaxKind::Title:
    case syntax::SyntaxKind::CommentBlock:
        res = ccNone; break;

    case syntax::SyntaxKind::String:
    case syntax::SyntaxKind::CommentLine:
    case syntax::SyntaxKind::CommentEndline:
    case syntax::SyntaxKind::CommentInline:
        res = ccNoDco; break;

    case syntax::SyntaxKind::Identifier:
    case syntax::SyntaxKind::IdentifierDim:
    case syntax::SyntaxKind::IdentifierDimEnd:
    case syntax::SyntaxKind::IdentifierDescription:
    case syntax::SyntaxKind::IdentifierAssignment:
    case syntax::SyntaxKind::AssignmentLabel:
    case syntax::SyntaxKind::AssignmentValue:
    case syntax::SyntaxKind::IdentifierAssignmentEnd:
    case syntax::SyntaxKind::IdentifierTableAssignmentColHead:
    case syntax::SyntaxKind::IdentifierTableAssignmentRowHead:
    case syntax::SyntaxKind::IdentifierTableAssignmentRow:
        res = ccDco; break;

    case syntax::SyntaxKind::Embedded:
    case syntax::SyntaxKind::EmbeddedBody:
    case syntax::SyntaxKind::EmbeddedEnd:
    case syntax::SyntaxKind::Reserved:
    case syntax::SyntaxKind::Solve:
    case syntax::SyntaxKind::SolveBody:
    case syntax::SyntaxKind::SolveKey:
    case syntax::SyntaxKind::Option:
    case syntax::SyntaxKind::OptionKey:
    case syntax::SyntaxKind::Execute:
        res = ccStart; break;

    case syntax::SyntaxKind::ExecuteBody:
    case syntax::SyntaxKind::ExecuteKey:
        res = ccExec; break;

    case syntax::SyntaxKind::OptionBody:
        res = ccOpt; break;
    default: ;
    }
    bool isWhitespace = true;
    QString text = cur.block().text();
    for (int i = 0; i < start; ++i) {
        if (text.at(i) != ' ' && text.at(i) != '\t') {
            isWhitespace = false;
            break;
        }
    }
    if (isWhitespace) {
        if (syntax::SyntaxKind(syntaxKind) == syntax::SyntaxKind::CommentBlock)
            res = ccDco2;
        else if (!(res & ccDco))
            res = res & ccDco;
    } else
        res = res & ccNoDco;

    DEB() << " -> selected: " << syntax::SyntaxKind(syntaxKind) << ":" << syntaxFlavor << "     filter: " << QString::number(res, 16);
    return res;
}

} // namespace studio
} // namespace gams
