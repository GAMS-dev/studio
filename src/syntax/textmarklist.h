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

#include "textmark.h"

namespace gams {
namespace studio {

class ProjectFileNode;
class ProjectGroupNode;

class TextMarkList: public QObject
{
    Q_OBJECT
public:
    TextMarkList(ProjectGroupNode* group, const QString &fileName);
    void unbind();
    void bind(ProjectFileNode* fc);
    void updateMarks();
    void rehighlight();
    QVector<TextMark*> marksForBlock(QTextBlock block, TextMark::Type refType = TextMark::all);
    QVector<TextMark*> marks() { return mMarks;}
    int textMarkCount(QSet<TextMark::Type> tmTypes);
    ProjectFileNode* fileNode();
    QTextDocument* document() const;
    ProjectFileNode* openFileNode();

signals:
    void getFileNode(QString filePath, ProjectFileNode *&resultFile, ProjectGroupNode* fileGroup = nullptr);

public slots:
    void shareMarkHash(QHash<int, TextMark*>* marks, TextMark::Type filter);
    void textMarkIconsEmpty(bool* hasIcons);
    void documentOpened();
    void documentChanged(int pos, int charsRemoved, int charsAdded);

protected:
    friend class TextMark;
    friend class ProjectLogNode;
    friend class ProjectFileNode;
    friend class ProjectGroupNode;
    TextMark* generateTextMark(TextMark::Type tmType, int value, int line, int column, int size = 0);
    void removeTextMarks(QSet<TextMark::Type> tmTypes, bool doRehighlight = false);
    void removeTextMark(TextMark* mark);
    QVector<TextMark*> findMarks(const QTextCursor& cursor);
    TextMark* firstErrorMark();
    void connectDoc();

private:
    ProjectGroupNode* mGroupNode = nullptr;
    ProjectFileNode* mFileNode = nullptr;
    QString mFileName;
    QVector<TextMark*> mMarks;
};

} // namespace studio
} // namespace gams

#endif // TEXTMARKLIST_H
