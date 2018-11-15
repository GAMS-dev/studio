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
    QTest::addColumn<bool>("recentlyDefined");
    QTest::addColumn<QString>("optionValue");
    QTest::addColumn<bool>("errorRead");
    QTest::addColumn<int>("errorCode");

    // comment
    QTest::newRow("*---------------------------------------------------------------------------*")
            << "*---------------------------------------------------------------------------*"
            << "" << false << false << "" << true << getErrorCode(optMsgUserError);
    QTest::newRow("* ask CPLEX to construct an OPT file tuned for the problem")
            << "* ask CPLEX to construct an OPT file tuned for the problem"
            << "" << false << false << "" << true << getErrorCode(optMsgUserError);
    QTest::newRow("*---------------------------------------------------------------------------*")
            << "*---------------------------------------------------------------------------*"
            << "" << false << false << "" << true << getErrorCode(optMsgUserError);

    QTest::newRow("*iterationlim 400000")  << "*iterationlim 400000"  << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgUserError);
    QTest::newRow("*  lpmethod 1" )        << "*  lpmethod 1"         << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgUserError);

    // empty string
    QTest::newRow("                    ")  << "                    "  << "                    " << false << false << "n/a"        << false   << -1;
    QTest::newRow(" ")                     << " "                     << " "                    << false << false << "n/a"        << false   << -1;
    QTest::newRow("" )                     << ""                      << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgValueWarning);

    // indicator
    QTest::newRow("indic constr01$y 0")            << "indic constr01$y 0"             << ""    << false  << false  << "n/a"      << false   << -1;
    QTest::newRow("indic equ1(i,j,k)$bin1(i,k) 1") << "indic equ1(i,j,k)$bin1(i,k) 1"  << ""    << false  << false  << "n/a"      << false   << -1;

    QTest::newRow("secret abc 1 34")       << "secret abc 1 34"       << ""                     << true  << true << "abc 1 34"    << false   << -1;
    QTest::newRow("secret def 1 34")       << "secret def 1 34"       << ""                     << true  << true << "def 1 34"   << false   << -1;

    // deprecated option
    QTest::newRow("SCALE -1")              << "SCALE -1"              << "SCALE"                << true  << true << "-1"        << true    << getErrorCode(optMsgDeprecated);
    QTest::newRow("iterationlim 400000")   << "iterationlim 400000"   << "iterationlim"         << true  << true << "400000"    << true    << getErrorCode(optMsgDeprecated);

}

void TestOptionAPI::testReadFromStr()
{
    QFETCH(QString, optionStr);
    QFETCH(QString, optionName);
    QFETCH(bool, defined);
    QFETCH(bool, recentlyDefined);
    QFETCH(QString, optionValue);
    QFETCH(bool, errorRead);
    QFETCH(int, errorCode);

    // given
    optResetAllRecent( mOPTHandle );

    // when
    optReadFromStr(mOPTHandle, optionStr.toLatin1());
    int messageType = logAndClearMessage();
    QCOMPARE( messageType, errorCode );
    QCOMPARE( messageType != -1, errorRead );

    // then
    isDefined(  defined, recentlyDefined, optionValue );

    // cleanup
    logAndClearMessage();
    optResetAll(mOPTHandle);
}

