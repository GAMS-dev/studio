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
#ifndef FILEWORKER_H
#define FILEWORKER_H

#include <QObject>
#include "search/abstractsearchfilehandler.h"
#include "searchhelpers.h"

namespace gams {
namespace studio {
namespace search {

///
/// \brief The FileWorker class
/// The FileWorker is used to handle collecting, sorting and filtering of files to be searched.
/// It can be used either synchronously or asynchronously, depending on the use case.
/// Use setParameters to pass all search options either by manually creating a SearchParameters object
/// or by using SearchDialog::createSearchParameters to extract everything necessary from the Search Dialog  .
/// Connect the filesCollected signal to further handle the file list once everything is done.
///
class FileWorker : public QObject
{
    Q_OBJECT

public:
    FileWorker(AbstractSearchFileHandler *fh);
    FileWorker(const SearchParameters &params, AbstractSearchFileHandler *fh);
    void setParameters(const SearchParameters &parameters);
    QList<SearchFile> collectFilesInFolder();
    QList<SearchFile> filterFiles(const QList<SearchFile> &files, const SearchParameters &params);

signals:
    void filesCollected(QList<gams::studio::search::SearchFile> files);

private:
    SearchParameters mParameters;
    AbstractSearchFileHandler* mFileHandler;
};

}
}
}

#endif // FILEWORKER_H
