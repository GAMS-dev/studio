#ifndef GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
#define GAMS_STUDIO_SYNTAX_CODECOMPLETER_H

#include <QListView>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

namespace gams {
namespace studio {

enum CodeCompleterType {
    ccNone = 0x00000000,
    ccDco1 = 0x00000001, // DCO (starter and standalone)
    ccDco2 = 0x00000002, // DCO offText
    ccDco  = 0x000000FF, // all DCOs

    ccRes1 = 0x00000100, // declarations (starter and standalone)
    ccRes2 = 0x00000200, // declaration "set"
    ccRes3 = 0x00000400, // declaration "variable"
    ccRes  = 0x0000FF00, // all declarations

    ccOpt  = 0x00010000, // options
    ccMod  = 0x00020000, // models
};

class CodeCompleterModel : public QAbstractListModel
{
    QStringList mData;
    QStringList mDescription;
    QMap<int, CodeCompleterType> mType;
    Q_OBJECT
public:
    CodeCompleterModel(QObject *parent = nullptr);
    ~CodeCompleterModel() override {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

};

class FilterCompleterModel : public QSortFilterProxyModel
{
    int mTypeFilter = 0;
    Q_OBJECT
public:
    FilterCompleterModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}
     ~FilterCompleterModel() override {}
    QVariant data(const QModelIndex &index, int role) const override;
    void setFilter(int completerTypeFilter);
};

class CodeEdit;

class CodeCompleter : public QListView
{
    Q_OBJECT

public:
    CodeCompleter(CodeEdit *parent);
    ~CodeCompleter() override;
    void updateFilter();
    void updateDynamicData(QStringList symbols);
    int rowCount();

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void insertCurrent();

private:
    CodeEdit *mEdit;
    CodeCompleterModel *mModel;
    FilterCompleterModel *mFilterModel;
    QString mFilterText;

};

} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
