/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017 GAMS Development Corp. <support@gams.com>
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
#ifndef TEXTMARK_H
#define TEXTMARK_H

#include <QtWidgets>

namespace gams {
namespace studio {

class FileContext;

class TextMark
{
public:
    enum Type {
        all,
        none,
        error,
        link,
        bookmark,
    };

    explicit TextMark(TextMark::Type tmType);
    void mark(FileContext* fileContext, int line, int column, int size = 0);
    void updateCursor();
    void setRefMark(TextMark* refMark);
    void jumpToRefMark();
    void jumpToMark();

    void showToolTip();

    int value() const;
    void setValue(int value);

    QIcon icon();
    Type type() const;
    bool isValid();
    QTextBlock textBlock();
    QTextCursor textCursor() const;

    int line() const;
    int column() const;
    int size() const;
    bool inColumn(int col) const;

private:
    FileContext* mFileContext = nullptr;
    Type mType = none;
    int mLine = -1;
    int mColumn = 0;
    int mSize = 0;
    int mValue = 0;
    QTextCursor mCursor;
    TextMark* mReference = nullptr;
};

} // namespace studio
} // namespace gams

#endif // TEXTMARK_H
