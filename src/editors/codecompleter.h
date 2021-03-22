#ifndef GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
#define GAMS_STUDIO_SYNTAX_CODECOMPLETER_H

#include <QListView>
#include <QAbstractListModel>

class QSortFilterProxyModel;

namespace gams {
namespace studio {

class CodeEdit;
class CodeCompleterModel;

class CodeCompleter : public QListView
{
    Q_OBJECT

public:
    CodeCompleter(CodeEdit *parent);
    ~CodeCompleter() override;

signals:
    void requestData(QStringList &data);

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    CodeEdit *mEdit;
    CodeCompleterModel *mModel;
    QSortFilterProxyModel *mFilter;

};

class CodeCompleterModel : public QAbstractListModel
{
    QHash<Qt::ItemDataRole, QList<QPair<QString, QString>>> mData;
    Q_OBJECT
public:
    CodeCompleterModel(QObject *parent = nullptr);
    ~CodeCompleterModel() override {}
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

};

} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
