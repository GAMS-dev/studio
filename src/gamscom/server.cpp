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
#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QCollator>
#include <QFile>
#include <QDir>
#include <QTimer>

#include "profiler.h"
#include "exception.h"
#include "logger.h"

namespace gams {
namespace studio {
namespace gamscom {

const quint16 CFirstDebugPort = 3563;
const quint16 CLastDebugPort = 4563;
QSet<int> Server::mPortsInUse;

Server::Server(const QString &path, QObject *parent) : QObject(parent), mPath(path)
{
    init();
    mServer = new QTcpServer(this);
//    QTcpSocket::ReuseAddressHint
    connect(mServer, &QTcpServer::newConnection, this, &Server::newConnection, Qt::UniqueConnection);
}

Server::~Server()
{
    stopAndDelete();
    mServer = nullptr;
}

void Server::setProfiler(Profiler *profiler)
{
    mProfiler = profiler;
}

void Server::init()
{
    // calls
    mCalls.insert(invalidReply, "invalidReply");
    mCalls.insert(addBP, "addBP");
    mCalls.insert(delBP, "delBP");

    mCalls.insert(run, "run");
    mCalls.insert(stepLine, "stepLine");
    mCalls.insert(pause, "pause");
    mCalls.insert(writeGDX, "writeGDX");

    // replies
    mReplies.insert("invalidCall", invalidCall);
    mReplies.insert("features", features);
    mReplies.insert("linesMap", linesMap);
    mReplies.insert("linesMapDone", linesMapDone);

    mReplies.insert("paused", paused);
    mReplies.insert("gdxReady", gdxReady);

    mReplies.insert("profileData", profileData);
}

bool Server::start(ComFeatures features)
{
    if (mServer->isListening()) {
        logMessage("WARNING: The GAMScom-Server already listens to port" + QString::number(mServer->serverPort()));
        return true;
    }

    if (mPortsInUse.size() > 10) return false;

    quint16 port = CFirstDebugPort;
    while (mPortsInUse.contains(port) || !mServer->listen(QHostAddress::LocalHost, port)) {
        ++port;
        if (port > CLastDebugPort) {
            logMessage("GAMScom-Server could not find a free port!");
            return false;
        }
    }
    mComFeatures = features;
    mPortsInUse << mServer->serverPort();

    DEB() << "GAMScom-Server: Studio connected to GAMS on port " << port;
    return true;
}

ComFeatures Server::comFeatures()
{
    return mComFeatures;
}

void Server::stopAndDelete()
{
    setState(Finished);
    deleteSocket();
    if (mPortsInUse.contains(mServer->serverPort()))
        mPortsInUse.remove(mServer->serverPort());
    if (isListening())
        mServer->close();
    mServer->deleteLater();
}

void Server::newConnection()
{
    if (!mServer) return;
    QTcpSocket *socket = mServer->nextPendingConnection();
    if (mSocket) {
        logMessage("GAMScom-Server: already connected to a socket");
        if (socket) socket->deleteLater();
        return;
    }
    if (!socket) return;
    mSocket = socket;
    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        DEB() << "GAMScom-Server: Socket disconnected";
        deleteSocket();
    });
    connect(socket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = mSocket->readAll();
        QList<QByteArray> packets = data.split('\0');
        for (const QByteArray &packet : std::as_const(packets)) {
            if (packet.isEmpty()) continue;
            if (mIncompletePacket.isEmpty())
                mIncompletePacket = packet;
            else
                mIncompletePacket += packet;
            if (mIncompletePacket.at(mIncompletePacket.size() - 1) != '\n')
                return;

            ParseResult ret = handleReply(mIncompletePacket);
            if (ret != prOk) {
                if (!mIncompletePacket.contains("invalidCall"))
                    callProcedure(invalidReply, QStringList() << mIncompletePacket);
            }
            if (ret != prIncomplete)
                mIncompletePacket.clear();
        }
    });
    DEB() << "GAMScom-Server: Socket connected to GAMS";
    setState(Prepare);
    emit connected();
}

QString Server::gdxTempFile() const
{
    return mPath + "/temp" + QString::number(mServer->serverPort()) + ".gdx";
}

void Server::setVerbose(bool verbose)
{
    mVerbose = verbose;
}

