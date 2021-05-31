#ifndef GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
#define GAMS_STUDIO_SYNTAX_CODECOMPLETER_H

#include <QListView>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QTextBlock>

namespace gams {
namespace studio {

enum CodeCompleterCasing {
    caseCamel,
    caseLower,
    caseUpper,
    caseDynamic
};

enum CodeCompleterType {
    ccNone      = 0x00000000,

    ccBitFlags  = 0x7FFFFF00, // [Prepare] BIT filter for all flags
    ccBitValue  = 0x000000FF, // [Prepare] BIT filter for the int value part

    // TODO(JM)  Reduce bit-consumption by reserving the lowest byte as normal int value (the current DCO bits). The
    //   |       bit-filter then is only applied to the remain (see ccBitFlags and ccBitValue above). Some flags can
    //   V       can be combined and get a different value part to distinct them (like starter and ender DCOs)

    ccDco1      = 0x00000001, // DCO (starter and standalone)
    ccDco2      = 0x00000002, // DCO (ender, e.g. $offText)
    ccSubDcoA   = 0x00000010, // sub DCO of $abort
    ccSubDcoC   = 0x00000020, // sub DCO of $call
    ccSubDcoE   = 0x00000040, // sub DCO of $eval
    ccDco       = 0x000000FF, // all DCOs
    ccSubDco    = 0x000000F0, // all sub DCOs
    ccNoDco     = 0x7FFFFF00, // no DCOs

    ccRes1      = 0x00000100, // declarations
    ccResS      = 0x00000200, // declaration: Set
    ccResV      = 0x00000400, // declaration: Variable
    ccResT      = 0x00000800, // declaration: Table
    ccRes2      = 0x00001000, // declaration additions for "variable" and "set"
    ccRes3      = 0x00002000, // other reserved words
    ccRes4      = 0x00004000, // embedded end
    ccRes       = 0x0000FF00, // all declarations

    ccOpt       = 0x00010000, // options
    ccMod       = 0x00020000, // models
    ccSolve     = 0x00040000, // solve
    ccExec      = 0x00080000, // execute additions

    ccSysDat    = 0x00100000, // system data
    ccSysSufC   = 0x00200000, // system suffix compile-time
    ccSysSufR   = 0x00400000, // system suffix run-time

    ccStart     = 0x7F00F7FD, // all starting keywords

    ccAll       = 0x7FFFFFFF
};

class CodeCompleterModel : public QAbstractListModel
{
    QStringList mData;
    QStringList mDescription;
    QList<int> mDescriptIndex;
    QMap<int, CodeCompleterType> mType;
    CodeCompleterCasing mCasing;
    Q_OBJECT
public:
    CodeCompleterModel(QObject *parent = nullptr);
    ~CodeCompleterModel() override {}
    void setCasing(CodeCompleterCasing casing);
    CodeCompleterCasing casing() { return mCasing; }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    void initData();
    void addDynamicData();
};

class FilterCompleterModel : public QSortFilterProxyModel
{
    QString mFilterText;
    int mTypeFilter = 0;
    bool mNeedDot = true;
    Q_OBJECT
public:
    FilterCompleterModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}
     ~FilterCompleterModel() override {}
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setTypeFilter(int completerTypeFilter, bool needDot);
    void setFilterText(QString filterText);
};

class CodeEdit;

class CodeCompleter : public QListView
{
    Q_OBJECT
public:
    CodeCompleter(CodeEdit *parent = nullptr);
    ~CodeCompleter() override;
    void setCodeEdit(CodeEdit *edit);
    void updateFilter();
    void updateDynamicData(QStringList symbols);
    int rowCount();
    void ShowIfData();
    void setCasing(CodeCompleterCasing casing);

public slots:
    void setVisible(bool visible) override;

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void focusOutEvent(QFocusEvent *event) override;
    void actionEvent(QActionEvent *event) override;

private:
    void insertCurrent(bool equalPartOnly = false);
    int findBound(int pos, const QString &nextTwo, int good, int look);
    int findFilterRow(const QString &text, int top, int bot);
    int getFilterFromSyntax(const QPair<int, int> &syntax, int dcoFlavor);
    QPair<int, int> getSyntax(QTextBlock block, int pos, int &dcoFlavor);

private:
    CodeEdit *mEdit;
    CodeCompleterModel *mModel;
    FilterCompleterModel *mFilterModel;
    QString mFilterText;
    QString mPreferredText;
    bool mNeedDot = false;
};

} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
