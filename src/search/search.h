/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
#ifndef SEARCH_H
#define SEARCH_H

#include <QObject>
#include <mainwindow.h>

#include <file/filemeta.h>

namespace gams {
namespace studio {
namespace search {

class Search : public QObject
{
    Q_OBJECT
public:
    enum SearchScope {
        ThisFile = 0,
        ThisGroup= 1,
        OpenTabs = 2,
        AllFiles = 3
    };

    enum SearchStatus {
        Searching = 0,
        NoResults = 1,
        Clear = 2,
        Replacing = 4
    };

    enum SearchDirection {
        Forward = 0,
        Backward = 1
    };

    explicit Search(QList<FileMeta*> files, QRegularExpression regex, QFlags<QTextDocument::FindFlag>);
    ~Search();
    void start();

signals:
    void resultReady();

private:
    Result findInFiles();
    void findInDoc(FileMeta* fm);
    void findNext(SearchDirection direction, bool ignoreReadOnly = false);
    void selectNextMatch(SearchDirection direction, bool firstLevel = true);
    void findOnDisk(QRegularExpression searchRegex, FileMeta *fm, SearchResultList* collection);

//    void replaceNext();
//    void replaceAll();
    int replaceOpened(FileMeta* fm, QRegularExpression regex, QString replaceTerm, QFlags<QTextDocument::FindFlag> flags);
    int replaceUnopened(FileMeta* fm, QRegularExpression regex, QString replaceTerm);

private slots:
    void finished();

private:
    MainWindow *mMain;
    SearchResultList *mResults = nullptr;
    QList<FileMeta*> mFiles;
    QRegularExpression mRegex;
    QFlags<QTextDocument::FindFlag> mOptions;
    bool isRunning = false;

    QThread mThread;
    bool mSearching = false;
    bool mOutsideOfList = true;

    bool mSplitSearchContinue = false;
};

}
}
}

#endif // SEARCH_H
