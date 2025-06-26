/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
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
#include "codecompleter.h"
//#include "editors/codeedit.h"
#include "editors/sysloglocator.h"
#include "editors/abstractsystemlogger.h"
#include "syntaxdata.h"
#include "syntax/syntaxformats.h"
#include "logger.h"
#include "exception.h"

#include <QPlainTextEdit>
#include <QSortFilterProxyModel>
#include <QGuiApplication>
#include <QScreen>
#include <QAction>

// uncomment this to generate elements for testcompleter
// #define COMPLETER_DEBUG

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
    mNameSuffixAssignments.clear();
    mNameSuffixAssignments.insert('c', {"$offEcho"});
    mNameSuffixAssignments.insert('p', {"$offPut"});
    mNameSuffixAssignments.insert('m', {"$offEmbeddedCode"});
    mNameSuffixAssignments.insert('e', {"endEmbeddedCode", "pauseEmbeddedCode"});

    mData.clear();
    mDescription.clear();
    mDescriptIndex.clear();
    mType.clear();
    QList<QList<QPair<QString, QString>>::ConstIterator> delayedIterators;

    // DCOs
    mDollarGroupRow = int(mData.size());
    mData << "$...";
    mDescription << "";
    mType.insert(int(mData.size())-1, ccDcoStrt);

    QList<QPair<QString, QString>> src = syntax::SyntaxData::directives();
    QList<QPair<QString, QString>>::ConstIterator it = src.constBegin();
    while (it != src.constEnd()) {
        if ((!it->first.endsWith("mbeddedCode") && !it->first.endsWith("mbeddedCodeS")
             && !it->first.endsWith("mbeddedCodeV")) || it->first.startsWith('o')) {
            if (it->first == "offText" || it->first == "offEcho" || it->first == "offPut") {
                mData << '$' + it->first;
                mDescription << it->second;
            } else {
                delayedIterators << it;
            }
        }
        ++it;
    }
    mType.insert(int(mData.size())-1, ccDcoEnd);

    for (const QList<QPair<QString, QString>>::ConstIterator &it : std::as_const(delayedIterators)) {
        mData << '$' + it->first;
        mDescription << it->second;
    }
    mType.insert(int(mData.size())-1, ccDcoStrt);

    // declarations
    src = syntax::SyntaxData::declaration();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        if (it->first.startsWith("Equation")) {
            if (it->first.endsWith("s")) {
                mData << "Equation Table";
                mDescription << it->second;
            }
            mType.insert(int(mData.size())-1, ccDecl);
        } else if (it->first.startsWith("Parameter")) {
            if (it->first.endsWith("s")) {
                mData << "Parameter Table";
                mDescription << it->second;
            }
            mType.insert(int(mData.size())-1, ccDecl);
        } else if (it->first.startsWith("Set")) {
            mType.insert(int(mData.size())-1, ccDeclS);
            if (it->first.endsWith("s")) {
                mData << "Set Table";
                mDescription << it->second;
            }
            mType.insert(int(mData.size())-1, ccDecl);
        } else if (it->first.startsWith("Variable")) {
            if (it->first.endsWith("s")) {
                mData << "Variable Table";
                mDescription << it->second;
            }
            mType.insert(int(mData.size())-1, ccDeclV);
        } else if (it->first == "Table") {
            mType.insert(int(mData.size())-1, ccDeclT);
        } else {
            mType.insert(int(mData.size())-1, ccDecl);
        }
        ++it;
    }

    // declaration additions for "Variable" and "Set"
    src = syntax::SyntaxData::declaration4Var();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + " Variable" << it->first + " Variables";
        mDescription << it->second << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccDeclAddV);
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + " Variable Table" << it->first + " Variables Table";
        mDescription << it->second << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccDeclAddV);
    src = syntax::SyntaxData::declaration4Set();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + " Set" << it->first + " Sets";
        mDescription << it->second << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccDeclAddS);

    // reserved
    src = syntax::SyntaxData::reserved();
    it = src.constBegin();
    while (it != src.constEnd()) {
        if (it->first == "ord") {
            mData << "option" << "options";
            mDescription << "" << "";
        }
        if (it->first == "sum") {
            mData << "solve ";
            mDescription << "";
        }
        mData << it->first;
        mDescription << it->second;

        if (it->first == "abort") {
            mData << "abort.noError";
            mDescription << "Don't throw an execution error";
        }
        ++it;
    }
    // embedded
    src = syntax::SyntaxData::embedded();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    // execute
    src = syntax::SyntaxData::keyExecute();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    mData << "executeTool";
    mDescription << "Execute a GAMS tool";
    mType.insert(int(mData.size())-1, ccRes);

    // embedded end
    src = syntax::SyntaxData::embeddedEnd();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccResEnd);

    // options
    src = syntax::SyntaxData::options();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccOpt);

    // models
    src = syntax::SyntaxData::modelTypes();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccMod);

    // models
    src = syntax::SyntaxData::extendableKey();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ' << it->second;
        mDescription << it->second << "";
        ++it;
    }
    mType.insert(int(mData.size())-1, ccSolve);

    // sub DCOs
    src = syntax::SyntaxData::execute();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "$call." + it->first;
        mDescription << it->second;
        mData << "$hiddenCall." + it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccDcoStrt);

    mData << "$eval.set";
    mDescription << "compile-time variable based on a GAMS set";
    mData << "$evalGlobal.set";
    mDescription << "compile-time variable based on a GAMS set";
    mData << "$evalLocal.set";
    mDescription << "compile-time variable based on a GAMS set";
    mType.insert(int(mData.size())-1, ccDcoStrt);

    mData << "$abort.noError";
    mDescription << "Abort without error";
    mType.insert(int(mData.size())-1, ccDcoStrt);

    mData << "$save.keepCode";
    mDescription << "Create save also with unexecuted execution code";
    mType.insert(int(mData.size())-1, ccDcoStrt);

    // system data
    src = syntax::SyntaxData::systemData();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "system." + it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccSysDat);

    // system data
    src = syntax::SyntaxData::systemAttributes();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "system." + it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccSysSufR);

    mPercentGroupRow = int(mData.size());
    mData << "%...";
    mDescription << "";
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "%system." + it->first + "%";
        mDescription << it->second;
        ++it;
    }
    mData << "%" + syntax::SyntaxAbstract::systemEmpData.first + "%";
    mDescription << syntax::SyntaxAbstract::systemEmpData.second;
    mType.insert(int(mData.size())-1, ccSysSufC);

    QHash<QString, QString> descript;
    for (const QPair<QString, QString> &entry : syntax::SyntaxData::systemCTConstText())
        descript.insert(entry.first, entry.second);
    {
        QList<QPair<QString, int>> src = syntax::SyntaxData::systemCTConstants();
        QList<QPair<QString, int>>::ConstIterator it = src.constBegin();
        while (it != src.constEnd()) {
            mData << '%'+it->first+'%';
            QString key = it->first.left(it->first.indexOf('.'));
            mDescription << descript.value(key) + ": " + QString::number(it->second);
            ++it;
        }
        mType.insert(int(mData.size())-1, ccCtConst);
    }

    // execute additions
    src = syntax::SyntaxData::execute();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first;
        mDescription << it->second;
        mData << '.' + it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(int(mData.size())-1, ccExec);

    mData << "checkErrorLevel";
    mDescription << "Check errorLevel automatically after executing a GAMS tool";
    mData << ".checkErrorLevel";
    mDescription << "Check errorLevel automatically after executing a GAMS tool";
    mType.insert(int(mData.size())-1, ccExecT);

    mData << "noError";
    mDescription << "Abort without error";
    mData << ".noError";
    mDescription << "Abort without error";
    mType.insert(int(mData.size())-1, ccAbort);

    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "execute." + it->first;
        mDescription << it->second;
        ++it;
    }
    mData << "executeTool.checkErrorLevel";
    mDescription << "Check errorLevel automatically after executing a GAMS tool";
    mType.insert(int(mData.size())-1, ccRes);


    // add description index
    for (int i = 0; i < mData.size(); ++i) {
        mDescriptIndex << i;
    }
    mTempDataStart = int(mData.size());
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
            iType.insert(int(data.size())-1, mType.value(i));
        }
        if (i == mDollarGroupRow)
            mDollarGroupRow = int(data.size())-1;
        if (i == mPercentGroupRow)
            mPercentGroupRow = int(data.size())-1;
    }
    mData = data;
    mDescriptIndex = descriptIndex;
    mType = iType;
    mTempDataStart = int(mData.size());
}

