/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
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
#ifndef SEARCHCOMMON_H
#define SEARCHCOMMON_H

#include <QRegularExpression>
#include <QString>
#include <QStringList>

namespace gams {
namespace studio {
namespace search {

enum Scope {
    Selection,
    ThisFile,
    ThisProject,
    OpenTabs,
    AllFiles,
    Folder
};

class Parameters
{
public:
    QString searchTerm() const;
    void setSearchTerm(const QString &newSearchTerm);

    QString replaceTerm() const;
    void setReplaceTerm(const QString &newReplaceTerm);

    bool caseSensitive() const;
    void setCaseSensitive(bool newCaseSensitive);

    bool useRegex() const;
    void setUseRegex(bool newUseRegex);

    bool wholeWords() const;
    void setWholeWords(bool newWholeWords);

    bool searchBackwards() const;
    void setSearchBackwards(bool newSearchBackwards);

    Scope scope() const;
    void setScope(Scope newScope);

    QStringList includeFilter() const;
    void setIncludeFilter(const QStringList &newIncludeFilter);

    QStringList excludeFilter() const;
    void setExcludeFilter(const QStringList &newExcludeFilter);

    QString directory() const;
    void setDirectory(const QString &newDirectory);

    bool includeSubdirs() const;
    void setIncludeSubdirs(bool newIncludeSubdirs);

    bool showResults() const;
    void setShowResults(bool newShowResults);

    bool ignoreReadOnly() const;
    void setIgnoreReadOnly(bool newIgnoreReadOnly);

    QRegularExpression regex() const;
    void setRegex(const QRegularExpression &newRegex);

    static QChar listSeparator()
    {
        return ';';
    }

private:
    ///
    /// \brief The current UI search term.
    ///
    QString mSearchTerm;

    ///
    /// \brief The current UI replace term.
    ///
    QString mReplaceTerm;

    ///
    /// \brief The current UI case sensitive flag.
    ///
    bool mCaseSensitive = false;

    ///
    /// \brief The current UI use regex flag.
    ///
    bool mUseRegex = false;

    ///
    /// \brief The current UI whole workds flag.
    ///
    bool mWholeWords = false;

    ///
    /// \brief The UI scope value.
    /// \remark <c>Scope::ThisFile</c> is the default selection.
    ///
    Scope mScope = Scope::ThisFile;

    ///
    /// \brief The UI exclude filter.
    ///
    QStringList mExcludeFilter;

    ///
    /// \brief The UI include filter.
    ///
    QStringList mIncludeFilter;

    ///
    /// \brief The search directed choosen in the UI.
    ///
    QString mDirectory;

    ///
    /// \brief The UI include subdirs flag.
    ///
    bool mIncludeSubdirs = true;

    ///
    /// \brief searchBackwards
    ///
    bool mSearchBackwards = false;

    bool mShowResults = false;

    bool mIgnoreReadOnly = false;

    QRegularExpression mRegex;
};

class SearchCommon
{
private:
    SearchCommon();

public:
    static QString toRegularExpression(const QString &wildcard);

    static void includeFilters(const QStringList &wildcards, QList<QRegularExpression> &includeRegEx);

    static void excludeFilters(const QStringList &wildcards, QList<QRegularExpression> &excludeRegEx);

    static QString fileName(const QString &path, QChar separator);
};

}
}
}

#endif // SEARCHCOMMON_H
