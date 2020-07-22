#include "neosprocess.h"
#include "neosmanager.h"
#include "logger.h"
#include "commonpaths.h"
#include <QStandardPaths>
#include <QDir>

#ifdef _WIN32
#include "Windows.h"
#endif


namespace gams {
namespace studio {
namespace neos {

NeosProcess::NeosProcess(QObject *parent) : AbstractGamsProcess("gams", parent)
{
    connect(&mProcess, &QProcess::stateChanged, this, &NeosProcess::compileStateChanged);
    connect(&mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &NeosProcess::compileCompleted);

    mManager = new NeosManager(this);
    mManager->setUrl("https://neos-server.org:3333");
    connect(mManager, &NeosManager::rePing, this, &NeosProcess::rePing);

}

NeosProcess::~NeosProcess()
{
    delete mManager;
}

void NeosProcess::execute()
{
    // 1. compile gms
    // 2. neos: submitJob
    // 3. neos: monitor the job
    // 4. get and unpack result file

    if (!prepareCompileParameters()) return;
    mProcess.setWorkingDirectory(workingDirectory());
#if defined(__unix__) || defined(__APPLE__)
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
#else
    mProcess.setNativeArguments(parameters().join(" "));
    mProcess.setProgram(nativeAppPath());
    DEB() << "STARTING: " << nativeAppPath() << " " << parameters().join(" ");
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start();
    setNeosState(NeosCompile);
#endif
}

void NeosProcess::compileStateChanged(QProcess::ProcessState state)
{
    // TODO(JM)
}

void NeosProcess::compileCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0 && exitStatus == QProcess::NormalExit && mNeosState == NeosCompile) {
        // TODO(JM) submit job to start monitoring

        setNeosState(NeosMonitor);
    }
}


void NeosProcess::interrupt()
{
    mSubProc = new QProcess(nullptr);
    QStringList params;
    prepareKill(params);
    mSubProc->setWorkingDirectory(workingDirectory());
    connect(mSubProc, &QProcess::readyReadStandardOutput, this, &NeosProcess::readSubStdOut);
    connect(mSubProc, &QProcess::readyReadStandardError, this, &NeosProcess::readSubStdErr);
#if defined(__unix__) || defined(__APPLE__)
    mSubProc->start(nativeAppPath(), params);
#else
    mSubProc->setNativeArguments(params.join(" ") + " lo=3 ide=1 er=99 errmsg=1 pagesize=0");
    mSubProc->setProgram(nativeAppPath());
    DEB() << "STARTING: " << nativeAppPath() << " " << (params.join(" ")  + " lo=3 ide=1 er=99 errmsg=1 pagesize=0");
    mSubProc->start();
    connect(mSubProc, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), this, &NeosProcess::compileCompleted);
    DEB() << "   - state: " << mSubProc->state();
#endif
}

void NeosProcess::readStdChannel(QProcess::ProcessChannel channel)
{
    if (mJobPassword.isNull() && channel == QProcess::StandardOutput) {
        // scan until jobNumber and password was passed
        mOutputMutex.lock();
        mProcess.setReadChannel(channel);
        int avail = mProcess.bytesAvailable();
        mOutputMutex.unlock();

        if (avail) {
            mOutputMutex.lock();
            scanForCredentials(mProcess.peek(avail).constData());
            mOutputMutex.unlock();
        }
    }
    AbstractGamsProcess::readStdChannel(channel);
}

void NeosProcess::completed(int exitCode)
{
    QFile::remove(mRunFile.left(mRunFile.lastIndexOf('.')) + ".neos");
    AbstractGamsProcess::completed(exitCode);
}

void NeosProcess::rePing(const QString &value)
{
    DEB() << "PING: " << value;
}

void NeosProcess::reVersion(const QString &value)
{
    DEB() << "VERSION: " << value;
}

void NeosProcess::reSubmitJob(const int &jobNumber, const QString &jobPassword)
{
    DEB() << "SUBMIT: " << jobNumber << " - pw: " << jobPassword;
    // TODO(JM) store jobnumber and password for later resuming

    // monitoring starts automatically after successfull submission

    setNeosState(NeosMonitor);
}

void NeosProcess::reGetJobStatus(const QString &value)
{
    if (value.compare("done", Qt::CaseInsensitive) == 0 && mNeosState == NeosMonitor) {

        // TODO(JM) Load result file (for large files this may take a while, check if neos supports progress monitoring)

        setNeosState(NeosGetResult);
    }
}

void NeosProcess::reGetCompletionCode(const QString &value)
{

}

void NeosProcess::reGetJobInfo(const QString &category, const QString &solverName, const QString &input, const QString &status, const QString &completionCode)
{

}

void NeosProcess::reKillJob()
{

}

void NeosProcess::reGetIntermediateResults(const QByteArray &data)
{
    // TODO(JM) replace "[LST:" by "[LS1:"
    // TODO(JM) replace neos-path by local path
}

