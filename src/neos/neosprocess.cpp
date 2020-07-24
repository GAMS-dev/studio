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
    extractOutputPath();
    QStringList params = compileParameters();
    mProcess.setWorkingDirectory(workingDirectory());

#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(nativeAppPath(), params);
#else
    mProcess.setNativeArguments(params.join(" "));
    mProcess.setProgram(nativeAppPath());
    mProcess.start();
#endif

    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    setNeosState(Neos1Compile);
}

void NeosProcess::extractOutputPath()
{
    mOutPath = parameters().size() ? parameters().first() : QString();
    if (mOutPath.startsWith('"'))
        mOutPath = mOutPath.mid(1, mOutPath.length()-2);
    int i = mOutPath.lastIndexOf('.');
    if (i >= 0) mOutPath = mOutPath.left(i);
}

QStringList NeosProcess::compileParameters()
{
    if (mOutPath.isEmpty()) {
        DEB() << "Error: No runable file assigned to the NEOS process";
        return QStringList();
    }
    QFileInfo fi(mOutPath);
    QDir dir(fi.path());
    dir.mkdir(fi.fileName());
    QStringList params = parameters();
    QMutableListIterator<QString> i(params);
    bool needsXSave = true;
    bool needsActC = true;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("xsave=", Qt::CaseInsensitive) || par.startsWith("xs=", Qt::CaseInsensitive)) {
            needsXSave = false;
            i.setValue("xsave=" + fi.fileName());
        } else if (par.startsWith("action=", Qt::CaseInsensitive) || par.startsWith("a=", Qt::CaseInsensitive)) {
            needsActC = false;
            i.setValue("action=c");
        }
    }
    if (needsXSave) params << ("xsave=" + fi.fileName());
    if (needsActC) params << ("action=c");
    return params;
}

QStringList NeosProcess::remoteParameters()
{
    QStringList params = parameters();
    if (params.size()) params.removeFirst();
    QMutableListIterator<QString> i(params);
    bool needsFw = true;
    while (i.hasNext()) {
        QString par = i.next();
        if (par.startsWith("forcework=", Qt::CaseInsensitive) || par.startsWith("fw=", Qt::CaseInsensitive)) {
            needsFw = false;
            i.setValue("fw=1");
        } else if (par.startsWith("action=", Qt::CaseInsensitive) || par.startsWith("a=", Qt::CaseInsensitive)) {
            i.remove();
            continue;
        } else if (par.startsWith("xsave=", Qt::CaseInsensitive) || par.startsWith("xs=", Qt::CaseInsensitive)) {
            i.remove();
            continue;
        }
    }
    if (needsFw) params << ("fw=1");
    return params;
}

void NeosProcess::compileCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        DEB() << "Error on compilation, exitCode " << QString::number(exitCode);
        return;
    }
    if (exitCode) {
        DEB() << "Compilation errors ";
        return;
    }
    if (exitStatus == QProcess::NormalExit && mNeosState == Neos1Compile) {
        // TODO(JM) submit job to start monitoring
        QStringList params = remoteParameters();
        QString g00 = mOutPath + ".g00";
        DEB() << "ready to submit:\n" << g00 << "\n    " << params.join(" ");
        mManager->submitJob(g00, params.join(" "));
    }
}


void NeosProcess::interrupt()
{
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
    DEB() << "SUBMITED: " << jobNumber << " - pw: " << jobPassword;
    // TODO(JM) store jobnumber and password for later resuming

    // monitoring starts automatically after successfull submission

    setNeosState(Neos2Monitor);
}

void NeosProcess::reGetJobStatus(const QString &value)
{
    if (value.compare("done", Qt::CaseInsensitive) == 0 && mNeosState == Neos2Monitor) {

        // TODO(JM) Load result file (for large files this may take a while, check if neos supports progress monitoring)

        setNeosState(Neos3GetResult);
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
    if (newState != NeosIdle && int(newState) != int(mNeosState)+1) {
        DEB() << "Warning: NeosState jumped from " << mNeosState << " to " << newState;
    }
    mNeosState = newState;
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

QByteArray NeosProcess::convertReferences(const QByteArray &data)
{
    QByteArray res;
    res.reserve(data.size()+mOutPath.length());
    QByteArray remotePath("/var/lib/condor/execute/dir_+/gamsexec");
    QByteArray lstTag("[LST:");
    int iRP = 0;
    int iLT = 0;
    int iCount = 0;  // count of chars currently not copied

    for (int i = 0; i < data.size(); ++i) {
        if (iRP == remotePath.length()) {
            // add local path
            iRP = 0;
        }
        // Check if still in remotePath pattern
        if (data.at(i) >= '0' && data.at(i) <= '9') {
            if (remotePath.at(iRP) != '+') iRP = 0;
        } else {
            if (remotePath.at(iRP) == '+') ++iRP;
            if (iRP < remotePath.length()) {
                if (remotePath.at(iRP) == data.at(i)) ++iRP;
                else iRP = 0;
            }
        }
        // Check if still in lstTag pattern
        if (lstTag.at(iLT) == data.at(i)) ++iLT;
        else iLT = 0;
        ++iCount;
        if (iLT || iRP) ; // TODO(JM) paste checked part to res
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
