#ifndef GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
#define GAMS_STUDIO_SYNTAX_CODECOMPLETER_H

#include <QListView>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

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
    ccDco1      = 0x00000001, // DCO (starter and standalone)
    ccDco2      = 0x00000002, // DCO $offText
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

    ccStart     = 0x7FF0F7FD, // all starting keywords

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

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void focusOutEvent(QFocusEvent *event) override;
    void actionEvent(QActionEvent *event) override;

private:
    void insertCurrent();
    int getFilterFromSyntax();

private:
    CodeEdit *mEdit;
    CodeCompleterModel *mModel;
    FilterCompleterModel *mFilterModel;
    QString mFilterText;
    bool mNeedDot = false;
};

} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