void NeosProcess::reGetFinalResultsNonBlocking(const QByteArray &data)
{

}

void NeosProcess::reGetOutputFile(const QByteArray &data)
{

    // TODO(JM) check if neos sends partial files if the result-file is too large

}

void NeosProcess::reError(const QString &errorText)
{
    DEB() << "ERROR: " << errorText;
}

void NeosProcess::setNeosState(NeosState newState)
{
    if (newState != NeosNoState && int(newState) != int(mNeosState)+1) {
        DEB() << "Warning: NeosState jumped from " << mNeosState << " to " << newState;
    }
    emit neosStateChanged(this, mNeosState);
}

void NeosProcess::readSubStdOut()
{
    DEB() << mSubProc->readAllStandardOutput();
}

void NeosProcess::readSubStdErr()
{
    DEB() << mSubProc->readAllStandardError();
}

void NeosProcess::setGmsFile(QString gmsPath)
{
    mRunFile = gmsPath;
}

bool NeosProcess::prepareCompileParameters()
{
    if (mRunFile.isEmpty()) {
        DEB() << "Error: No runable file assigned to the NEOS process";
        return false;
    }

    int lastDot = mRunFile.lastIndexOf('.');
    QString neosPath = mRunFile.left(lastDot) + ".neos";
    QFile neosFile(neosPath);
    if (neosFile.exists()) {
        neosFile.remove();
    }
    // remove parameter with original run-filename
    QStringList params = parameters();
    params.removeAt(0);

    // TODO(JM) remove when GAMS33 is out (also in pythons gams-compile command)
    QMutableListIterator<QString> i(params);
    while (i.hasNext()) {
        i.next();
        if (i.value().startsWith("previousWork", Qt::CaseInsensitive)) i.remove();
    }


    if (!neosFile.open(QFile::WriteOnly)) {
        DEB() << "error opening neos file: " << neosPath;
        return false;
    }

    QByteArray data = rawData(CommonPaths::nativePathForProcess(mRunFile), params.join(" ")
                              , mRunFile.left(lastDot)+'/').toUtf8();

    neosFile.write(data);
    neosFile.flush();
    neosFile.close();

    // prepend parameter with replaced neos run-filename
    params.prepend(CommonPaths::nativePathForProcess(neosPath));
    setParameters(params);
    return true;
}

bool NeosProcess::prepareKill(QStringList &tempParams)
{
    if (mJobNumber.isEmpty()) {
        DEB() << "Error: No running NEOS process";
        return false;
    }
    if (mRunFile.isEmpty()) {
        DEB() << "Error: No runable file assigned to the NEOS process";
        return false;
    }
    int lastDot = mRunFile.lastIndexOf('.');
    QString neosPath = mRunFile.left(lastDot) + ".neosKill";
    QFile neosFile(neosPath);
    if (neosFile.exists()) {
        neosFile.remove();
    }

    if (!neosFile.open(QFile::WriteOnly)) {
        DEB() << "error opening neos file: " << neosPath;
        return false;
    }

    QByteArray data = rawKill().toUtf8();

    neosFile.write(data);
    neosFile.flush();
    neosFile.close();

    // prepend parameter with replaced neos run-filename
    tempParams.prepend(CommonPaths::nativePathForProcess(neosPath));
    return true;
}

const QList<QByteArray> cCredentials{"--- Job ", "number = ", "password = "};

void NeosProcess::scanForCredentials(const QByteArray &data)
{
    int p = data.indexOf(cCredentials.at(0));
    if (p < 0) return;
    p += cCredentials.at(0).length();
    int credIndex = mJobNumber.isNull() ? 1 : mJobPassword.isNull() ? 2 : 0;
    while (credIndex) {
        QByteArray ba = data.right(data.length()-p);
        if (ba.startsWith(cCredentials.at(credIndex))) {
            p += cCredentials.at(credIndex).length();
            int end = p;
            for (int i = p; i < data.length(); ++i) {
                if (data.at(i) == '\n' || data.at(i) == '\r') break;
                end = i+1;
            }
            if (credIndex == 1) {
                mJobNumber = data.mid(p, end-p);
                DEB() << "JobNr: " << mJobNumber;
                p = data.indexOf(cCredentials.at(0), end);
                if (p < 0) break;
                p += cCredentials.at(0).length();
                ++credIndex;
            } else {
                mJobPassword = data.mid(p, end-p);
                DEB() << "JobPw: " << mJobPassword;
                break;
            }
        } else {
            p = data.indexOf(cCredentials.at(0), p+1);
            if (p < 0) break;
        }
    }
}