void CodeCompleterModel::removeTempData(CodeCompleterType type)
{
    QPoint range = mTempDataIndicees.value(type, QPoint(-1,-1));
    if (range.x() < 0 || range.y() < 0) return;
    if (mData.size() <= range.x() || mData.size() <= range.y()) return;
    mTempDataIndicees.remove(type);

    QList<int> removedDescriptions;
    beginRemoveRows(QModelIndex(), range.x(), range.y());

    for (int i = range.y(); i >= range.x(); --i) { // backwards!
        mData.removeAt(i);
        if (!removedDescriptions.contains(mDescriptIndex.at(i)))
            removedDescriptions << mDescriptIndex.at(i);
        mDescriptIndex.removeAt(i);
    }
    for (const int &i : std::as_const(removedDescriptions)) {
        mDescription.removeAt(i);
    }
    QMap<int, CodeCompleterType> addType;
    QMap<int, CodeCompleterType>::iterator it = mType.find(range.y());
    if (it != mType.end())
        it = mType.erase(QMap<int, CodeCompleterType>::const_iterator(it));
    while (it != mType.end()) {
        addType.insert(it.key() - range.y() + range.x(), it.value());
        it = mType.erase(QMap<int, CodeCompleterType>::const_iterator(it));
    }
    mType.insert(addType);
    endRemoveRows();
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
    return int(mData.size());
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

void CodeCompleterModel::setActiveNameSuffix(const QString &suffix)
{
    if (mLastNameSuffix == suffix) return;
    removeTempData(ccSufName);

    mLastNameSuffix = suffix;
    QString suffName = suffix.right(suffix.length()-1);
    if (suffix.length() < 2) return;
    QStringList suffAssigns = mNameSuffixAssignments.value(suffix.at(0));
    if (suffAssigns.isEmpty()) return;

    QStringList addData;
    int lastAddedDescIndex = -1;
    for (int i = 0; i < mData.length(); ++i) {
        for (const QString &keyword: std::as_const(suffAssigns)) {
            if (mData.at(i).compare(keyword, Qt::CaseInsensitive) == 0) {
                addData << mData.at(i) + '.' + suffName;
                if (lastAddedDescIndex != i) {
                    mDescription << mDescription.at(mDescriptIndex.at(i)) + " (name: " +suffName+ ")";
                    lastAddedDescIndex = i;
                }
                mDescriptIndex << int(mDescription.size()) - 1;
                break;
            }
        }
    }
    mTempDataIndicees.insert(ccSufName, QPoint(int(mData.size()), int(mData.size() + addData.size()) - 1));
    beginInsertRows(QModelIndex(), int(mData.size()), int(mData.size() + addData.size()) - 1);
    for (int i = 0; i < addData.size(); ++i) {
        mData << addData.at(i);
    }
    mType.insert(int(mData.length()), ccSufName);
    endInsertRows();
}

bool CodeCompleterModel::hasActiveNameSuffix()
{
    return mLastNameSuffix.length() > 1;
}


// ----------- Filter ---------------

void FilterCompleterModel::setEmpty(bool isEmpty)
{
    mEmpty = isEmpty;
}

bool FilterCompleterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    int type = sourceModel()->data(index, Qt::UserRole).toInt();
    if (type & ccSufName) return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    if (!test(type, mTypeFilter)) return false;
    QString text = sourceModel()->data(index, Qt::DisplayRole).toString();
    if (type & cc_SubDco || type & ccExec || type & ccExecT || type & ccAbort) {
        if (text.startsWith('.') != mNeedDot)
            return false;
    }
    if (type == ccDcoEnd) {
        switch (mSubType) {
        case 1: if (!text.startsWith("$offtext", Qt::CaseInsensitive)) return false; break;
        case 2: if (!text.startsWith("$offecho", Qt::CaseInsensitive)) return false; break;
        case 3: if (!text.startsWith("$offput", Qt::CaseInsensitive)) return false; break;
        case 4: if (!text.startsWith("$offembeddedcode", Qt::CaseInsensitive)) return false; break;
        case 5: if (!text.startsWith("$endembeddedcode", Qt::CaseInsensitive) &&
                    !text.startsWith("$pauseembeddedcode", Qt::CaseInsensitive))
                return false;
            break;
        default: ;
        }
    }
//    if (type == ccResEnd) {
//        if (text.toLower() != "endembeddedcode" && text.toLower() != "pauseembeddedcode")
//            return false;
//    }
    if (mEmpty) {
        if (sourceRow == mDollarGroupRow || sourceRow == mPercentGroupRow)
            return true;
        else if ((text.startsWith('$') || text.startsWith('%')) && type != ccDcoEnd)
            return false;
    } else if (sourceRow == mDollarGroupRow || sourceRow == mPercentGroupRow)
        return false;

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

void FilterCompleterModel::setGroupRows(int dollarRow, int percentRow)
{
    mDollarGroupRow = dollarRow;
    mPercentGroupRow = percentRow;
}

bool FilterCompleterModel::isGroupRow(int row)
{
    QModelIndex ind = mapToSource(index(row, 0));
    return ind.row() == mDollarGroupRow || ind.row() == mPercentGroupRow;
}

bool FilterCompleterModel::test(int type, int flagPattern) const
{
    if (type & cc_SubDco && flagPattern & cc_SubDco) {
        if ((type & cc_SubDco) == (flagPattern & cc_SubDco))
            return true;
        flagPattern -= flagPattern & cc_SubDco;
    }
    return type & flagPattern;
}

void FilterCompleterModel::setTypeFilter(int completerTypeFilter, int subType, bool needDot)
{
    mTypeFilter = completerTypeFilter;
    mSubType = subType;
    mNeedDot = needDot;
    invalidateFilter();
}

bool FilterCompleterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int pat = source_left.data(Qt::UserRole).toInt() | source_right.data(Qt::UserRole).toInt();
    if ((pat & ccSufName) && pat != ccSufName) {
        return source_left.data(Qt::UserRole).toInt() == ccSufName;
    }
    if ((pat & ccDcoEnd) && pat != ccDcoEnd) {
        return source_left.data(Qt::UserRole).toInt() == ccDcoEnd;
    }
    if ((pat & ccResEnd) && pat != ccResEnd) {
        return source_left.data(Qt::UserRole).toInt() == ccResEnd;
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}


// ----------- Completer ---------------

CodeCompleter::CodeCompleter(QPlainTextEdit *parent) :
    QListView(nullptr),
    mEdit(parent),
    mModel(new CodeCompleterModel(this)),
    mFilterModel(new FilterCompleterModel(this))
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

void CodeCompleter::setCodeEdit(QPlainTextEdit *edit)
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
    if (mSuppressOpen) {
        if (mSuppressOpen == 1)
            mSuppressOpen = 0;
        return;
    }
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
        insertCurrent(e->key() == Qt::Key_Tab);
    }   break;
    case Qt::Key_Backspace: {
        qApp->sendEvent(mEdit, e);
        if (mFilterText.size() == 0) hide();
        else updateFilter();
    }   break;
    default: {
        if (e->key() == Qt::Key_Space)
            hide();
        if (mEdit)
            qApp->sendEvent(mEdit, e);
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
        if (mEdit) qApp->sendEvent(mEdit, e);
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
    clNumSym,
    clFix,
    clBreak,
    clSpace,
};

