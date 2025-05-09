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
#ifndef RESULT_H
#define RESULT_H

#include <QString>
#include "common.h"
#include "theme.h"

namespace gams {
namespace studio {
namespace search {

class Result
{
    friend class SearchResultList;
public:
    Result();

    explicit Result(int lineNr, int colNr, int length, const QString &fileLoc,
                    const NodeId& parent, const QString& context = "");

    bool operator==(const Result& r1) {
        return (filePath()==r1.filePath() && lineNr()==r1.lineNr() && colNr()==r1.colNr());
    }

    int lineNr() const;
    int colNr() const;

    QString filePath() const;
    void setFilePath(const QString &fp);

    QString context() const;
    int length() const;
    NodeId parentGroup() const;
    void setParentGroup(const NodeId &parent);

    int logicalIndex() const;
    void setLogicalIndex(int index);

private:
    static const int MAX_CONTEXT_LENGTH = 60;

    int mLineNr;
    int mColNr;
    int mLength;
    QString mFilePath;
    QString mContext;
    NodeId mParent;
    int mLogicalIndex;

    QString matchHighlightStart = QString("<b style=background-color:%1;color:%2>")
            .arg(Theme::color(Theme::Edit_matchesBg).name(), QColor(Qt::white).name());
    QString matchHighlightEnd = QString("</b>");
};

}
}
}

#endif // RESULT_H
