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
    QList<QList<QPair<QString, QString>>::ConstIterator> delayedIterators;

    // DCOs
    mDollarGroupRow = mData.size();
    mData << "$...";
    mDescription << "";
    QList<QPair<QString, QString>> src = syntax::SyntaxData::directives();
    QList<QPair<QString, QString>>::ConstIterator it = src.constBegin();
    while (it != src.constEnd()) {
        if (it->first == "offText" || it->first == "offPut" ||
                it->first == "offEmbeddedCode" || it->first == "endEmbeddedCode" || it->first == "pauseEmbeddedCode")
            delayedIterators << it;
        else {
            mData << '$' + it->first;
            mDescription << it->second;
        }
        ++it;
    }
    mType.insert(mData.size()-1, ccDcoStrt);
    for (const QList<QPair<QString, QString>>::ConstIterator &it : qAsConst(delayedIterators)) {
        mData << '$' + it->first;
        mDescription << it->second;
    }
    mType.insert(mData.size()-1, ccDcoEnd);

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
            mType.insert(mData.size()-1, ccDecl);
        } else if (it->first.startsWith("Parameter")) {
            if (it->first.endsWith("s")) {
                mData << "Parameter Table";
                mDescription << it->second;
            }
            mType.insert(mData.size()-1, ccDecl);
        } else if (it->first.startsWith("Set")) {
            mType.insert(mData.size()-1, ccDeclS);
            if (it->first.endsWith("s")) {
                mData << "Set Table";
                mDescription << it->second;
            }
            mType.insert(mData.size()-1, ccDecl);
        } else if (it->first.startsWith("Variable")) {
            if (it->first.endsWith("s")) {
                mData << "Variable Table";
                mDescription << it->second;
            }
            mType.insert(mData.size()-1, ccDeclV);
        } else if (it->first == "Table") {
            mType.insert(mData.size()-1, ccDeclT);
        } else {
            mType.insert(mData.size()-1, ccDecl);
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
    mType.insert(mData.size()-1, ccDeclAddV);
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + " Variable Table" << it->first + " Variables Table";
        mDescription << it->second << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccDeclAddV);
    src = syntax::SyntaxData::declaration4Set();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + " Set" << it->first + " Sets";
        mDescription << it->second << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccDeclAddS);

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
    // execute
    src = syntax::SyntaxData::keyExecute();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccRes);

    // embedded end
    src = syntax::SyntaxData::embeddedEnd();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccResEnd);

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
    mType.insert(mData.size()-1, ccDcoStrt);

    mData << ".set";
    mDescription << "compile-time variable based on a GAMS set";
    mType.insert(mData.size()-1, ccSubDcoE);
    mData << "$eval.set";
    mDescription << "compile-time variable based on a GAMS set";
    mData << "$evalGlobal.set";
    mDescription << "compile-time variable based on a GAMS set";
    mData << "$evalLocal.set";
    mDescription << "compile-time variable based on a GAMS set";
    mType.insert(mData.size()-1, ccDcoStrt);

    mData << "noError";
    mDescription << "abort without error";
    mData << ".noError";
    mDescription << "abort without error";
    mType.insert(mData.size()-1, ccSubDcoA);
    mData << "$abort.noError";
    mDescription << "abort without error";
    mType.insert(mData.size()-1, ccDcoStrt);

    // system data
    src = syntax::SyntaxData::systemData();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "system." + it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccSysDat);

    // system data
    src = syntax::SyntaxData::systemAttributes();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "system." + it->first;
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccSysSufR);

    mPercentGroupRow = mData.size();
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
    mType.insert(mData.size()-1, ccSysSufC);

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
        mType.insert(mData.size()-1, ccCtConst);
    }

    // execute additions
    src = syntax::SyntaxData::execute();
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << it->first + ' ';
        mDescription << it->second;
        mData << '.' + it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccExec);
    it = src.constBegin();
    while (it != src.constEnd()) {
        mData << "execute." + it->first + ' ';
        mDescription << it->second;
        ++it;
    }
    mType.insert(mData.size()-1, ccRes);


    // add description index
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
        if (i == mDollarGroupRow)
            mDollarGroupRow = data.size()-1;
        if (i == mPercentGroupRow)
            mPercentGroupRow = data.size()-1;
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

void FilterCompleterModel::setEmpty(bool isEmpty)
{
    mEmpty = isEmpty;
}