CharGroup group(const QChar &c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') return clAlpha;
    if (c >= '0' && c <= '9') return clNum;
    if (c == ':' || c == '.') return clNumSym;
    if (c == '$' || c == '%') return clFix;
    if (c == ' ' || c == '\t') return clSpace;
    return clBreak;
}

const QSet<int> CodeCompleter::cEnteringSyntax {
    int(syntax::SyntaxKind::IdentifierDescription),
    int(syntax::SyntaxKind::String)
};
const QSet<int> CodeCompleter::cExecuteSyntax {
    int(syntax::SyntaxKind::Execute),
    int(syntax::SyntaxKind::ExecuteKey),
    int(syntax::SyntaxKind::ExecuteBody),
    int(syntax::SyntaxKind::ExecuteTool),
    int(syntax::SyntaxKind::ExecuteToolKey),
    int(syntax::SyntaxKind::Abort),
    int(syntax::SyntaxKind::AbortKey),
};
const QSet<int> CodeCompleter::cExecuteToolSyntax {
};
const QSet<int> CodeCompleter::cBlockSyntax {
    int(syntax::SyntaxKind::CommentBlock),
    int(syntax::SyntaxKind::IgnoredBlock),
    int(syntax::SyntaxKind::EmbeddedBody)
};

QPair<int, int> CodeCompleter::getSyntax(const QTextBlock &block, int pos, int &dcoFlavor, int &dotPos)
{
    QString suffixName;
    emit syntaxFlagData(block, syntax::flagSuffixName, suffixName);
    mModel->setActiveNameSuffix(suffixName);
    QPair<int, int> res(0, 0);
    QMap<int, QPair<int, int>> blockSyntax;
    emit scanSyntax(block, blockSyntax);
    if (!blockSyntax.isEmpty()) res = blockSyntax.first();
    int lastEnd = 0;
    dotPos = -1;
    for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
        if (cExecuteSyntax.contains(it.value().first) && it.value().second % 2) {
            if ((res.first == int(syntax::SyntaxKind::Abort) || res.first == int(syntax::SyntaxKind::Execute)
                 || res.first == int(syntax::SyntaxKind::ExecuteTool)) && !(res.second % 2))
                dotPos = lastEnd;
        }
        if (it.key() >= pos) {
            if (cEnteringSyntax.contains(res.first)) {
                if (res.first == int(syntax::SyntaxKind::IdentifierDescription) && lastEnd == pos)
                    break;
                res = it.value();
            }
            if (cEnteringSyntax.contains(it.value().first) && lastEnd < pos)
                res = it.value();
            if (cBlockSyntax.contains(it.value().first) && lastEnd == pos)
                res = it.value();
            break;
        }
        lastEnd = it.key();
        res = it.value();
        if (syntax::SyntaxKind(res.first) == syntax::SyntaxKind::Dco)
            dcoFlavor = res.second;
    }

