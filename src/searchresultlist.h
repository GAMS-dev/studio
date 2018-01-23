#ifndef SEARCHRESULTLIST_H
#define SEARCHRESULTLIST_H

#include <QtWidgets>
#include <QAbstractTableModel>

namespace gams {
namespace studio {

class Result
{
    friend class SearchResultList;
public:
    int locLineNr() const;
    QString locFile() const;
    QString context() const;

private:
    int mLocLineNr;
    QString mLocFile;
    QString mContext;
    explicit Result(int locLineNr, QString locFile, QString context = "");
};

class SearchResultList : public QAbstractTableModel
{
    Q_OBJECT
public:
    SearchResultList(SearchResultList &searchResultList);
    SearchResultList(const QString &searchTerm, QObject *parent = nullptr);
    virtual ~SearchResultList();
    QList<Result> resultList();
    void addResult(int locLineNr, QString locFile, QString context = "");
    void addResultList(QList<Result> resList);
    QString searchTerm() const;
    bool isRegex() const;
    void useRegex(bool regex);
    int size();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QString mSearchTerm;
    bool mIsRegex;
    QList<Result> mResultList;
};

}
}
#endif // SEARCHRESULTLIST_H