bool FilterCompleterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    int type = sourceModel()->data(index, Qt::UserRole).toInt();
    if (!test(type, mTypeFilter)) return false;
    QString text = sourceModel()->data(index, Qt::DisplayRole).toString();
    if (type & cc_SubDco || type & ccExec) {
        if (text.startsWith('.') != mNeedDot)
            return false;
    }
    if (type == ccDcoEnd) {
        switch (mSubType) {
        case 1: if (text.toLower() != "$offtext") return false; break;
        case 2: if (text.toLower() != "$offput") return false; break;
        case 3: if (text.toLower() != "$offembeddedcode") return false; break;
        case 4: if (text.toLower() != "$endembeddedcode" && text.toLower() != "$pauseembeddedcode") return false; break;
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
//    DEB() << "NeedDot: " << needDot;
    invalidateFilter();
}


// ----------- Completer ---------------

CodeCompleter::CodeCompleter(QPlainTextEdit *parent) :
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
    default: {
        if (e->key() == Qt::Key_Space)
            hide();
        if (mEdit)
            qApp->sendEvent(mEdit, e);
//            mEdit->keyPressEvent(e);
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
        if (mEdit) qApp->sendEvent(mEdit, e); // mEdit->keyReleaseEvent(e);
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
    int(syntax::SyntaxKind::ExecuteBody)
};

QPair<int, int> CodeCompleter::getSyntax(QTextBlock block, int pos, int &dcoFlavor, int &dotPos)
{
    QPair<int, int> res(0, 0);
    QMap<int, QPair<int, int>> blockSyntax;
    emit scanSyntax(block, blockSyntax);
    int lastEnd = 0;
    dotPos = -1;
    for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
        if (cExecuteSyntax.contains(it.value().first) && it.value().second % 2) {
            if (res.first == int(syntax::SyntaxKind::Execute) && !(res.second % 2))
                dotPos = lastEnd;
        }
        if (it.key() > pos) {
            if (cEnteringSyntax.contains(res.first)) {
                if (res.first == int(syntax::SyntaxKind::IdentifierDescription) && lastEnd == pos)
                    break;
                res = it.value();
            }
            if (cEnteringSyntax.contains(it.value().first) && lastEnd < pos)
                res = it.value();
            break;
        }
        lastEnd = it.key();
        res = it.value();
        if (syntax::SyntaxKind(res.first) == syntax::SyntaxKind::Dco)
            dcoFlavor = res.second;
    }

#ifdef QT_DEBUG
    // uncomment this to generate elements for testcompleter
//    if (mEdit) {
//        if (block.isValid()) DEB() << "    mSynSim.clearBlockSyntax();";
//        for (QMap<int,QPair<int, int>>::ConstIterator it = blockSyntax.constBegin(); it != blockSyntax.constEnd(); ++it) {
//            if (block.isValid())
//                DEB() << "    mSynSim.addBlockSyntax(" << it.key()
//                      << ", SyntaxKind::" << syntax::syntaxKindName(it.value().first) << ", " << it.value().second << ");";
//        }
//    }
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
        // uncomment this to generate elements for testcompleter
#ifdef QT_DEBUG
//        QString debugLine = line;
//        DEB() << "    // TEST: \n    line = \"" << debugLine.replace("\"", "\\\"") << "\";";
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
    QString filterRex = mFilterText;
    filterRex.replace(".", "\\.").replace("$", "\\$");
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
    cur.setPosition(cur.position() - mFilterText.length());
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
        {cc_None,"cc_None"}, {cc_Dco,"cc_Dco"}, {cc_SubDco,"ccSubDco"}, {cc_Res,"cc_Res"},
        {cc_Start,"cc_Start"}, {cc_All,"cc_All"},
    };
    static const QMap<CodeCompleterType, QString> baseTypes {
        {ccDcoStrt,"ccDcoStrt"}, {ccDcoEnd,"ccDcoEnd"}, {ccSubDcoA,"ccSubDcoA"}, {ccSubDcoC,"ccSubDcoC"}, {ccSubDcoE,"ccSubDcoE"},
        {ccSysDat,"ccSysDat"}, {ccSysSufR,"ccSysSufR"}, {ccSysSufC,"ccSysSufC"}, {ccCtConst,"ccCtConst"}, {ccDecl,"ccDecl"},
        {ccDeclAddS,"ccDeclAddS"}, {ccDeclAddS,"ccDeclAddV"}, {ccRes,"ccRes"}, {ccResEnd,"ccResEnd"}, {ccDeclS,"ccDeclS"},
        {ccDeclV,"ccDeclV"}, {ccDeclT,"ccDeclT"}, {ccOpt,"ccOpt"}, {ccMod,"ccMod"}, {ccSolve,"ccSolve"}, {ccExec,"ccExec"}
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
            cur.setPosition(cur.position()-mFilterText.length());
        int start = cur.positionInBlock();
        QString res = model()->data(currentIndex()).toString();
        if (mFilterModel->isGroupRow(currentIndex().row()) && !res.isEmpty())
            res = res.at(0);
        mPreferredText = res;

        if (equalPartOnly && res.length() > mFilterText.length()+1) {
            int pos = mFilterText.length();
            QString resTwo = res.mid(pos, 2);
            int indBefore = findBound(pos, resTwo, currentIndex().row(), 0);
            int indAfter = findBound(pos, resTwo, currentIndex().row(), rowCount()-1);
            QModelIndex fromInd = model()->index(indBefore, 0);
            QModelIndex toInd = model()->index(indAfter, 0);
            QString first = fromInd.isValid() ? model()->data(fromInd).toString() : res;
            QString last  = toInd.isValid() ? model()->data(toInd).toString() : res;
            int j = mFilterText.length();
            while (first.length() > j && last.length() > j && first.at(j).toLower() == last.at(j).toLower())
                ++j;
            if (j > mFilterText.length()) {
                res = res.left(j);
            }
        }
        int i = mFilterText.length();
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
    if (str.length() > pos && str.midRef(pos, 2).compare(nextTwo, Qt::CaseInsensitive) == 0)
        return findBound(pos, nextTwo, ind, look);
    if (ind == look) return ind;
    return findBound(pos, nextTwo, good, ind);
}

