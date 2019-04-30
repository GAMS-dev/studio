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
#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <QString>
#include "syntax/textmarkrepo.h"
#include "file/projectgroupnode.h"

namespace gams {
namespace studio {

class LogParser
{
public:
    enum ExtractionState {
        Outside,
        Entering,
        Inside,
        Exiting,
        FollowupError,
    };
    Q_ENUM(ExtractionState)

    struct ErrorData {
        int lstLine = 0;
        int errNr = 0;
        QString text;
    };

    struct MarkData {
        void setMark(QString f, TextMark::Type t, int v, int l, int c, int s) {
            file = f; type = t; value = v; line = l; col = c; size = s; fileId = -1;
        }
        void setMark(FileId f, TextMark::Type t, int v, int l, int c, int s) {
            fileId = f; type = t; value = v; line = l; col = c; size = s; file = QString();
        }
        void setLogPos(int col1, int col2) {
            logCol = col1; logSize = col2 - col1;
        }
        QString file;
        FileId fileId = -1;
        TextMark::Type type = TextMark::none;
        int value = 0;
        int line = 0;
        int col = 0;
        int size = 1;
        int logCol = 0;
        int logSize = 1;
    };
    struct MarksBlockState {
        QVector<MarkData> marks;
        ErrorData errData;
        bool inErrorText = false;
        QString lastSourceFile;
        bool spreadLst = false;
        int spreadedLines = 1;
    };
    struct LinkData {
        TextMark* textMark = nullptr;
        int col = 0;
        int size = 1;
    };

public:
    // Functionality that needs to be implemented in calling structure:
    //  - editor move to end
    //  - conceal lines that only break with '\r'
    //  - create textmarks for all MarkData and link them together

    LogParser(TextMarkRepo* tmRepo, FileMetaRepo* fmRepo, FileId fileId, ProjectRunGroupNode* runGroup, QTextCodec *codec);
    QStringList parseLine(const QByteArray &data, ExtractionState state, bool &hasError, MarksBlockState *mbState = nullptr);
    void setDebugMode(bool debug) { mDebugMode = debug; }

private:
    QString extractLinks(const QString &text, ExtractionState &state, MarksBlockState *mbState, bool &hasError);

    TextMarkRepo* mMarkRepo;
    FileMetaRepo* mMetaRepo;
    FileId mFileId;
    ProjectRunGroupNode *mRunGroup;
    QTextCodec *mCodec;
    ProjectFileNode *mLstNode = nullptr; // TODO(JM) create this at start!
    bool mDebugMode = false;
};

} // namespace studio
} // namespace gams

#endif // LOGPARSER_H