void Server::callProcedure(CallReply call, const QStringList &arguments)
{
    if (!mSocket || !mSocket->isOpen()) {
        QString additionals = arguments.count() > 1 ? QString(" (and %1 more)").arg(arguments.count()-1) : QString();
        logMessage("GAMScom-Server: Socket not open, can't process '" + mCalls.value(call, "undefinedCall")
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    if (mVerbose)
        logMessage("Sending to GAMS: " + mCalls.value(call) + "\n" + arguments.join("\n"));
    QString keyword = mCalls.value(call);
    if (keyword.isEmpty()) {
        QString additionals = arguments.count() > 1 ? QString(" (and %1 more)").arg(arguments.count()-1) : QString();
        logMessage("GAMScom-Server: Undefined call '" + QString::number(int(call))
                   + (arguments.isEmpty() ? "'" : (":" + arguments.at(0) + "'" + additionals)));
        return;
    }
    QStringList remain(arguments);
    QString ready = keyword;
    bool progress = true;
    while (remain.length() || progress) {
        if (remain.length() && ready.length() + 2 + remain.first().length() < 255) {
            ready += '\n' + remain.takeFirst();
            progress = true;
        }
        if (!progress) {
            logMessage("GAMScom-Server: Call line too long. Can't send more than 255 characters at once '"
                       + QString::number(int(call)) + ":" + remain.first() + "'");
            break;
        }
        if (remain.isEmpty() || ready.length() + 2 + remain.first().length() >= 255) {
            mSocket->write(ready.toUtf8() + '\t');
            ready = keyword;
            progress = false;
        }
    }
}

Server::ParseResult Server::handleReply(const QString &replyData)
{
    if (mVerbose)
        logMessage("\nFrom GAMS: " + QString(replyData).replace("\n", "↲"));
    QStringList reList = replyData.split('\n', Qt::SkipEmptyParts);
    CallReply reply = invalid;
    if (!reList.isEmpty()) {
        reply = mReplies.value(reList.at(0), invalid);
        if (reply == invalid && mLastReply == linesMap) {
            reply = mLastReply;
        } else if (reply != invalid) {
            reList.removeFirst();
            mLastReply = reply;
        }
    }
    QStringList data;
    if (reList.size()) {
        data = reList.first().split('|');
    }

    bool ok = false;
    switch (reply) {
    case invalidCall:
        logMessage("GAMScom-Server: GAMS refused to process this request: " + reList.join(", "));
        break;
    case features: {
        if (reList.size() < 1) return prIncomplete; // wait for more data
        if (reList.size() > 1)
            logMessage("GAMScom-Server: [features] Only one entry expected. Additional data ignored.");

        int flags = data.at(0).toInt(&ok);

        if (!ok) {
            logMessage("GAMScom-Server: [features] Can't parse continuous line number: " + data.at(0));
            return prError;
        }
        emit signalProfiler(flags & 2);
    }   break;
    case linesMap: {
        if (reList.size() < 1) return prIncomplete; // wait for more data
        for (const QString &line : std::as_const(reList)) {
            parseLinesMap(line);
        }
    }   break;
    case linesMapDone: {
        if (!mRemainData.isEmpty())
            parseLinesMap(" ");
        if (mComFeatures.testFlag(cfProfiler)) {
            if (mIncludes.isEmpty()) {
                if (!mBreakLinesFile.isEmpty())
                    mIncludes << new IncludeLine(mBreakLinesFile);
            } else {
                if (mIncludes.first()->contLine == 1 && mIncludes.first()->outerContLine == std::numeric_limits<int>::max())
                    mIncludes.first()->outerContLine = 1;
                else
                    mIncludes.prepend(new IncludeLine(mIncludes.first()->parentFile));
            }
            mProfiler->addIncludes(mIncludes);
        }
        calcSourceMetrics();

        emit signalMapDone();
    }   break;
    case paused: {
        if (reList.size() < 1) return prIncomplete; // wait for more data
        if (reList.size() > 1)
            logMessage("GAMScom-Server: [paused] Only one entry expected. Additional data ignored.");
        int line = data.at(0).toInt(&ok);
        if (!ok) {
            logMessage("GAMScom-Server: [paused] Can't parse continuous line number: " + data.at(0));
            return prError;
        }

        setState(Paused);
        emit signalPaused(line);
    }   break;
    case gdxReady: {
        QString file;
        if (reList.size())
            file = QDir::fromNativeSeparators(data.first());

        if (file.isEmpty()) {
            logMessage("GAMScom-Server: [gdxReady] Missing name for GDX file.");
            return prIncomplete;
        }
        if (!QFile::exists(file)) {
            logMessage("GAMScom-Server: [gdxReady] File not found: " + file);
            return prError;
        }
        emit signalGdxReady(file);
    }   break;
    case profileData: {
        if (reList.size() < 1) return prIncomplete; // wait for more data
        if (reList.size() > 1)
            logMessage("GAMScom-Server: [profileData] Only one entry expected. Additional data ignored.");
        return parseProfileData(reList.at(0));
    }   break;

    default:
        logMessage("GAMScom-Server: Unknown GAMS request: " + reList.join(", "));
        return prError;
    }


    return prOk;
}

QString Server::toBpString(const QList<int> &lines)
{
    if (lines.isEmpty()) return QString();
    QString res;
    for (int line : lines) {
        if (!res.isEmpty())
            res += '|';
        res += QString::number(line);
    }
    return res;
}

void Server::parseLinesMap(const QString &breakData)
{
    if (breakData.isEmpty()) return;
    QStringList data;
    QChar c = breakData.at(0);
    if (mRemainData.isEmpty()) {
        data = breakData.split('|');
    } else if ((c < '0' || c > '9') && c != '=' && c != '|') {
        // This is probably a filename, then mRemainData is complete and can be processed
        QList<int> lines;
        QList<int> coLNs;
        QList<int> incLines;
        QList<int> firstLines;
        QList<int> retLines;
        if (getPair(mRemainData, lines, coLNs, incLines, firstLines, retLines)) {
            if (!mBreakLinesFile.isEmpty()) {
                if (!incLines.isEmpty())
                    addInclude(mBreakLinesFile, incLines);
                else if (!firstLines.isEmpty())
                    addIncludeFrom(mBreakLinesFile, firstLines.last());
                else if (!retLines.isEmpty())
                    addIncludeEnd(mBreakLinesFile, retLines);
            }
            if (!lines.isEmpty())
                emit signalLinesMap(mBreakLinesFile, lines, coLNs);
        } else {
            logMessage("GAMScom-Server: breakLines parse error: " + mRemainData);
        }
        if (mVerbose) {
            QString list("DATA-> " + mBreakLinesFile + " : ");
            for (int i = 0; i < lines.size(); ++i) {
                list += "|" + QString::number(lines.at(i)) + ":" + QString::number(coLNs.at(i));
            }
            logMessage(list);
        }
        mRemainData = QString();
        if (breakData != " ")
            data = breakData.split('|');
    } else {
        data = ("|" + mRemainData + breakData).split('|');
    }
    if (data.size() > 0)
        mRemainData = data.takeLast();
    else mRemainData = QString();
    if (data.isEmpty()) {
        return;
    }

    if (!data.first().isEmpty())
        mBreakLinesFile = QDir::fromNativeSeparators(data.first());
    QList<int> lines;
    QList<int> coLNs;
    for (int i = 1; i < data.size(); ++i) {
        QList<int> incLines;
        QList<int> firstLines;
        QList<int> retLines;
        bool ok = getPair(data.at(i), lines, coLNs, incLines, firstLines, retLines);
        if (!mBreakLinesFile.isEmpty()) {
            if (!incLines.isEmpty())
                addInclude(mBreakLinesFile, incLines);
            else if (!firstLines.isEmpty())
                addIncludeFrom(mBreakLinesFile, firstLines.last());
            else if (!retLines.isEmpty())
                addIncludeEnd(mBreakLinesFile, retLines);
        }
        incLines.clear();
        if (!ok) logMessage("GAMScom-Server: breakLines parse error: " + data.at(i));
    }
    if (!lines.isEmpty()) {
        if (mVerbose) {
            QString list("DATA=> " + mBreakLinesFile + " : ");
            for (int i = 0; i < lines.size(); ++i) {
                list += "|" + QString::number(lines.at(i)) + ":" + QString::number(coLNs.at(i));
            }
            logMessage(list);
        }
        emit signalLinesMap(mBreakLinesFile, lines, coLNs);
    }
}

Server::ParseResult Server::parseProfileData(const QString &rawData)
{
    QStringList data = rawData.split('|');
    if (data.size() < 4) return prIncomplete;
    bool ok;
    int contLine = data.at(0).toInt(&ok);
    if (!ok) return prError;
    qreal sec = data.at(2).toDouble(&ok);
    if (!ok) return prError;
    qreal mbMem = data.at(3).toDouble(&ok);
    if (!ok) return prError;
    size_t mem = qRound64(mbMem / .000001);
    size_t rows = 0;
    if (data.size() > 4)
        rows = data.at(4).toULongLong(&ok);
    if (mProfiler) {
        mProfiler->add(contLine, data.at(1), sec, mem, rows);
    }
    return prOk;
}

bool Server::getPair(const QString &assignment, QList<int> &lines, QList<int> &coLNs, QList<int> &incLines, QList<int> &firstLines, QList<int> &retLines)
{
    bool isInc = assignment.startsWith('!');
    bool isRet = assignment.startsWith('?');
    QStringList linePair = (isInc || isRet ? assignment.last(assignment.length()-1) : assignment).split("=");
    bool ok = (linePair.size() == 2 && !linePair.at(1).isEmpty());
    if (ok) {
        int line = linePair.at(0).toInt(&ok);
        if (ok) {
            int coLN = linePair.at(1).toInt(&ok);
            if (ok) {
                if (isInc) {
                    incLines << line << coLN;
                } else if (isRet) {
                    retLines << line << coLN;
                } else if (!line) {
                    firstLines << line << coLN;
                } else {
                    lines << line;
                    coLNs << coLN;
                }
            }
        }
    }
    return ok;
}

void Server::addInclude(const QString &filename, QList<int> &incLine)
{
    if (incLine.length() != 2) FATAL() << "Error in Server::setLastInclude: wrong count of numbers";
    if (filename.isEmpty()) EXCEPT() << "Filename for include-line must not be empty";

    mIncludes << new IncludeLine(filename, incLine.at(0), incLine.at(1));
}

void Server::addIncludeFrom(const QString &filename, int contLine)
{
    if (!mIncludes.size() || !mIncludes.last()->childFile.isEmpty()) {
        DEB() << "GamsCom warning: Unexpected include start for " << filename << ":0 [" << contLine << "]. Data ignored.";
        return;
    }
    if (mIncludes.last()->contLine != contLine) {
        DEB() << "GamsCom warning: Include start at " << contLine << " doesn't match expected "
              << mIncludes.last()->contLine << ". Data ignored.";
        return;
    }
    mIncludes.last()->childFile = filename;
}

void Server::addIncludeEnd(const QString &filename, QList<int> &incLine)
{
    for (int i = mIncludes.size() - 1; i >= 0; --i) {
        if (mIncludes[i]->line == incLine.first() && mIncludes[i]->parentFile.compare(filename) == 0) {
            if (mIncludes[i]->outerContLine != std::numeric_limits<int>::max()) {
                DEB() << "GamsCom warning: Include for " << mIncludes[i]->parentFile << ":" << mIncludes[i]->line
                      << " already has a continuous line of " << mIncludes[i]->outerContLine
                      << ". Replaced by "<< incLine.last() << ".";
            }
            mIncludes[i]->outerContLine = incLine.last();
            break;
        }
        if (mIncludes[i]->line < incLine.first() && mIncludes[i]->parentFile.compare(filename) == 0) {
            DEB() << "GamsCom warning: Missing Include for " << mIncludes[i]->parentFile << ":" << mIncludes[i]->line;
            break;
        }
    }
}

void Server::setState(DebugState state)
{
    if (mState != state) {
        mState = state;
        emit stateChanged(state);
    }
}

QRegularExpression CRexLF("(\\r\\n)|(\\n\\r)|\\r|\\n");

QStringList readFile(const QString &filename, int lines)
{
    // TODO(JM) temporary here -> move to SyntaxHighlighter or ContinuousLineData
    QStringList res;
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QString content = QString::fromUtf8(data); // TODO(JM) convert encoding as specified in Studio for this file by user

        int pos = 0;
        QRegularExpressionMatch match;
        while (pos < content.length() && (lines < 0 ||  res.size() < lines)) {
            int i = content.indexOf(CRexLF, pos, &match);
            if (i < 0) i = content.length();
            res << content.mid(pos, i - pos);
            pos = i + match.capturedLength();
        }
    }
    return res;
}

void Server::calcSourceMetrics()
{
    QMap<QString, QMap<int,QString>> incLines;
    QMap<QString, int> lineCount;

    // separate include positions and line counts
    lineCount.insert(mMainFilename, -1);
    for (const IncludeLine *incDat : std::as_const(mIncludes)) {
        // if (mVerbose)
        //     DEB() << "       from -> " << incDat->toString();
        if (!incLines.contains(incDat->parentFile))
            incLines.insert(incDat->parentFile, QMap<int, QString>());
        incLines[incDat->parentFile].insert(incDat->line, incDat->childFile);
        if (!lineCount.contains(incDat->childFile))
            lineCount.insert(incDat->childFile, incDat->outerContLine - incDat->contLine);
    }

    // adjust line count from cumulative to raw count
    QMap<QString, int> lineCountRaw;
    for (auto itSize = lineCount.constBegin(); itSize != lineCount.constEnd(); ++itSize) {
        lineCountRaw.insert(itSize.key(), itSize.value());
        for (auto itInc = incLines.constBegin(); itInc != incLines.constEnd(); ++itInc) {
            if (itSize.key() == itInc.key()) {
                for (auto itLine = itInc.value().constBegin(); itLine != itInc.value().constEnd(); ++itLine) {
                    lineCountRaw[itInc.key()] -= lineCount.value(itLine.value());
                }
            }
        }
    }

    // TODO(JM) extend ContinuousLineData to generate source (all includes combined to one file)
    // TODO(JM) pass mIncludes and lineCount to ContinuousLineData


    if (mVerbose) {
        DEB() << "\nINCLUDE:";
        for (auto it = incLines.constBegin(); it != incLines.constEnd(); ++it) {
            for (auto itLine = it.value().constBegin(); itLine != it.value().constEnd(); ++itLine) {
                DEB() << "  - " << it.key() << ":" << itLine.key() << "  " << itLine.value();
            }
        }
        DEB() << "\nSIZES:";
        for (auto it = lineCountRaw.constBegin(); it != lineCountRaw.constEnd(); ++it) {
            DEB() << "  - " << it.key() << ": " << it.value();
        }

        // Read files
        QMap<QString, QStringList> contents;
        QMap<QString, int> iLine;
        for (auto itSize = lineCount.constBegin(); itSize != lineCount.constEnd(); ++itSize) {
            contents.insert(itSize.key(), readFile(itSize.key(), itSize.value()));
            iLine.insert(itSize.key(), 0);
        }

        // Create continuous line content
        // QString main;
        // for (int i = mIncludes.size()-1; i >= 0; --i) {
        //     const IncludeLine *incDat = mIncludes.at(i);
        //     for (int j = contents.value(incDat->childFile).size() - 1; j >= 0; --j)
        //         contents[incDat->parentFile].insert(incDat->line, contents.value(incDat->childFile).at(j));
        //     main = incDat->parentFile;
        // }

        // DEB() << "\n### CONTINUOUS LINES:\n";
        // for (int i = 0; i < contents.value(main).size(); ++i) {
        //     DEB() << (i<9?" ":"") << (i+1) << "   " << contents.value(main).at(i);
        // }
    }

}

DebugState Server::state() const
{
    return mState;
}

void Server::addBreakpoint(int contLine)
{
    callProcedure(addBP, {QString::number(contLine)});
}

void Server::addBreakpoints(const QList<int> &contLines)
{
    QString bps = toBpString(contLines);
    if (!bps.isEmpty())
        callProcedure(addBP, {bps});
}

void Server::delBreakpoint(int contLine)
{
    callProcedure(delBP, {QString::number(contLine)});
}

void Server::clearBreakpoints()
{
    callProcedure(delBP);
}

void Server::sendRun()
{
    callProcedure(run);
    setState(Running);
}

void Server::sendStepLine()
{
    callProcedure(stepLine);
    setState(Running);
}

void Server::sendPause()
{
    callProcedure(pause);
}

void Server::sendWriteGdx(const QString &gdxFile)
{
    callProcedure(writeGDX, {gdxFile});
}

bool Server::isListening()
{
    return mServer && mServer->isListening();
}

quint16 Server::port()
{
    return mServer ? mServer->serverPort() : -1;
}

void Server::setMain(const QString &mainFilename)
{
    mMainFilename = mainFilename;
}

void Server::logMessage(const QString &message)
{
    emit addProcessLog(message.toUtf8() + '\n');
}

void Server::deleteSocket()
{
    if (mSocket) {
        QTcpSocket *socket = mSocket;
        mSocket = nullptr;
        socket->close();
        socket->deleteLater();
    }
}


} // namespace debugger
} // namespace studio
} // namespace gams
