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
#ifndef SEARCH_H
#define SEARCH_H

#include <QObject>
#include "file/filemeta.h"
#include "searchhelpers.h"
#include "searchresultmodel.h"

namespace gams {
namespace studio {
namespace search {

class SearchDialog;
class AbstractSearchFileHandler;

class Search : public QObject
{
    Q_OBJECT

public:
    enum Status {
        Searching,
        NoResults,
        Clear,
        Replacing,
        InvalidPath,
        EmptySearchTerm,
        CollectingFiles,
        InvalidRegex,
        Ok
    };

    enum Direction {
        Forward,
        Backward
    };

    Search(SearchDialog *sd, AbstractSearchFileHandler *fileHandler);
    ~Search();
    Search(const Search &) = delete;
    Search(Search &&) = delete;
    Search& operator=(const Search &) = delete;
    Search& operator=(Search &&) = delete;

    void start(const SearchParameters &parameters);
    void runSearch(QList<SearchFile> files);
    void requestStop();

    void findNext(Direction direction);
    void replaceNext(const QString& replacementText);
    void replaceAll(SearchParameters parameters);
    void selectNextMatch(Direction direction = Direction::Forward, bool firstLevel = true);

    bool isSearching() const;
    QList<Result> results() const;
    QList<Result> filteredResultList(const QString &fileLocation);
    bool caseSensitive() const;
    const QRegularExpression regex() const;
    Scope scope() const;
    bool hasSearchSelection();
    void reset();
    void invalidateCache();
    void resetResults();
    void activeFileChanged();
    bool hasCache() const;

signals:
    void invalidateResults();
    void updateUI();
    void selectResult(int matchNr);

private:

    void findInDoc(FileMeta* fm);
    void findInSelection(bool showResults);
    void findOnDisk(QRegularExpression searchRegex, FileMeta *fm, SearchResultModel* collection);

    int replaceOpened(FileMeta* fm, const SearchParameters &parameters);
    int replaceUnopened(FileMeta* fm, const SearchParameters &parameters);

    QPair<int, int> cursorPosition();
    int findNextEntryInCache(Search::Direction direction);

    int NavigateOutsideCache(Direction direction, bool firstLevel);
    int NavigateInsideCache(Direction direction);

    void checkFileChanged(const FileId &fileId);
    bool hasResultsForFile(const QString &filePath);

    QFlags<QTextDocument::FindFlag> createFindFlags(const SearchParameters &parameters,
                                                    Direction direction = Direction::Forward);

private slots:
    void finished();

private:
    SearchDialog* mSearchDialog;
    QList<Result> mResults;
    QHash<QString, QList<Result>> mResultHash;
    SearchParameters mLastSearchParameters;

    AbstractSearchFileHandler* mFileHandler;
    FileId mSearchSelectionFile;
    QThread mFileThread;
    QThread mSearchThread;

    bool mSearching = false;
    bool mJumpQueued = false;
    bool mCacheAvailable = false;
    bool mOutsideOfList = false;
    int mLastMatchInOpt = -1;
    bool mStopRequested = false;

    bool mSplitSearchContinue = false;
};

}
}
}

#endif // SEARCH_H
