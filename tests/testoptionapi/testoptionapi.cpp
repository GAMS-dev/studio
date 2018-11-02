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
#include <QStandardPaths>
#include <QtMath>

#include "commonpaths.h"
#include "testoptionapi.h"

using gams::studio::CommonPaths;

void TestOptionAPI::initTestCase()
{
    // given
    const QString expected = QFileInfo(QStandardPaths::findExecutable("gams")).absolutePath();
    CommonPaths::setSystemDir(expected.toLatin1());
    // when
    char msg[GMS_SSSIZE];
    optCreateD(&mOPTHandle, CommonPaths::systemDir().toLatin1(), msg, sizeof(msg));
    if (msg[0] != '\0')
        Dcreated = false;
    else
        Dcreated = true;

    // test cplex for now
    QString optdef = "optcplex.def";
    if (optReadDefinition(mOPTHandle, QDir(CommonPaths::systemDir()).filePath(optdef).toLatin1())) {
        optdefRead = false;
        QFAIL( QString("Fail to read option file '%1' from '%2'").arg(optdef).arg(CommonPaths::systemDir()).toLatin1() );
    } else {
        optdefRead = true;
    }
}

void TestOptionAPI::testReadFromStr_data()
{
    QVERIFY( Dcreated && optdefRead );

    QTest::addColumn<QString>("optionStr");
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("defined");
    QTest::addColumn<QString>("optionValue");
    QTest::addColumn<bool>("errorRead");
    QTest::addColumn<int>("errorCode");

    // comment
    QTest::newRow("*---------------------------------------------------------------------------*")
            << "*---------------------------------------------------------------------------*"
            << "" << false << "" << true << getErrorCode(optMsgUserError);
    QTest::newRow("* ask CPLEX to construct an OPT file tuned for the problem")
            << "* ask CPLEX to construct an OPT file tuned for the problem"
            << "" << false << "" << true << getErrorCode(optMsgUserError);
    QTest::newRow("*---------------------------------------------------------------------------*")
            << "*---------------------------------------------------------------------------*"
            << "" << false << "" << true << getErrorCode(optMsgUserError);
    QTest::newRow("*iterationlim 400000")  << "*iterationlim 400000"  << ""                     << false << ""        << true    << getErrorCode(optMsgUserError);
    QTest::newRow("*  lpmethod 1" )        << "*  lpmethod 1"         << ""                     << false << ""        << true    << getErrorCode(optMsgUserError);

    // empty string
    QTest::newRow("                    ")  << "                    "  << "                    " << false << ""        << false   << getErrorCode(optMsgValueWarning);
    QTest::newRow(" ")                     << " "                     << " "                    << false << ""        << false   << getErrorCode(optMsgValueWarning);
    QTest::newRow("" )                     << ""                      << ""                     << false << ""        << true    << getErrorCode(optMsgValueWarning);

    // boolean option
    QTest::newRow("  iis yes" )            << "  iis yes"             << "iis"                  << true  << "1"       << false   << -1;
    QTest::newRow("iis yes")               << "iis yes"               << "iis"                  << true  << "1"       << false   << -1;

    // deprecated option
    QTest::newRow("SCALE -1")              << "SCALE -1"              << "SCALE"                << false << ""        << true    << getErrorCode(optMsgDeprecated);
    QTest::newRow("iterationlim 400000")   << "iterationlim 400000"   << "iterationlim"         << false << ""        << true    << getErrorCode(optMsgDeprecated);

    QTest::newRow("cuts 2")                << "cuts 2"                << "cuts"                 << true  << "2"       << false   << -1;
    QTest::newRow("RERUN=YES")             << "RERUN=YES"             << "RERUN"                << true  << "YES"     << false   << -1;

    // strlist option
    QTest::newRow("tuning str1 str2 str3") << "tuning str1 str2 str3" << "tuning"               << true  << "str1 str2 str3"  << false   << -1;
    QTest::newRow("tuning str4 str5")      << "tuning str4 str5"      << "tuning"               << true  << "str4 str5"       << false   << -1;

    // enumint option
    QTest::newRow("lpmethod 9")            << "lpmethod 9"            << "lpmethod"            << false  << "0"      << true    << getErrorCode(optMsgValueError);
    QTest::newRow("  lpmethod 4")          << "  lpmethod 4"          << "lpmethod"            << true   << "4"      << false   << -1;
    QTest::newRow("lpmethod 19")           << "lpmethod 19"           << "lpmethod"            << false  << "0"      << true    << getErrorCode(optMsgValueError);

    QTest::newRow("localimplied -1")       << "localimplied -1"       << "localimplied"         << true  << "-1"     << false   << -1;
    QTest::newRow("localimplied 55")       << "localimplied 55"       << "localimplied"         << false << "0"      << true    << getErrorCode(optMsgValueError);

    // integer option
    QTest::newRow("solnpoolcapacity=1100000") << "solnpoolcapacity=1100000"  << "solnpoolcapacity"   << true  << "1100000"  << false   << -1;
    QTest::newRow("aggind -3")                << "aggind -3"                 << "aggind"             << true  << "-1"       << true    << getErrorCode(optMsgValueError);

    // TODO notice difference between defined of "localimplied 55" and "aggind -3"

    // double option
    QTest::newRow("divfltlo 1e-5")          << "divfltlo 1e-5"        << "divfltlo"             << true  << "1e-5"    << false   << -1;
    QTest::newRow(" epgap  0.00001")       << " epgap  0.00001"       << "epgap"                << true  << "0.00001" << false   << -1;

    // dot option
    QTest::newRow("x.feaspref 0.0009")      << "x.feaspref 0.0009"         << ".feaspref"          << true  << "0.0009"  << false   << -1;
    QTest::newRow("xyz.benderspartition 3") << "xyz.benderspartition 3"    << ".benderspartition"  << true  << "3"       << false   << -1;
    QTest::newRow("z.feasopt 0.0001")       << "x.feasopt 0.0009"          << ".feasopt"           << false << "0"       << true    << getErrorCode(optMsgUserError);

    QTest::newRow("  startalg 4")      << "  startalg 4"        << "startalg"     << true  << "4" << false   << -1;
}