void TestOptionAPI::testReadIntOptionFromStr_data()
{
    QVERIFY( Dcreated && optdefRead );

    QTest::addColumn<QString>("optionStr");
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("defined");
    QTest::addColumn<bool>("recentlyDefined");
    QTest::addColumn<QString>("optionValue");
    QTest::addColumn<bool>("errorRead");
    QTest::addColumn<int>("errorCode");

    // row                                     | "optionStr"              | "optionName"         |"defined" |"definedR" |"optionValue" | "errorRead" | "errorCode"
    QTest::newRow("mipinterval 2")            << "mipinterval 2"          << "mipinterval"       << true    << true     << "2"        << false       << -1;
    QTest::newRow("solnpoolcapacity=11000")   << "solnpoolcapacity=11000" << "solnpoolcapacity"  << true    << true     << "11000"    << false       << -1;
    QTest::newRow("mipinterval -2")           << "mipinterval -2"         << "mipinterval"       << true    << true     << "-2"       << false       << -1;
    QTest::newRow("aggind -3")                << "aggind -3"              << "aggind"            << true    << true     << "-1"       << true        << getErrorCode(optMsgValueError);

    // notice difference between defined of "localimplied 55" and "aggind -3"
    // enumimt option
    QTest::newRow("advind=1")                 << "advind=1"               << "advind"            << true    << true     << "1"        << false       << -1;
    QTest::newRow("advind=-1")                << "advind=-1"              << "advind"            << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);
    QTest::newRow("advind 0.2")               << "advind 0.2"             << "advind"            << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);

    QTest::newRow("  lpmethod 4")             << "  lpmethod 4"          << "lpmethod"           << true    << true     << "4"        << false       << -1;
    QTest::newRow("lpmethod 9")               << "lpmethod 9"            << "lpmethod"           << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);
    QTest::newRow("lpmethod 19")              << "lpmethod 19"           << "lpmethod"           << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);

    QTest::newRow("localimplied -1")          << "localimplied -1"       << "localimplied"       << true    << true     << "-1"       << false       << -1;
    QTest::newRow("localimplied 55")          << "localimplied 55"       << "localimplied"       << false   << false     << "n/a"     << true        << getErrorCode(optMsgValueError);

    // boolean option
    QTest::newRow("  iis yes" )                 << "  iis yes"                   << "iis"                      << true    << true     << "1"       << false   << -1;
    QTest::newRow("iis yes")                    << "iis yes"                     << "iis"                      << true    << true     << "1"       << false   << -1;
    QTest::newRow("benderspartitioninstage no") << "benderspartitioninstage no"  << "benderspartitioninstage"  << true    << true     << "0"       << false   << -1;
}

void TestOptionAPI::testReadIntOptionFromStr()
{
    QFETCH(QString, optionStr);
    QFETCH(QString, optionName);
    QFETCH(bool, defined);
    QFETCH(bool, recentlyDefined);
    QFETCH(QString, optionValue);
    QFETCH(bool, errorRead);
    QFETCH(int, errorCode);

    // given
    optResetAllRecent( mOPTHandle );

    // when
    optReadFromStr(mOPTHandle, optionStr.toLatin1());
    int messageType = logAndClearMessage();
    QCOMPARE( messageType, errorCode );
    QCOMPARE( messageType != -1, errorRead );

    // then
    isDefined(  defined, recentlyDefined, optionValue );

    // cleanup
    logAndClearMessage();
    optResetAll(mOPTHandle);
}

void TestOptionAPI::testReadDoubleOptionFromStr_data()
{
    QVERIFY( Dcreated && optdefRead );

    QTest::addColumn<QString>("optionStr");
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("defined");
    QTest::addColumn<bool>("recentlyDefined");
    QTest::addColumn<QString>("optionValue");
    QTest::addColumn<bool>("errorRead");
    QTest::addColumn<int>("errorCode");

    // row                                 | "optionStr"           | "optionName" |"defined" |"definedR" |"optionValue" | "errorRead" | "errorCode"
    // double option
    QTest::newRow("divfltlo 1e-5")         << "divfltlo 1e-5"      << "divfltlo"  << true    << true    << "1e-5"       << false      << -1;
    QTest::newRow(" epgap  0.00001")       << " epgap  0.00001"    << "epgap"     << true    << true    << "0.00001"    << false      << -1;
    QTest::newRow("tilim 2.1")             << "tilim 2.1"          << "epgap"     << true    << true    << "2.1"        << false      << -1;

    QTest::newRow("divfltup abc12-345")   << "divfltup abc12-345"  << "divfltup"  << false   << false   << "n/a"        << true      << getErrorCode(optMsgValueError);
    QTest::newRow("epgap -0.1")           << "epgap -0.1"          << "epgap"     << true    << true    << "0"          << true      << getErrorCode(optMsgValueError);
    QTest::newRow("epint=-0.9")           << "epint=-0.9"          << "epint"     << true    << true    << "0"          << true      << getErrorCode(optMsgValueError);
}

