/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2018 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2018 GAMS Development Corp. <support@gams.com>
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
    int locCol() const;
    QString locFile() const;
    QString context() const;

private:
    int mLocLineNr;
    int mLocCol;
    QString mLocFile;
    QString mContext;
    explicit Result(int locLineNr, int locCol, QString locFile, QString context = "");
};

class SearchResultList : public QAbstractTableModel
{
    Q_OBJECT
public:
    SearchResultList(SearchResultList &searchResultList);
    SearchResultList(const QString &searchTerm, QObject *parent = nullptr);
    virtual ~SearchResultList();
    QList<Result> resultList();
    void addResult(int locLineNr, int locCol, QString locFile, QString context = "");
    void addResultList(QList<Result> resList);
    QString searchTerm() const;
    bool isRegex() const;
    void useRegex(bool regex);
    int size();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QString mSearchTerm;
    bool mIsRegex;
    QList<Result> mResultList;
};

}
}
#endif // SEARCHRESULTLIST_H
