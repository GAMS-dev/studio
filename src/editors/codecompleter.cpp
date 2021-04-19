#include "codecompleter.h"
#include "editors/codeedit.h"
#include "syntaxdata.h"
#include "syntax/syntaxformats.h"
#include "logger.h"
#include "exception.h"

#include <QSortFilterProxyModel>
#include <QGuiApplication>
#include <QScreen>
#include <QAction>

namespace gams {
namespace studio {


// ----------- Model ---------------

CodeCompleterModel::CodeCompleterModel(QObject *parent): QAbstractListModel(parent)
{
    mCasing = caseCamel;
    initData();
}

void CodeCompleterModel::initData()
{
    mData.clear();
    mDescription.clear();
    mDescriptIndex.clear();
    mType.clear();

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
        if (it->first.startsWith("Equation")) {
            mType.insert(mData.size()-1, ccRes1);
            if (it->first.endsWith("s")) {
                mData << "Equation Table";
                mDescription << it->second;
            }
        } else if (it->first == "Parameter") {
            mType.insert(mData.size()-1, ccRes1);
            if (it->first.endsWith("s")) {
                mData << "Parameter Table";
                mDescription << it->second;
            }
        } else if (it->first == "Set") {
            mType.insert(mData.size()-1, ccResS);
            if (it->first.endsWith("s")) {
                mData << "Set Table";
                mDescription << it->second;
            }
        } else if (it->first == "Variable") {
            mType.insert(mData.size()-1, ccResV);
            if (it->first.endsWith("s")) {
                mData << "Variable Table";
                mDescription << it->second;
            }
        } else if (it->first == "Table") {
            mType.insert(mData.size()-1, ccResT);
        } else {
            mType.insert(mData.size()-1, ccRes1);
        }
        ++it;
    }

    // declaration additions for "Variable" and "Set"
    src = syntax::SyntaxData::declaration4Var();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + " Variable";
        mDescription << it->second;
        ++it;
    }
    src = syntax::SyntaxData::declaration4Set();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + " Set";
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

    // sub DCOs
    src = syntax::SyntaxData::execute();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        mData << '.' + it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccSubDcoC);
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "$call." + it->first + ' ';
        mDescription << it->second;
        mData << "$hiddenCall." + it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccDco1);

    mData << "set";
    mDescription << "compile-time variable based on a GAMS set";
    mData << ".set";
    mDescription << "compile-time variable based on a GAMS set";
    mType.insert(mData.size()-1, ccSubDcoE);
    mData << "$eval.set";
    mDescription << "compile-time variable based on a GAMS set";
    mData << "$evalGlobal.set";
    mDescription << "compile-time variable based on a GAMS set";
    mData << "$evalLocal.set";
    mDescription << "compile-time variable based on a GAMS set";
    mType.insert(mData.size()-1, ccDco1);

    mData << "noError";
    mDescription << "abort without error";
    mData << ".noError";
    mDescription << "abort without error";
    mType.insert(mData.size()-1, ccSubDcoA);
    mData << "$abort.noError";
    mDescription << "abort without error";
    mType.insert(mData.size()-1, ccDco1);

    for (int i = 0; i < mData.size(); ++i) {
        mDescriptIndex << i;
    }
}

void CodeCompleterModel::addDynamicData()
{
    QStringList data;
    QList<int> descriptIndex;
    QMap<int, CodeCompleterType> iType;
    for (int i = 0; i < mData.size(); ++i) {
        if (mDescriptIndex.at(i) != i) {
            FATAL() << "ERROR addDynamicData() MUST not be called twice.";
        }
        data << mData.at(i);
        descriptIndex << i;
        if (mData.at(i).toLower() != mData.at(i)) {
            data << mData.at(i).toLower();
            descriptIndex << i;
        }
        if (mData.at(i).toUpper() != mData.at(i)) {
            data << mData.at(i).toUpper();
            descriptIndex << i;
        }
        if (mType.contains(i)) {
            iType.insert(data.size()-1, mType.value(i));
        }
    }
    mData = data;
    mDescriptIndex = descriptIndex;
    mType = iType;
}

