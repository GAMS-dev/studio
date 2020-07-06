#include "neosprocess.h"
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
}

void NeosProcess::execute()
{
    TRACE()
    if (!prepareNeosParameters()) return;
    mProcess.setWorkingDirectory(workingDirectory());
#if defined(__unix__) || defined(__APPLE__)
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start(nativeAppPath(), parameters());
#else
    mProcess.setNativeArguments(parameters().join(" "));
    mProcess.setProgram(nativeAppPath());
    emit newProcessCall("Running:", appCall(nativeAppPath(), parameters()));
    mProcess.start();
#endif
}

void NeosProcess::interrupt()
{
    QProcess proc(this);
    QStringList params;
    prepareKill(params);
    mProcess.setWorkingDirectory(workingDirectory());
#if defined(__unix__) || defined(__APPLE__)
    mProcess.start(nativeAppPath(), params);
#else
    mProcess.setNativeArguments(params.join(" "));
    mProcess.setProgram(nativeAppPath());
    mProcess.start();
#endif
}

void NeosProcess::setGmsFile(QString gmsPath)
{
    mRunFile = gmsPath;
}

bool NeosProcess::prepareNeosParameters()
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

    if (!neosFile.open(QFile::WriteOnly)) {
        DEB() << "error opening neos file: " << neosPath;
        return false;
    }

    QByteArray data = rawData(CommonPaths::nativePathForProcess(mRunFile), params.join(" ")).toUtf8();

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

QString NeosProcess::rawData(QString runFile, QString parameters)
{
    QString s1 =
R"s1(* Create temp.g00
$call.checkErrorLevel gams %1 lo=%gams.lo% er=99 ide=1 a=c xs=temp.g00
* Set switches and parameters for NEOS submission
$set restartFile temp.g00
$set priority    short
$set wantgdx     yes
$set parameters  ' %2'
)s1";
    s1 = s1.arg(runFile).arg(parameters);

    QString s2 =
R"s2(
$onEmbeddedCode Python:
import os
import sys
import time
import base64
import re
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
neos = xmlrpclib.ServerProxy('https://neos-server.org:3333')
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

offset = 0
echo = 1
status = ''
while status != 'Done':
    time.sleep(1)
    (msg, offset) = neos.getIntermediateResults(jobNumber, password, offset)
    if echo == 1:
       s = msg.data.decode()
       if s.find('Composing results.') != -1:
          s = s.split('Composing results.', 1)[0]
          echo = 0;
       s = re.sub('/var/lib/condor/execute/dir_\d+/gamsexec/', ':filepath:', s)
       s = s.replace(':filepath:',r'%gams.wdir% '.rstrip())
       sys.stdout.write(s)
       sys.stdout.flush()

    status = neos.getJobStatus(jobNumber, password)

msg = neos.getOutputFile(jobNumber, password, 'solver-output.zip')
with open('solver-output.zip', 'wb') as rf:
    rf.write(msg.data)
$offEmbeddedCode
$hiddencall rm -f solve.log solve.lst solve.lxi out.gdx && gmsunzip -o solver-output.zip
)s2";
    return s1 + s2;
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
neos.getIntermediateResults(jobNumber, password)

$offEmbeddedCode
)s1";
    return s1.arg(mJobNumber).arg(mJobPassword);
}


} // namespace neos
} // namespace studio
} // namespace gams