#ifdef COMPLETER_DEBUG
    if (mEdit) {
        if (block.isValid()) DEB() << "  mSynSim.clearBlockSyntax();";
        for (QMap<int,QPair<int, int> >::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
            if (block.isValid())
                DEB() << "  mSynSim.addBlockSyntax(" << it.key()
                      << ", SyntaxKind::" << syntax::syntaxKindName(it.value().first) << ", " << it.value().second << ");";
        }
        DEB() << "   <" << syntax::syntaxKindName(res.first) << ", " << res.second << ">";
    }
#endif

    return res;
}

void CodeCompleter::updateFilter(int posInBlock, QString line)
{
    if (!mEdit && posInBlock < 0) return;
    QTextBlock block;
    if (posInBlock < 0 && mEdit) {
        QTextCursor cur = mEdit->textCursor();
        block = cur.block();
        line = cur.block().text();
        posInBlock = cur.positionInBlock();
#ifdef COMPLETER_DEBUG
        QString debugLine = line;
        DEB() << "  // TEST: \n    line = \"" << debugLine.replace("\"", "\\\"") << "\";";
#endif
    }

    int peekStart = posInBlock;

    int dcoFlavor = 0;
    int dotPos;
    QPair<int,int> syntax = getSyntax(block, posInBlock, dcoFlavor, dotPos);

    int validStart = peekStart;
    while (peekStart > 0) {
        --peekStart;
        CharGroup cg = group(line.at(peekStart));
        if (cg >= clBreak) break;
        if (cg == clAlpha || cg == clNum) validStart = peekStart;
        if (cg == clNumSym) {
            if (cExecuteSyntax.contains(syntax.first) && dotPos >= 0) {
                validStart = peekStart+1;
                break;
            } else if (syntax.first == int(syntax::SyntaxKind::IdentifierAssignment)
                    || syntax.first == int(syntax::SyntaxKind::AssignmentLabel)
                    || syntax.first == int(syntax::SyntaxKind::AssignmentSystemData)) {
                if (line.at(peekStart) == '.') {
                    const QString sys("system.");
                    if (peekStart >= sys.length() && sys.compare(line.mid(validStart - sys.length(), sys.length()), Qt::CaseInsensitive) == 0) {
                        validStart -= sys.length();
                        if (validStart > 0 && line.at(validStart-1) == '%')
                            --validStart;
                    }

                    break;
                }
            } else
                validStart = peekStart;
        }
        if (cg == clFix) {
            validStart = peekStart;
            break;
        }
    }
    int len = posInBlock - validStart;
    if (!len) {
        mFilterText = "";
    } else {
        mFilterText = line.mid(validStart, len);
    }
    mFilterModel->setEmpty(!len);

    // assign filter
    if (mModel->casing() == caseDynamic && mFilterModel->filterCaseSensitivity() == Qt::CaseInsensitive)
         mFilterModel->setFilterCaseSensitivity(Qt::CaseSensitive);
    mFilterModel->setGroupRows(mModel->dollarGroupRow(), mModel->percentGroupRow());
    updateFilterFromSyntax(syntax, dcoFlavor, line, posInBlock);
    QString filterRex = QRegularExpression::escape(mFilterText);
    mFilterModel->setFilterRegularExpression('^'+filterRex+".*");

    if (mModel->casing() == caseDynamic && !mFilterModel->rowCount())
         mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    if (!mFilterModel->rowCount() ||
            (mFilterModel->rowCount() == 1 && mFilterModel->data(mFilterModel->index(0,0)).toString() == mFilterText)) {
        hide();
        return;
    }
    mFilterModel->sort(0);

    // find best index
    if (!mPreferredText.isEmpty()) {
        QModelIndex ind = mFilterModel->index(findFilterRow(mPreferredText, 0, mFilterModel->rowCount()-1), 0);
        if (ind.isValid())
            setCurrentIndex(ind);
    } else {
        int validEnd = posInBlock;
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
    }

    scrollTo(currentIndex());

    // adapt size
    if (!mEdit) return;
    QTextCursor cur = mEdit->textCursor();
    cur.setPosition(cur.position() - int(mFilterText.length()));
    QPoint pos = mEdit->cursorRect(cur).bottomLeft()
            + QPoint(mEdit->viewport()->contentsMargins().left(), mEdit->viewport()->contentsMargins().top());

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

int CodeCompleter::findFilterRow(const QString &text, int top, int bot)
{
    int ind = (top + bot) / 2;
    if (top == ind || bot == ind) return 0;
    QString str = model()->data(model()->index(ind, 0)).toString();
    int res = str.compare(text);
    if (res < 0) return findFilterRow(text, ind, bot);
    if (res > 0) return findFilterRow(text, top, ind);
    return ind;
}


void CodeCompleter::updateDynamicData(const QStringList &symbols)
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
    if (mSuppressOpen) {
        if (mSuppressOpen == 1)
            mSuppressOpen = 0;
        return;
    }
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

void CodeCompleter::setDebugMode(bool debug)
{
    mDebug = debug;
}

QString CodeCompleter::filterText() const
{
    return mFilterText;
}

int CodeCompleter::typeFilter() const
{
    return mFilterModel->typeFilter();
}

QStringList CodeCompleter::splitTypes(int filter)
{
    static const QMap<CodeCompleterType, QString> groupTypes {
        {cc_None,"cc_None"}, {cc_Dco,"cc_Dco"}, {cc_Res,"cc_Res"},
        {cc_Start,"cc_Start"}, {cc_All,"cc_All"},
    };
    static const QMap<CodeCompleterType, QString> baseTypes {
        {ccDcoStrt,"ccDcoStrt"}, {ccDcoEnd,"ccDcoEnd"},
        {ccSysDat,"ccSysDat"}, {ccSysSufR,"ccSysSufR"}, {ccSysSufC,"ccSysSufC"}, {ccCtConst,"ccCtConst"}, {ccDecl,"ccDecl"},
        {ccDeclAddS,"ccDeclAddS"}, {ccDeclAddS,"ccDeclAddV"}, {ccRes,"ccRes"}, {ccResEnd,"ccResEnd"}, {ccDeclS,"ccDeclS"},
        {ccDeclV,"ccDeclV"}, {ccDeclT,"ccDeclT"}, {ccOpt,"ccOpt"}, {ccMod,"ccMod"}, {ccSolve,"ccSolve"}, {ccExec,"ccExec"},
        {ccExecT,"ccExecT"}, {ccAbort,"ccAbort"},
    };
    QStringList res;
    int merge = 0;
    if (filter < 0) filter = typeFilter();

    for (QMap<CodeCompleterType, QString>::ConstIterator it = groupTypes.constBegin() ; it != groupTypes.constEnd() ; ++it) {
        if (filter == it.key()) {
            return QStringList() << it.value();
        }
        if (it == groupTypes.constBegin()) continue;
        if ((filter & it.key()) == it.key()) {
            res << it.value();
            merge = it.key();
        }
    }
    if (res.size() != 1) merge = 0;
    for (QMap<CodeCompleterType, QString>::ConstIterator it = baseTypes.constBegin() ; it != baseTypes.constEnd() ; ++it) {
        if (mFilterModel->test(it.key(), (filter))) {
            if (!merge || !(merge & it.key()))
                res << it.value();
        }
    }
    return res;
}

bool CodeCompleter::isOpenSuppressed()
{
    return mSuppressOpen;
}

void CodeCompleter::suppressOpenBegin()
{
    mSuppressOpen = 2;
}

void CodeCompleter::suppressOpenStop()
{
    mSuppressOpen = 0;
}

void CodeCompleter::suppressNextOpenTrigger()
{
    mSuppressOpen = 1;
}

void CodeCompleter::setVisible(bool visible)
{
    if (!visible) mPreferredText = QString();
    QListView::setVisible(visible);
}

void CodeCompleter::insertCurrent(bool equalPartOnly)
{
    if (!mEdit) return;
    bool hideIt = !equalPartOnly;
    if (currentIndex().isValid()) {
        QTextCursor cur = mEdit->textCursor();
        QString line = cur.block().text();
        cur.beginEditBlock();
        if (mFilterText.length())
            cur.setPosition(cur.position() - int(mFilterText.length()));
        int start = cur.positionInBlock();
        QString res = model()->data(currentIndex()).toString();
        if (mFilterModel->isGroupRow(currentIndex().row()) && !res.isEmpty())
            res = res.at(0);
        mPreferredText = res;

        if (equalPartOnly && res.length() > mFilterText.length()+1) {
            int pos = int(mFilterText.length());
            QString resTwo = res.mid(pos, 2);
            int indBefore = findBound(pos, resTwo, currentIndex().row(), 0);
            int indAfter = findBound(pos, resTwo, currentIndex().row(), rowCount()-1);
            QModelIndex fromInd = model()->index(indBefore, 0);
            QModelIndex toInd = model()->index(indAfter, 0);
            QString first = fromInd.isValid() ? model()->data(fromInd).toString() : res;
            QString last  = toInd.isValid() ? model()->data(toInd).toString() : res;
            int j = int(mFilterText.length());
            while (first.length() > j && last.length() > j && first.at(j).toLower() == last.at(j).toLower())
                ++j;
            if (j > mFilterText.length()) {
                res = res.left(j);
            }
        }
        int i = int(mFilterText.length());
        for ( ; i < res.length() && start+i < line.length(); ++i) {
            if (line.at(start+i).toLower() != res.at(i).toLower()) break;
        }
        if (i > 0)
            cur.setPosition(cur.position() + i, QTextCursor::KeepAnchor);

        if (res.length() == 1)
            hideIt = false;
        else
            hideIt = (res == mFilterText) || hideIt;
        cur.insertText(res);
        cur.endEditBlock();
        mEdit->setTextCursor(cur);
    }
    if (hideIt) {
        mEdit->setFocus();
        hide();
    }
}

int CodeCompleter::findBound(int pos, const QString &nextTwo, int good, int look)
{
    if (nextTwo.length() != 2) return -1;
    int ind = (good + look) / 2;
    if (good == ind) {
        if (good == look) return ind;
        ind = look;
    }
    QString str = model()->data(model()->index(ind, 0)).toString();
    if (str.length() > pos && str.mid(pos, 2).compare(nextTwo, Qt::CaseInsensitive) == 0)
        return findBound(pos, nextTwo, ind, look);
    if (ind == look) return ind;
    return findBound(pos, nextTwo, good, ind);
}

void CodeCompleter::updateFilterFromSyntax(const QPair<int, int> &syntax, int dcoFlavor, const QString &line, int pos)
{
    int filter = cc_All;
    qsizetype start = pos - mFilterText.length();
    bool needDot = false;
    syntax::SyntaxKind synKind = syntax::SyntaxKind(syntax.first);

    switch (synKind) {
    case syntax::SyntaxKind::Standard:
    case syntax::SyntaxKind::Formula:
    case syntax::SyntaxKind::Assignment:
    case syntax::SyntaxKind::Semicolon:
    case syntax::SyntaxKind::CommaIdent:
        filter = cc_Start; break;

    case syntax::SyntaxKind::SubDCO:
    case syntax::SyntaxKind::AssignmentSystemData:
    case syntax::SyntaxKind::SystemCompileAttrib:
    case syntax::SyntaxKind::SystemCompileAttribR:
    case syntax::SyntaxKind::UserCompileAttrib:
        filter = cc_None; break;

    case syntax::SyntaxKind::Declaration:  // [set parameter variable equation] allows table
        filter = ccDcoStrt | ccSysSufC | ccCtConst | (syntax.second == syntax::flavorAbort ? ccDeclT : 0); break;
    case syntax::SyntaxKind::DeclarationSetType:
        filter = ccDcoStrt | ccDeclS; break;
    case syntax::SyntaxKind::DeclarationVariableType:
        filter = ccDcoStrt | ccDeclV; break;

    case syntax::SyntaxKind::Dco:
        filter = ccDcoStrt | ccSysSufC | ccCtConst; break;

    case syntax::SyntaxKind::IgnoredHead:
    case syntax::SyntaxKind::IgnoredBlock:
    case syntax::SyntaxKind::DcoComment:
    case syntax::SyntaxKind::CommentBlock:
    case syntax::SyntaxKind::CommentLine:
    case syntax::SyntaxKind::CommentEndline:
    case syntax::SyntaxKind::CommentInline:
        filter = cc_None; break;

    case syntax::SyntaxKind::DcoBody:
    case syntax::SyntaxKind::Title:
    case syntax::SyntaxKind::String:
        filter = ccSysSufC | ccCtConst; break;

    case syntax::SyntaxKind::Identifier:
    case syntax::SyntaxKind::IdentifierDim:
    case syntax::SyntaxKind::IdentifierDimEnd:
    case syntax::SyntaxKind::IdentifierDescription:
    case syntax::SyntaxKind::AssignmentValue:
    case syntax::SyntaxKind::IdentifierAssignmentEnd:
    case syntax::SyntaxKind::IdentifierTableAssignmentColHead:
    case syntax::SyntaxKind::IdentifierTableAssignmentRowHead:
    case syntax::SyntaxKind::IdentifierTableAssignmentRow:
        filter = ccDcoStrt | ccSysSufC | ccCtConst; break;
    case syntax::SyntaxKind::IdentifierAssignment:
    case syntax::SyntaxKind::AssignmentLabel:
        filter = ccDcoStrt | ccSysDat | ccSysSufC | ccCtConst; break;

    case syntax::SyntaxKind::Embedded:
    case syntax::SyntaxKind::EmbeddedBody:
    case syntax::SyntaxKind::EmbeddedEnd:
        filter = (syntax.second) ? cc_None : ccResEnd; break;
    case syntax::SyntaxKind::Reserved:
    case syntax::SyntaxKind::Solve:
    case syntax::SyntaxKind::SolveBody:
    case syntax::SyntaxKind::SolveKey:
    case syntax::SyntaxKind::Put:
    case syntax::SyntaxKind::PutFormula:
        filter = cc_Start; break;
    case syntax::SyntaxKind::Abort:
    case syntax::SyntaxKind::AbortKey:
    case syntax::SyntaxKind::ExecuteTool:
    case syntax::SyntaxKind::ExecuteToolKey:
    case syntax::SyntaxKind::ExecuteKey:
    case syntax::SyntaxKind::ExecuteBody:
    case syntax::SyntaxKind::Execute: {
        int subFilter = (synKind == syntax::SyntaxKind::Abort || synKind == syntax::SyntaxKind::AbortKey)
                            ? ccAbort
                            :(synKind == syntax::SyntaxKind::ExecuteTool || synKind == syntax::SyntaxKind::ExecuteToolKey)
                                  ? ccExecT
                                  : ccExec;
        if (start < line.length() && line.at(start) == '.')
            needDot = true;
        else
            needDot = (syntax.second % 2 == 0);

        if ((synKind == syntax::SyntaxKind::ExecuteBody || synKind == syntax::SyntaxKind::Formula) && !needDot && !(syntax.second % 2))
            filter = ccDcoStrt | ccSysSufC | ccCtConst;
        else if (needDot)
            filter = cc_Start | subFilter;
        else
            filter = ccDcoStrt | ccSysSufC | ccCtConst | subFilter;
    }   break;

    case syntax::SyntaxKind::OptionKey:
    case syntax::SyntaxKind::Option:
    case syntax::SyntaxKind::OptionBody:
        filter = ccOpt | ccMod; break;
    default: ;
    }

    start = qMin(start, qMax(0, line.length()-1));
    bool isWhitespace = true;
    for (qsizetype i = 0; i < start; ++i) {
        if (line.at(i) != ' ' && line.at(i) != '\t') {
            isWhitespace = false;
            break;
        }
    }
    int dcoCheck = qMin(start+2, line.length());
    int dCount = 1;

    for (qsizetype i = 0; i < dcoCheck; ++i) {
        if (line.at(i) != ' ' && line.at(i) != '\t' && line.at(i) != '$') {
            dCount = 0;
            break;
        }
        if (line.at(i) == '$')
            ++dCount;
        else if (dCount > 0 && line.at(i) == '$')
            ++dCount;
        else if (dCount == 1)
            dCount = 0;
        if (dCount == 2)
            break;
    }

    int subType = 0;
    if (isWhitespace) {
        if (dCount != 2 && start > 0)
            filter = filter & ~cc_Dco;
        else if (synKind == syntax::SyntaxKind::CommentBlock) {
            filter = cc_Start | ccDcoEnd;
            subType = 1;
        } else if (synKind == syntax::SyntaxKind::IgnoredHead
                   || synKind == syntax::SyntaxKind::IgnoredBlock) {
            filter = cc_Start | ccDcoEnd;
            subType = syntax.second == syntax::flavorEcho1 ? 2 : 3;
        } else if (synKind == syntax::SyntaxKind::EmbeddedBody) {
            if (syntax.second == 0) {
                filter = cc_Start | ccResEnd;
            } else {
                filter = cc_Start | ccDcoEnd;
                subType = (syntax.second == syntax::flavorEmbed1) ? 4 : 5;
            }
        } else if (!mFilterModel->test(filter, cc_Dco))
            filter = filter & ccDcoStrt;
    } else if (dcoFlavor >= syntax::flavorAbort) {
        needDot = true;
        for (qsizetype i = start; i > 0; --i) {
            if (needDot && line.at(i) == '.') {
                needDot = false;
            } else {
                CharGroup gr = group(line.at(i));
                if (gr >= clBreak) {
                    filter = filter & ~cc_Dco;
                    break;
                }
            }
        }
        if (dcoFlavor >= syntax::flavorAbort && dcoFlavor <= syntax::flavorEval)
            filter = ccSysSufC | ccCtConst;
        else
            filter = filter & ~cc_Dco;
    } else if (dCount != 2  && start > 0) {
        filter = filter & ~cc_Dco;
    }

    if (mDebug) {
        // for analysis
#ifdef COMPLETER_DEBUG
        DEB() << " -> " << start << ": " << syntax::syntaxKindName(syntax.first) << "," << syntax.second
              << "   filter: " << QString::number(filter, 16) << " [" << splitTypes(filter).join(",") << "]";
        DEB() << "--- Line: \"" << line << "\"   start:" << start << " pos:" << pos;
#endif
        QString debugText = "Completer at " + QString::number(start) + ": "
                            + syntax::syntaxKindName(synKind) + "[" + QString::number(syntax.second)
                            + "], filters " + QString::number(filter, 16);
        if (SysLogLocator::systemLog())
            SysLogLocator::systemLog()->append(debugText, LogMsgType::Info);
    }
    if (mModel->hasActiveNameSuffix())
        filter |= ccSufName;
    mFilterModel->setTypeFilter(filter, subType, needDot);
}

} // namespace studio
} // namespace gams