void CodeCompleterModel::setCasing(CodeCompleterCasing casing)
{
    bool isDynamic = (mCasing == caseDynamic);
    mCasing = casing;
    if (isDynamic != (casing == caseDynamic)) {
        beginResetModel();
        initData();
        if (casing == caseDynamic)
            addDynamicData();
        endResetModel();
    }
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
        return mCasing == caseLower ? mData.at(index.row()).toLower()
                                    : mCasing == caseUpper ? mData.at(index.row()).toUpper() : mData.at(index.row());
    case Qt::ToolTipRole:
        return mDescription.at(mDescriptIndex.at(index.row()));
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
    if (type & ccSubDco) {
        if (sourceModel()->data(index, Qt::DisplayRole).toString().startsWith('.') != mNeedDot)
            return false;
    }
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

void FilterCompleterModel::setTypeFilter(int completerTypeFilter, bool needDot)
{
    mTypeFilter = completerTypeFilter;
    mNeedDot = needDot;
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
    mFilterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    setModel(mFilterModel);
    setWindowFlag(Qt::FramelessWindowHint);
}

CodeCompleter::~CodeCompleter()
{
}

void CodeCompleter::setCodeEdit(CodeEdit *edit)
{
    mEdit = edit;
    if (mEdit) setFont(mEdit->font());
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
    QListView::showEvent(event);
    setFocus();
}

void CodeCompleter::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    insertCurrent();
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
        if (e->key() == Qt::Key_Space)
            hide();
        if (mEdit)
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
        if (mEdit) mEdit->keyReleaseEvent(e);
    }
}

void CodeCompleter::focusOutEvent(QFocusEvent *event)
{
    QListView::focusOutEvent(event);
    hide();
}

void CodeCompleter::actionEvent(QActionEvent *event)
{
    Q_UNUSED(event)
    hide();
}

enum CharGroup {
    clAlpha,
    clNum,
    clFix,
    clBreak,
    clSpace,
};

CharGroup group(const QChar &c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') return clAlpha;
    if ((c >= '0' && c <= '9') || c == ':' || c == '.') return clNum;
    if (c == '$') return clFix;
    if (c == ' ' || c == '\t') return clSpace;
    return clBreak;
}