void TestOptionAPI::testReadFromStr()
{
    QFETCH(QString, optionStr);
    QFETCH(QString, optionName);
    QFETCH(bool, defined);
    QFETCH(QString, optionValue);
    QFETCH(bool, errorRead);
    QFETCH(int, errorCode);

    // when
    optReadFromStr(mOPTHandle, optionStr.toLatin1());

    // then
    int count = optMessageCount(mOPTHandle);
    // then
    bool foundError = false;
    int itype;
    char svalue[GMS_SSSIZE];
    QStringList messages;
    for (int i = 1; i <= count; ++i) {
        optGetMessage(mOPTHandle, i, svalue, &itype );
        if (itype !=6 && itype != 7)
            messages.append(QString("#Message: %1 : %2 : %3").arg(i).arg(svalue).arg(itype));
        if (itype == errorCode) {
            foundError = true;
        }
    }

//    for(QString m : messages)
//        qDebug() << m;

    if (!messages.isEmpty())
        QVERIFY( foundError );
    else
        QVERIFY( !foundError );

    QCOMPARE( errorRead, foundError );

    // cleanup
    optClearMessages(mOPTHandle);

    QCOMPARE( defined, isDefined(optionName) );
    QVERIFY( hasDefinedValue(optionName, optionValue) );

    // cleanup
    optResetAll(mOPTHandle);
}

void TestOptionAPI::cleanupTestCase()
{
    optFree(&mOPTHandle);
}

int TestOptionAPI::getErrorCode(optMsgType type)
{
    return type;
}

bool TestOptionAPI::isDefined(QString &optionName)
{
    for (int i = 1; i <= optCount(mOPTHandle); ++i) {
        int idefined;
        int itype;
        int iopttype;
        int ioptsubtype;
        int idummy;
        int irefnr;
        optGetInfoNr(mOPTHandle, i, &idefined, &idummy, &irefnr, &itype, &iopttype, &ioptsubtype);

        char name[GMS_SSSIZE];
        int group = 0;
        int helpContextNr;
        optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);

        // Not defined (defined==0) means either hidden or deprecated
        if (optionName.compare(QString::fromLatin1(name), Qt::CaseInsensitive)==0) {
            return (idefined != 0);
        }

    }
    return false;
}

bool TestOptionAPI::hasDefinedValue(QString &optionName, QString &optionValue)
{
    if (optionValue.isEmpty())
        return true;

    for (int i = 1; i <= optCount(mOPTHandle); ++i) {
        int idefined;
        int itype;
        int iopttype;
        int ioptsubtype;
        int idummy;
        int irefnr;
        optGetInfoNr(mOPTHandle, i, &idefined, &idummy, &irefnr, &itype, &iopttype, &ioptsubtype);

        char name[GMS_SSSIZE];
        int group = 0;
        int helpContextNr;
        optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);

        if (optionName.compare(QString::fromLatin1(name), Qt::CaseInsensitive)!=0)
            continue;

        int ivalue;
        double dvalue;
        char svalue[GMS_SSSIZE];

        optGetValuesNr(mOPTHandle, i, name, &ivalue, &dvalue, svalue);
        bool ok = false;
        switch(itype) {
        case optDataInteger: {
            int i = optionValue.toInt(&ok);
            return ( ok && i==ivalue );
        }
        case optDataDouble: {
            double d = optionValue.toDouble(&ok);
            return ( ok && qFabs(d - dvalue) < 0.000000001 );
        }
        case optDataString: {
            return (optionValue.compare( QString::fromLatin1(svalue), Qt::CaseInsensitive) == 0);
        }
        case optDataStrList: {
            QStringList strList;
            for (int j = 1; j <= optListCountStr(mOPTHandle, name ); ++j) {
               optReadFromListStr( mOPTHandle, name, j, svalue );
               strList << QString::fromLatin1(svalue);
            }
            return strList.contains(optionValue, Qt::CaseInsensitive);
        }
        case optDataNone:
        default:
            break;
        }

    }
    return true;
}


QTEST_MAIN(TestOptionAPI)