void TestOptionAPI::testReadDoubleOptionFromStr()
{
    QFETCH(QString, optionStr);
    QFETCH(QString, optionName);
    QFETCH(bool, defined);
    QFETCH(bool, recentlyDefined);
    QFETCH(QString, optionValue);
    QFETCH(bool, errorRead);
    QFETCH(int, errorCode);

    // given
    optResetAllRecent( mOPTHandle );

    // when
    optReadFromStr(mOPTHandle, optionStr.toLatin1());
    int messageType = logAndClearMessage();
    QCOMPARE( messageType, errorCode );
    QCOMPARE( messageType != -1, errorRead );

    // then
    isDefined(  defined, recentlyDefined, optionValue );

    // cleanup
    logAndClearMessage();
    optResetAll(mOPTHandle);
}

void TestOptionAPI::testReadStringOptionFromStr_data()
{
    QVERIFY( Dcreated && optdefRead );

    QTest::addColumn<QString>("optionStr");
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("defined");
    QTest::addColumn<bool>("recentlyDefined");
    QTest::addColumn<QString>("optionValue");
    QTest::addColumn<bool>("errorRead");
    QTest::addColumn<int>("errorCode");

    // row                                 | "optionStr"              | "optionName"          |"defined" |"definedR" |"optionValue" | "errorRead" | "errorCode"
    QTest::newRow("RERUN=YES")             << "RERUN=YES"             << "RERUN"              << true    << true     << "YES"       << false      << -1;

    QTest::newRow("cuts 2")                << "cuts 2"                << "cuts"               << true    << true     << "2"         << false      << -1;

    // strlist option
//    QTest::newRow("tuning str1 str2 str3") << "tuning str1 str2 str3" << "tuning"               << true  << "str1 str2 str3"  << false   << -1;
//    QTest::newRow("tuning str4 str5")      << "tuning str4 str5"      << "tuning"               << true  << "str4 str5"       << false   << -1;
//    QTest::newRow("tuning multple strlist1") << "tuning str1 str2 str3 \n tuning str4 str5"    << "tuning"   << true  << "\"str1 str2 str3\""  << false   << -1;
//    QTest::newRow("tuning multple strlist2") << "tuning str1 str2 str3 tuning str4 str5"    << "tuning"   << true  << "\"str4 str5\""       << false   << -1;
}

void TestOptionAPI::testReadStringOptionFromStr()
{
    QFETCH(QString, optionStr);
    QFETCH(QString, optionName);
    QFETCH(bool, defined);
    QFETCH(bool, recentlyDefined);
    QFETCH(QString, optionValue);
    QFETCH(bool, errorRead);
    QFETCH(int, errorCode);

    // given
    optResetAllRecent( mOPTHandle );

    // when
    optReadFromStr(mOPTHandle, optionStr.toLatin1());
    int messageType = logAndClearMessage();
    QCOMPARE( messageType, errorCode );
    QCOMPARE( messageType != -1, errorRead );

    // then
    isDefined(  defined, recentlyDefined, optionValue );

    // cleanup
    logAndClearMessage();
    optResetAll(mOPTHandle);
}

