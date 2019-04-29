#include "logparser.h"
#include "file.h"
#include <QTextCodec>
#include <QDir>

namespace gams {
namespace studio {

LogParser::LogParser(TextMarkRepo *tmRepo, FileMetaRepo *fmRepo, FileId fileId, ProjectRunGroupNode* runGroup, QTextCodec *codec)
    : mMarkRepo(tmRepo), mMetaRepo(fmRepo), mFileId(fileId), mRunGroup(runGroup), mCodec(codec)
{
}

QStringList LogParser::parseLine(const QByteArray &data, ExtractionState state, bool &hasError, MarksBlockState *mbState)
{
    QTextCodec::ConverterState convState;
    QString line(mCodec->toUnicode(data.constData(), data.size(), &convState));
    if (mCodec) {
        line = mCodec->toUnicode(data.constData(), data.size(), &convState);
    }
    if (!mCodec || convState.invalidChars > 0) {
        QTextCodec* locCodec = QTextCodec::codecForLocale();
        line = locCodec->toUnicode(data.constData(), data.size(), &convState);
    }

    hasError = false;
    QString newLine = extractLinks(line, state, mbState, hasError);
    if (mbState) {
        if (mbState->spreadLst &&  state == LogParser::Inside) {
            ++mbState->spreadedLines;
        } else {
            mbState->spreadLst = false;
        }
        if (state >= LogParser::Exiting) {
            QString lstErr = mbState->errData.text;
            mRunGroup->setLstErrorText(mbState->errData.lstLine, lstErr);
        }
    }
    if (state == LogParser::FollowupError) {
        newLine = extractLinks(line, state, mbState, hasError);
    }
    QList<int> scrollVal;
    QList<QTextCursor> cursors;

    QStringList res;
    if (mDebugMode) res << line;
    res << newLine;
    return res;
}


inline QStringRef capture(const QString &line, int &a, int &b, const int offset, const QChar ch)
{
    a = b + offset;
    b = line.indexOf(ch, a);
    if (b < 0) b = line.length();
    return line.midRef(a, b-a);
}


QString LogParser::extractLinks(const QString &line, ExtractionState &state, MarksBlockState *mbState, bool &hasError)
{
    if (mbState && mbState->inErrorText) {
        if (line.startsWith("***") || line.startsWith("---")) {
            state = FollowupError;
            mbState->inErrorText = false;
            return QString();
        } else if (line.startsWith(" ")) {
            if (mbState->errData.text.isEmpty()) {
                if (mbState->errData.errNr)
                    mbState->errData.text += QString("%1\t").arg(mbState->errData.errNr)+line.trimmed();
                else
                    mbState->errData.text += '\t'+line.trimmed();
            } else
                mbState->errData.text += "\n\t"+line.trimmed();
            state = Inside;
        } else {
            state = Exiting;
            mbState->inErrorText = false;
        }
        return line;
    }

    QString result;
    if (line.isEmpty()) return QString("");
//    TextMark* errMark = nullptr;
    bool errFound = false;
    bool isRuntimeError = false;
    int lstColStart = 4;
    int posA = 0;
    int posB = 0;
    if (line.startsWith("*** Error ")) {
        bool ok = false;
        posA = 9;
        while (posA < line.length() && (line.at(posA)<'0' || line.at(posA)>'9')) posA++;
        posB = posA;
        while (posB < line.length() && line.at(posB)>='0' && line.at(posB)<='9') posB++;
        int errNr = line.midRef(posA, posB-posA).toInt(&ok);
        bool isValidError = line.midRef(posB, 4) == " in ";
        if (mbState) {
            mbState->errData.lstLine = -1;
            mbState->errData.text = "";
        }

        QString fName;
        int lineNr;
        int size;
        int colStart = 0;
        posB = 0;
        if (line.midRef(9, 9) == " at line ") {
            isValidError = false;
            isRuntimeError = true;
            result = capture(line, posA, posB, 0, ':').toString();
            lineNr = errNr-1;
            size = 0;
            colStart = -1;
            if (mbState) {
                mbState->errData.errNr = 0;
                // TODO(JM) review for the case the file is in a sub-directory
                fName = mRunGroup->location() + '/' + mbState->lastSourceFile;
                if (posB+2 < line.length()) {
                    int subLen = (line.contains('[') ? line.indexOf('['): line.length()) - (posB+2);
                    mbState->errData.text = line.mid(posB+2, subLen);
                }
            }
        } else {
            lstColStart = -1;
            result = capture(line, posA, posB, 0, '[').toString();
            fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"').toString());
            lineNr = capture(line, posA, posB, 2, ',').toInt()-1;
            size = capture(line, posA, posB, 1, ']').toInt()-1;
            posB++;
            if (mbState) mbState->errData.errNr = ok ? errNr : 0;
        }
        if (!fName.isEmpty()) {
            QFileInfo fi(fName);
            if (!fi.exists() || fi.isDir()) fName = "";
        }

        if (mbState && isValidError && !fName.isEmpty()) {
            hasError = true;
            errFound = true;
            MarkData mark;
            mark.setLogPos(line.indexOf(" ")+1, result.length());
            mark.setMark(fName, TextMark::error, mbState->errData.lstLine, lineNr, colStart, size);
            mbState->marks << mark;
            mbState->inErrorText = true;
        }
    }
    if (mbState && line.startsWith("--- ")) {
//        isGamsLine = true;
        int fEnd = line.indexOf('(');
        if (fEnd >= 0) {
            int nrEnd = line.indexOf(')', fEnd);
            bool ok;
            line.mid(fEnd+1, nrEnd-fEnd-1).toInt(&ok);
            if (ok) mbState->lastSourceFile = line.mid(4, fEnd-4);
        }
    }

    // Now we should have a system output
    while (posA < line.length()) {
        result += capture(line, posA, posB, 0, '[');

        if (posB+5 < line.length()) {
            TextMark::Type tmType = errFound ? TextMark::link : TextMark::target;

            // LST:
            if (line.midRef(posB+1,4) == "LST:") {
                if (isRuntimeError) {
                    tmType = TextMark::error;
                    hasError = true;
                }
                int lineNr = capture(line, posA, posB, 5, ']').toInt()-1;
                posB++;
                if (mbState) {
                    mbState->errData.lstLine = lineNr;
                    MarkData mark;
                    mark.setLogPos(lstColStart, (lstColStart<0) ? -lstColStart : result.length() - 1);
                    mark.setMark("lst", tmType, mbState->errData.lstLine, lineNr, 0, 0);
                    if (errFound) {
                        mbState->marks[0].value = mbState->errData.lstLine;
                        errFound = false;
                    }
                    mbState->marks << mark;
                }

                // FIL + REF
            } else if (line.midRef(posB+1,4) == "FIL:" || line.midRef(posB+1,4) == "REF:") {
                QString fName = QDir::fromNativeSeparators(capture(line, posA, posB, 6, '"').toString());
                int lineNr = capture(line, posA, posB, 2, ',').toInt()-1;
                int col = capture(line, posA, posB, 1, ']').toInt()-1;
                posB++;

                if (mbState) {
                    MarkData mark;
                    mark.setLogPos(4, result.length() - 1);
                    mark.setMark(fName, tmType, mbState->errData.lstLine, lineNr, 0, col);

                    if (mRunGroup->findFile(fName))
                        errFound = false;
                    else
                        state = Outside;
                    mbState->marks << mark;
                }

                // TIT
            } else if (line.midRef(posB+1,4) == "TIT:") {
                return QString();
            } else {
                // no link reference: restore missing braces
                result += '['+capture(line, posA, posB, 1, ']')+']';
                posB++;
            }
        } else {
            if (posB < line.length()) result += line.right(line.length() - posB);
            break;
        }
    }
    return result;
}

} // namespace studio
} // namespace gams