QString NeosProcess::rawData(QString runFile, QString parameters, QString workdir)
{
    QString lstName = workdir + "solve.lst";
    QString resultDir = workdir.split('/', QString::SkipEmptyParts).last();
    QString sPrio = (mPrio == Priority::prioShort ? "short" : "long");
    QString s1 =
R"s1(* Create temp.g00
$call.checkErrorLevel gams %1 lo=%gams.lo% er=99 ide=1 a=c xs=temp.g00 previousWork=1 %2
* Set switches and parameters for NEOS submission
$set restartFile temp.g00
$set priority    %4
$set wantgdx     yes
$set parameters  ' %2'
$set workdir     '%3'
)s1";
    s1 = s1.arg(runFile).arg(parameters).arg(workdir).arg(sPrio);

    QString s2 =
R"s2(
$onEmbeddedCode Python:
import os
import sys
import time
import base64
import re
import ssl
try:
    import xmlrpc.client as xmlrpclib
except ImportError:
    import xmlrpclib

# NEOS XML Template (to be filled)
xml = r'''<document>
<category>lp</category>
<solver>BDMLP</solver>
<priority>%priority%</priority>
<inputType>GAMS</inputType>
<model><![CDATA[]]></model>
<options><![CDATA[]]></options>
<parameters><![CDATA[fw=1%parameters%]]></parameters>
<restart><base64>:restartb64:</base64></restart>
<wantlog><![CDATA[yes]]></wantlog>
<wantlst><![CDATA[yes]]></wantlst>
<wantgdx><![CDATA[%wantgdx%]]></wantgdx>
</document>'''
neos = xmlrpclib.ServerProxy('https://neos-server.org:3333', verbose=False, use_datetime=True, context=ssl._create_unverified_context())
alive = neos.ping()
if alive != "NeosServer is alive\n":
    raise NameError('\n***\n*** Could not make connection to NEOS Server\n***')
with open(r'%restartFile%', 'rb') as restartfile:
    restart = restartfile.read()
    xml = xml.replace(":restartb64:", base64.b64encode(restart).decode('utf-8'))


#with open('dryRun.xml, 'w') as rf:
#    rf.write(xml)

(jobNumber, password) = neos.submitJob(xml)
sys.stdout.write("\n--- Job number = %d\n--- Job password = %s\n" % (jobNumber, password))
sys.stdout.flush()

if jobNumber == 0:
    raise NameError('\n***\n*** NEOS Server error:' + password + '\n***')

sys.stdout.write("\n--- switch to NEOS %2/solve.lst[LS2:\"%1\"]\n")
if '%priority%' == 'long':
    sys.stdout.write('--- Priority: long (no intermediate messages)\n')
sys.stdout.flush()

offset = 0
echo = 1
status = ''
while status != 'Done':
    time.sleep(1)
    (msg, offset) = neos.getIntermediateResultsNonBlocking(jobNumber, password, offset)
    if echo == 1:
       s = msg.data.decode()
       if s.find('Composing results.') != -1:
          s = s.split('Composing results.', 1)[0]
          echo = 0;
       s = re.sub('/var/lib/condor/execute/dir_\d+/gamsexec/', ':filepath:', s)
       s = s.replace(':filepath:',r'%workdir% '.rstrip())
       s = s.replace('[LST:','[LS2:')
       sys.stdout.write(s)
       sys.stdout.flush()

    status = neos.getJobStatus(jobNumber, password)

os.makedirs('%workdir%', exist_ok=True)
msg = neos.getOutputFile(jobNumber, password, '%2/solver-output.zip')
with open('%2/solver-output.zip', 'wb') as rf:
    rf.write(msg.data)
$offEmbeddedCode
$hiddencall cd %2 && rm -f solve.log solve.lst solve.lxi out.gdx && gmsunzip -o solver-output.zip
$onEmbeddedCode Python:
if '%priority%' == 'long':
    with open('%2/solve.log', 'r') as rf:
        s = rf.read()
        s = re.sub('/var/lib/condor/execute/dir_\d+/gamsexec/', ':filepath:', s)
        s = s.replace(':filepath:',r'%workdir% '.rstrip())
        s = s.replace('[LST:','[LS2:')
        sys.stdout.write(s)
        sys.stdout.flush()

$offEmbeddedCode
)s2";
    return s1 + s2.arg(lstName).arg(resultDir);
}

QString NeosProcess::rawKill()
{
    QString s1 =
R"s1($onEmbeddedCode Python:
try:
    import xmlrpc.client as xmlrpclib
except ImportError:
    import xmlrpclib

neos = xmlrpclib.ServerProxy('https://neos-server.org:3333')
alive = neos.ping()
if alive != "NeosServer is alive\n":
    raise NameError('\n***\n*** Could not make connection to NEOS Server\n***')
jobNumber = %1
password = %2
neos.killJob(jobNumber, password)

$offEmbeddedCode
)s1";
    return s1.arg(mJobNumber).arg(mJobPassword);
}


} // namespace neos
} // namespace studio
} // namespace gams