void TestOptionAPI::testReadDotOptionFromStr_data()
{
    QVERIFY( Dcreated && optdefRead );

    QTest::addColumn<QString>("optionStr");
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<bool>("defined");
    QTest::addColumn<bool>("recentlyDefined");
    QTest::addColumn<QString>("optionValue");
    QTest::addColumn<bool>("errorRead");
    QTest::addColumn<int>("errorCode");

    // row                                         | "optionStr"               | "optionName"          |"defined" |"definedR" |"optionValue" | "errorRead" | "errorCode"
    // dot option
    QTest::newRow("x.feaspref 0.0006")            << "x.feaspref 0.0006"       << ".feaspref"          << true   << true    << "0.0006"    << false      << -1;
    QTest::newRow("x.feaspref 0.0007")            << "x.feaspref 0.0007"       << ".feaspref"          << true   << true    << "0.0007"    << false      << -1;
    QTest::newRow("x.feaspref xxxxx")             << "x.feaspref xxxxx"        << ".feaspref"          << false  << false   << "n/a"       << true       << getErrorCode(optMsgValueError);
    QTest::newRow("y.feaspref(*) 0.0008")         << "y.feaspref(*) 0.0008"    << ".feaspref"          << true   << true    << "0.0008"    << false      << -1;
    QTest::newRow("z.feaspref(*,*) 4")            << "z.feaspref(*,*) 4"        << ".feaspref"         << true   << true    << "4"         << false      << -1;
    QTest::newRow("xyz.benderspartition 3")       << "xyz.benderspartition 3"  << ".benderspartition"  << true   << true    << "3"         << false      << -1;

    QTest::newRow("y.feaspref(i) 0.0008")         << "y.feaspref(i) 0.0008"    << ".feaspref"          << false   << false    << "n/a"      << true       << getErrorCode(optMsgUserError);
    QTest::newRow("*z.feaspref(*,*) 4")           << "*z.feaspref(*,*) 4"      << ".feaspref"          << false   << false   << "n/a"       << true       << getErrorCode(optMsgUserError);
    QTest::newRow("* z.feaspref(*,*) 4")          << "* z.feaspref(*,*) 4"     << ".feaspref"          << false   << false   << "n/a"       << true       << getErrorCode(optMsgUserError);
    QTest::newRow("z.feasssopt 0.0001")           << "z.feasssopt 0.0009"      << ".feasssopt"         << false   << false   << "n/a"       << true       << getErrorCode(optMsgUserError);
}

void TestOptionAPI::testReadDotOptionFromStr()
{
    QFETCH(QString, optionStr);
    QFETCH(QString, optionName);
    QFETCH(bool, defined);
    QFETCH(bool, recentlyDefined);
    QFETCH(QString, optionValue);
    QFETCH(bool, errorRead);
    QFETCH(int, errorCode);

    // given
    optResetAllRecent( mOPTHandle );

    // when
    optReadFromStr(mOPTHandle, optionStr.toLatin1());
    int messageType = logAndClearMessage();
    QCOMPARE( messageType, errorCode );
    QCOMPARE( messageType != -1, errorRead );

    // then
    isDefined(  defined, recentlyDefined, optionValue );
    logAndClearMessage();

    int optcount = -1;
    optDotOptCount(mOPTHandle, &optcount);
    qDebug() << "DOTOptCount:" << optcount;

    char msg[GMS_SSSIZE];
    int ival;
    for (int i = 1; i <= optMessageCount(mOPTHandle); i++ ) {
        optGetMessage( mOPTHandle, i, msg, &ival );
         qDebug() << QString("#DOTMessage: %1 : %2 : %3").arg(i).arg(msg).arg(ival);
    }
    // cleanup
    logAndClearMessage();
    optResetAll(mOPTHandle);
}

void TestOptionAPI::cleanupTestCase()
{
    optFree(&mOPTHandle);
}

int TestOptionAPI::logAndClearMessage()
{
    int messageType = -1;
    int ival;
    char msg[GMS_SSSIZE];
    int count = optMessageCount(mOPTHandle);
    for (int i = 1; i <= count; i++ ) {
        optGetMessage( mOPTHandle, i, msg, &ival );
        qDebug() << QString("#Message: %1 : %2 : %3").arg(i).arg(msg).arg(ival);
        if (ival !=6 && ival != 7)
            messageType = ival;
    }
    optClearMessages(mOPTHandle);
    return messageType;
}

