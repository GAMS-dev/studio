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
#ifndef LOGCONTEXT_H
#define LOGCONTEXT_H

#include "filecontext.h"

namespace gams {
namespace studio {

class LogContext : public FileContext
{
public:
    void markOld();
    QTextDocument* document() const override;
    void addEditor(QWidget* edit) override;
    void removeEditor(QWidget* edit) override;
    void setParentEntry(FileGroupContext *parent) override;
    TextMark* firstErrorMark();
    void clearLog();
    void setDebugLog(bool debugLog = true) {mDebugLog = debugLog;}
    bool mJumpToLogEnd = true;
public slots:
    void addProcessData(QProcess::ProcessChannel channel, QString text);
    void setJumpToLogEnd(bool state);

protected:
    friend class FileRepository;
    LogContext(FileId fileId, QString name);

    struct LinkData {
        TextMark* textMark = nullptr;
        int col = 0;
        int size = 1;
    };
    QString extractError(QString text, ExtractionState &state, QList<LinkData>& marks);
    QString extractLinks(const QString &text, ExtractionState &state, QList<LinkData>& marks);

private:
    struct ErrorData {
        int lstLine = 0;
        int errNr = 0;
        QString text;
    };
    bool mInErrorDescription = false;
    QTextDocument *mDocument = nullptr;
    ErrorData mCurrentErrorHint;
//    QSet<FileContext*> mMarkedContextList;
    QString mLineBuffer;
    TextMark* mLastLstLink = nullptr;
    bool mConceal = false;
    bool mDebugLog = false;
    QString mDashLine;
};

} // namespace studio
} // namespace gams

#endif // LOGCONTEXT_H