void CodeCompleter::updateFilter()
{
    if (!mEdit) return;
    QTextCursor cur = mEdit->textCursor();
    QString line = cur.block().text();
    int peekStart = cur.positionInBlock();
    int validStart = peekStart;
    while (peekStart > 0) {
        --peekStart;
        CharGroup cg = group(line.at(peekStart));
        if (cg >= clBreak) break;
        if (cg == clAlpha || cg == clNum) validStart = peekStart;
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

    // assign filter
    if (mModel->casing() == caseDynamic && mFilterModel->filterCaseSensitivity() == Qt::CaseInsensitive)
         mFilterModel->setFilterCaseSensitivity(Qt::CaseSensitive);
    mFilterModel->setTypeFilter(getFilterFromSyntax(), mNeedDot);
    if (mFilterText.startsWith('$'))
        mFilterModel->setFilterRegularExpression("^\\"+mFilterText+".*");
    else
        mFilterModel->setFilterRegularExpression('^'+mFilterText+".*");
    if (mModel->casing() == caseDynamic && !mFilterModel->rowCount())
         mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    if (!mFilterModel->rowCount() ||
            (mFilterModel->rowCount() == 1 && mFilterModel->data(mFilterModel->index(0,0)).toString() == mFilterText)) {
        hide();
        return;
    }
    mFilterModel->sort(0);

    // find best index
    int validEnd = cur.positionInBlock();
    for (int i = validEnd+1; i < line.length(); ++i) {
        CharGroup cg = group(line.at(i));
        if (cg >= clBreak) break;
        validEnd = i;
    }
    QString fullWord = line.mid(validStart, validEnd - validStart + 1);
    int bestInd = 0;
    Qt::CaseSensitivity caseSens = mFilterModel->filterCaseSensitivity();
    while (bestInd+1 < mFilterModel->rowCount()) {
        QModelIndex ind = mFilterModel->index(bestInd, 0);
        QString itemWord = mFilterModel->data(ind).toString().left(fullWord.length());
        if (itemWord.compare(fullWord, caseSens) > 0)
            break;
        if (itemWord.compare(fullWord, caseSens) == 0)
            break;
        ++bestInd;
    }
    setCurrentIndex(mFilterModel->index(bestInd, 0));
    scrollTo(currentIndex());

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
    if (rowCount() &&
            (mFilterModel->rowCount() > 1 || mFilterModel->data(mFilterModel->index(0,0)).toString() != mFilterText)) {
        show();
    }
}

void CodeCompleter::setCasing(CodeCompleterCasing casing)
{
    mModel->setCasing(casing);
    mFilterModel->setFilterCaseSensitivity(casing == caseDynamic ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void CodeCompleter::insertCurrent()
{
    if (!mEdit) return;
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
    if (!mEdit) return ccNone;
    int res = ccAll;
    QTextCursor cur = mEdit->textCursor();
    int syntaxKind = 0;
    int syntaxFlavor = 0;
    int dcoFlavor = 0;

    QMap<int,QPair<int, int>> blockSyntax;
    emit mEdit->scanSyntax(cur.block(), blockSyntax);

    QString line = cur.block().text();
    int start = cur.positionInBlock() - mFilterText.length();
    for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
        if (it.key() > start) break;
        syntaxKind = it.value().first;
        syntaxFlavor = it.value().second;
        if (syntax::SyntaxKind(syntaxKind) == syntax::SyntaxKind::Dco)
            dcoFlavor = syntaxFlavor;
    }

    // for analysis
//    DEB() << "--- Line: \"" << cur.block().text() << "\"   start:" << start;
//    for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
//        DEB() << "pos: " << it.key() << " = " << syntax::SyntaxKind(it.value().first) << ":" << it.value().second;
//    }

    switch (syntax::SyntaxKind(syntaxKind)) {
    case syntax::SyntaxKind::Standard:
    case syntax::SyntaxKind::Formula:
    case syntax::SyntaxKind::Assignment:
    case syntax::SyntaxKind::IgnoredHead:
    case syntax::SyntaxKind::IgnoredBlock:
    case syntax::SyntaxKind::Semicolon:
    case syntax::SyntaxKind::CommaIdent:
        res = ccStart; break;

    case syntax::SyntaxKind::SubDCO:
        res = ccNone; break;

    case syntax::SyntaxKind::Declaration:  // [set parameter variable equation] allows table
        res = ccDco | ccResT; break;
    case syntax::SyntaxKind::DeclarationSetType:
        res = ccDco | ccResS; break;
    case syntax::SyntaxKind::DeclarationVariableType:
        res = ccDco | ccResV; break;

    case syntax::SyntaxKind::Dco:
        res = ccDco; break;

    case syntax::SyntaxKind::DcoBody:
    case syntax::SyntaxKind::DcoComment:
    case syntax::SyntaxKind::Title:
    case syntax::SyntaxKind::CommentBlock:
        res = ccNone; break;

    case syntax::SyntaxKind::String:
    case syntax::SyntaxKind::CommentLine:
    case syntax::SyntaxKind::CommentEndline:
    case syntax::SyntaxKind::CommentInline:
        res = ccNoDco; break;

    case syntax::SyntaxKind::Identifier:
        res = ccDco | ccResT; break;
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

    start = qMin(start, line.length()-1);
    bool isWhitespace = true;
    for (int i = 0; i < start; ++i) {
        if (line.at(i) != ' ' && line.at(i) != '\t') {
            isWhitespace = false;
            break;
        }
    }
    mNeedDot = false;
    if (isWhitespace) {
        if (syntax::SyntaxKind(syntaxKind) == syntax::SyntaxKind::CommentBlock)
            res = ccDco2;
        else if (!(res & ccDco))
            res = res & ccDco;
    } else if (dcoFlavor > 15) {
        mNeedDot = true;
        for (int i = start-1; i > 0; --i) {
            if (mNeedDot && line.at(i) == '.') {
                mNeedDot = false;
            } else {
                CharGroup gr = group(line.at(i));
                if (gr >= clBreak) return res = res & ccNoDco;
            }
        }
        if (dcoFlavor == 16)
            res = ccSubDcoA;
        else if (dcoFlavor == 17)
            res = ccSubDcoC;
        else if (dcoFlavor == 18)
            res = ccSubDcoE;
        else
            res = res & ccNoDco;
    } else {
        res = res & ccNoDco;
    }

//    DEB() << " -> selected: " << syntax::SyntaxKind(syntaxKind) << ":" << syntaxFlavor << "     filter: " << QString::number(res, 16);
    return res;
}

} // namespace studio
} // namespace gams
