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
#ifndef TEXTMARKLIST_H
#define TEXTMARKLIST_H

#include <QtCore>
#include "textmark.h"

namespace gams {
namespace studio {

class FileContext;

class TextMarkList: public QObject
{
    Q_OBJECT
public:
    TextMarkList(FileGroupContext* group, const QString &fileName);
    void unbind();
    void bind(FileContext* fc);
    void updateMarks();
    void rehighlight();
    QList<TextMark*> marksForBlock(QTextBlock block, TextMark::Type refType = TextMark::all);
    QList<TextMark*> marks() { return mMarks;}
    int textMarkCount(QSet<TextMark::Type> tmTypes);
    FileContext* fileContext();
    QTextDocument* document() const;
    FileContext* openFileContext();

signals:
    void getFileContext(QString filePath, FileContext *&resultFile, FileGroupContext* fileGroup = nullptr);

public slots:
    void shareMarkHash(QHash<int, TextMark*>* marks, TextMark::Type filter);
    void textMarkIconsEmpty(bool* hasIcons);
    void documentOpened();
    void documentChanged(int pos, int charsRemoved, int charsAdded);

protected:
    friend class TextMark;
    friend class LogContext;
    friend class FileContext;
    friend class FileGroupContext;
    TextMark* generateTextMark(TextMark::Type tmType, int value, int line, int column, int size = 0);
    void removeTextMarks(QSet<TextMark::Type> tmTypes);
    void removeTextMark(TextMark* mark);
    QList<TextMark*> findMarks(const QTextCursor& cursor);
    TextMark* firstErrorMark();
    void connectDoc();

private:
    FileGroupContext* mGroupContext = nullptr;
    FileContext* mFileContext = nullptr;
    QString mFileName;
    QList<TextMark*> mMarks;
};

} // namespace studio
} // namespace gams

#endif // TEXTMARKLIST_H