void CodeCompleter::updateFilterFromSyntax(const QPair<int, int> &syntax, int dcoFlavor, const QString &line, int pos)
{
    int filter = cc_All;
    int start = pos - mFilterText.length();
    bool needDot = false;

    switch (syntax::SyntaxKind(syntax.first)) {
    case syntax::SyntaxKind::Standard:
    case syntax::SyntaxKind::Formula:
    case syntax::SyntaxKind::Assignment:
    case syntax::SyntaxKind::IgnoredHead:
    case syntax::SyntaxKind::IgnoredBlock:
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
        filter = ((syntax.second == 16) ? ccDcoStrt | ccDeclT : ccDcoStrt) | ccSysSufC | ccCtConst; break;
    case syntax::SyntaxKind::DeclarationSetType:
        filter = ccDcoStrt | ccDeclS; break;
    case syntax::SyntaxKind::DeclarationVariableType:
        filter = ccDcoStrt | ccDeclV; break;

    case syntax::SyntaxKind::Dco:
        filter = ccDcoStrt | ccSysSufC | ccCtConst; break;

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
    case syntax::SyntaxKind::ExecuteKey:
    case syntax::SyntaxKind::ExecuteBody:
    case syntax::SyntaxKind::Execute: {
        if (start < line.length() && line.at(start) == '.')
            needDot = true;
        else
            needDot = (syntax.second % 2 == 0);
        if (syntax::SyntaxKind(syntax.first) == syntax::SyntaxKind::ExecuteBody && !needDot && !(syntax.second % 2))
            filter = ccDcoStrt | ccSysSufC | ccCtConst;
        else if (needDot)
            filter = cc_Start | ccExec;
        else
            filter = ccDcoStrt | ccSysSufC | ccCtConst | ccExec;
    }   break;

    case syntax::SyntaxKind::OptionKey:
    case syntax::SyntaxKind::Option:
    case syntax::SyntaxKind::OptionBody:
        filter = ccOpt | ccMod; break;
    default: ;
    }

    start = qMin(start, qMax(0, line.length()-1));
    bool isWhitespace = true;
    for (int i = 0; i < start; ++i) {
        if (line.at(i) != ' ' && line.at(i) != '\t') {
            isWhitespace = false;
            break;
        }
    }
    int subType = 0;
    if (isWhitespace) {
        if (syntax::SyntaxKind(syntax.first) == syntax::SyntaxKind::CommentBlock) {
            filter = ccDcoEnd;
            subType = 1;
        } else if (syntax::SyntaxKind(syntax.first) == syntax::SyntaxKind::IgnoredBlock && syntax.second == 5) {
            filter = ccDcoEnd;
            subType = 2;
        } else if (syntax::SyntaxKind(syntax.first) == syntax::SyntaxKind::EmbeddedBody) {
            if (syntax.second == 0) {
                filter = ccResEnd;
            } else {
                filter = ccDcoEnd;
                subType = (syntax.second == 19) ? 3 : 4;
            }
        } else if (!mFilterModel->test(filter, cc_Dco))
            filter = filter & ccDcoStrt;
    } else if (dcoFlavor > 15 ) {
        needDot = true;
        for (int i = start; i > 0; --i) {
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
        if (dcoFlavor == 16)
            filter = ccSubDcoA | ccSysSufC | ccCtConst;
        else if (dcoFlavor == 17)
            filter = ccSubDcoC | ccSysSufC | ccCtConst;
        else if (dcoFlavor == 18)
            filter = ccSubDcoE | ccSysSufC | ccCtConst;
        else
            filter = filter & ~cc_Dco;
    } else {
        filter = filter & ~cc_Dco;
    }


    if (mDebug) {
        // for analysis
#ifdef QT_DEBUG
//        DEB() << " -> " << start << ": " << syntax::syntaxKindName(syntax.first) << "," << syntax.second
//              << "   filter: " << QString::number(filter, 16) << " [" << splitTypes(filter).join(",") << "]";
//        DEB() << "--- Line: \"" << line << "\"   start:" << start << " pos:" << pos;
#endif
        QString debugText = "Completer at " + QString::number(start) + ": "
                + syntax::syntaxKindName(syntax::SyntaxKind(syntax.first)) + "[" + QString::number(syntax.second)
                + "], filters " + QString::number(filter, 16);
        if (SysLogLocator::systemLog())
            SysLogLocator::systemLog()->append(debugText, LogMsgType::Info);
    }
    mFilterModel->setTypeFilter(filter, subType, needDot);
}

} // namespace studio
} // namespace gams