int TestOptionAPI::getErrorCode(optMsgType type)
{
    return type;
}

void TestOptionAPI::isDefined(bool defined, bool definedR, QString &optionValue)
{
//    int nr, ref;
//    optFindStr( mOPTHandle, optionName.toLatin1(), &nr, &ref);
//    qDebug() << QString("%1: %2 %3").arg(optionName).arg(nr).arg(ref);
//    qDebug() << QString("%1: %2").arg(optionName).arg(optLookUp( mOPTHandle,optionName.toLatin1()));

    for (int i = 1; i <= optCount(mOPTHandle); ++i) {
        int idefined, idefinedR, irefnr, itype, iopttype, ioptsubtype;
        optGetInfoNr(mOPTHandle, i, &idefined, &idefinedR, &irefnr, &itype, &iopttype, &ioptsubtype);

        if (idefined || idefinedR) {
            char name[GMS_SSSIZE];
            int group = 0;
            int helpContextNr;
            optGetOptHelpNr(mOPTHandle, i, name, &helpContextNr, &group);

            qDebug() << QString("%1: %2: %3 %4 %5 [%6 %7 %8]").arg(name).arg(i)
                     .arg(idefined).arg(idefinedR).arg(irefnr).arg(itype).arg(iopttype).arg(ioptsubtype);
            QCOMPARE( idefined == 1, defined );
            QCOMPARE( idefinedR == 1, definedR );

//            int idxOption, idxVarEqu;
//            double value;
//            char varEquName[GMS_SSSIZE];
//            optGetDotOptNr(mOPTHandle, i, varEquName, &idxOption, &idxVarEqu, &value);
//            qDebug() << QString("%1.: %2: %3 %4 %5").arg(varEquName).arg(i).arg(idxOption).arg(idxVarEqu).arg(value);

            int ivalue;
            double dvalue;
            char svalue[GMS_SSSIZE];

            optGetValuesNr(mOPTHandle, i, name, &ivalue, &dvalue, svalue);
            bool ok = false;
            switch(itype) {
            case optDataInteger: {  // 1
                qDebug() << QString("%1: %2: dInt %3 %4 %5").arg(name).arg(i).arg(ivalue).arg(dvalue).arg(svalue);
                int i = optionValue.toInt(&ok);
                QVERIFY( ok && i==ivalue );
                return;
            }
            case optDataDouble: {  // 2
                qDebug() << QString("%1: %2: dDouble %3 %4 %5").arg(name).arg(i).arg(ivalue).arg(dvalue).arg(svalue);
                double d = optionValue.toDouble(&ok);
                QVERIFY( ok && qFabs(d - dvalue) < 0.000000001 );
                return;
            }
            case optDataString: {  // 3
                qDebug() << QString("%1: %2: dString %3 %4 %5").arg(name).arg(i).arg(ivalue).arg(dvalue).arg(svalue);
                QVERIFY( optionValue.compare( QString::fromLatin1(svalue), Qt::CaseInsensitive) == 0);
                return;
            }
            case optDataStrList: {  // 4
                QStringList strList;
                for (int j = 1; j <= optListCountStr(mOPTHandle, name ); ++j) {
                   optReadFromListStr( mOPTHandle, name, j, svalue );
                   qDebug() << QString("%1: %2: dStrList #%4 %5").arg(name).arg(i).arg(j).arg(svalue);
                   strList << QString::fromLatin1(svalue);
                }
                QVERIFY( strList.contains(optionValue, Qt::CaseInsensitive) );
                return;
            }
            case optDataNone:
            default: break;
            }
        }

    }
    QCOMPARE( defined , false );
    QCOMPARE( definedR, false );
}

QTEST_MAIN(TestOptionAPI)
