/*
 * This file is part of the GAMS Studio project.
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
#include <QStandardPaths>
#include <QtMath>

#include "common.h"
#include "commonpaths.h"
#include "testoptionapi.h"
#include "file/filetype.h"

using gams::studio::CommonPaths;
using gams::studio::FileType;
using gams::studio::FileKind;

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

    // test minos for now
    QString optdef = "optminos.def";
    if (optReadDefinition(mOPTHandle, QDir(CommonPaths::systemDir()).filePath(optdef).toLatin1())) {
        optdefRead = false;
        QFAIL( QString("Fail to read option file '%1' from '%2'").arg(optdef, CommonPaths::systemDir()).toLatin1() );
    } else {
        optdefRead = true;
    }
}

void TestOptionAPI::testOptFileSuffix_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("validAsOpt");
    QTest::addColumn<FileKind>("expectedKind");

    QTest::newRow("opt")   << "cplex.opt"    << true << FileKind::Opt;
    QTest::newRow("op2")   << "cplex.op2"    << true << FileKind::Opt;
    QTest::newRow("OP2")   << "cplex.OP2"    << true << FileKind::Opt;
    QTest::newRow("op9")   << "cplex.op9"    << true << FileKind::Opt;
    QTest::newRow("Op9")   << "cplex.Op9"    << true << FileKind::Opt;
    QTest::newRow("o10")   << "cplex.o10"    << true << FileKind::Opt;
    QTest::newRow("101")   << "cplex.101"    << true << FileKind::Opt;
    QTest::newRow("1222")  << "cplex.1222"   << true << FileKind::Opt;
    QTest::newRow("98765") << "cplex.98765"  << true << FileKind::Opt;

    QTest::newRow("Opt")   << "cplex.Opt"    << true << FileKind::Opt;
    QTest::newRow("OPt")   << "cplex.OPt"    << true << FileKind::Opt;
    QTest::newRow("opT")   << "cplex.opT"    << true << FileKind::Opt;
    QTest::newRow("OPT")   << "cplex.OPT"    << true << FileKind::Opt;

    QTest::newRow("notopt")   << "notASolver.opt"  << false << FileKind::None;
    QTest::newRow("notop2")   << "notASolver.op2"  << false << FileKind::None;

    QTest::newRow("opt9")   << "cplex.opt9"   << false << FileKind::None;
    QTest::newRow("opt99")  << "cplex.op99"   << false << FileKind::None;
    QTest::newRow("opt999") << "cplex.op999"  << false << FileKind::None;
    QTest::newRow("optt")   << "cplex.optt"   << false << FileKind::None;
    QTest::newRow("opt_1")  << "cplex.opt_1"  << false << FileKind::None;

    QTest::newRow("op0")    << "cpelx.op0"    << false << FileKind::None;
    QTest::newRow("op1")    << "cpelx.op1"    << false << FileKind::None;
    QTest::newRow("op01")   << "cpelx.op01"   << false << FileKind::None;
    QTest::newRow("op10")   << "cpelx.op10"   << false << FileKind::None;
    QTest::newRow("op99")   << "cpelx.op99"   << false << FileKind::None;
    QTest::newRow("op123")  << "cpelx.op123"  << false << FileKind::None;

    QTest::newRow("o1")     << "cplex.o1"     << false << FileKind::None;
    QTest::newRow("o02")    << "cplex.o02"    << false << FileKind::None;
    QTest::newRow("o123")   << "cplex.o123"   << false << FileKind::None;
    QTest::newRow("ox10")   << "cplex.ox10"   << false << FileKind::None;

    QTest::newRow("012")    << "cplex.012"    << false << FileKind::None;
    QTest::newRow("23")     << "cplex.23"     << false << FileKind::None;
    QTest::newRow("1p2")    << "cplex.1p2"    << false << FileKind::None;
    QTest::newRow("12t34")  << "cplex.12t34"  << false << FileKind::None;
    QTest::newRow("2pt")    << "cplex.2pt"    << false << FileKind::None;

    QTest::newRow("opt.opt")    << "cplex.opt.opt"    << false << FileKind::None;
    QTest::newRow("opt.op1")    << "cplex.opt.op1"    << false << FileKind::None;
    QTest::newRow("opt.o12")    << "cplex.opt.o12"    << false << FileKind::None;
    QTest::newRow("opt.100")    << "cplex.opt.100"    << false << FileKind::None;
    QTest::newRow("opt.101")    << "cplex.opt.101"    << false << FileKind::None;
    QTest::newRow("opt.123")    << "cplex.opt.123"    << false << FileKind::None;
    QTest::newRow("opt.1234")   << "cplex.opt.1234"   << false << FileKind::None;
    QTest::newRow("op1.opt")    << "cplex.op1.opt"    << false << FileKind::None;
    QTest::newRow("o12.opt")    << "cplex.o12.opt"    << false << FileKind::None;
    QTest::newRow("123.opt")    << "cplex.123.opt"    << false << FileKind::None;
    QTest::newRow("1234.opt")   << "cplex.1234.opt"   << false << FileKind::None;

    QTest::newRow("gsp")    << "xyz.gsp"    << false << FileKind::Gsp ;
    QTest::newRow("pro")    << "xyz.pro"    << false << FileKind::Gsp ;
    QTest::newRow("gms")    << "xyz.gms"    << false << FileKind::Gms ;
    QTest::newRow("inc")    << "xyz.inc"    << false << FileKind::Gms ;
    QTest::newRow("txt")    << "xyz.txt"    << false << FileKind::Txt ;
    QTest::newRow("log")    << "xyz.log"    << false << FileKind::TxtRO ;
    QTest::newRow("lst")    << "xyz.lst"    << false << FileKind::Lst ;
    QTest::newRow("lxi")    << "xyz.lxi"    << false << FileKind::Lxi ;
    QTest::newRow("gdx")    << "xyz.gdx"    << false << FileKind::Gdx ;
    QTest::newRow("ref")    << "xyz.ref"    << false << FileKind::Ref ;
    QTest::newRow("~log")   << "xyz.~log"   << false << FileKind::Log ;
    QTest::newRow("efi")    << "xyz.efi"    << false << FileKind::Efi ;

    QTest::newRow("Guc")   << "gamsconfig.yaml"     << false << FileKind::Guc ;
    QTest::newRow("yaml")  << "notAgamsconfig.yaml" << false << FileKind::GCon;
    QTest::newRow("~op")   << "abc.~op"             << false << FileKind::None;
}

void TestOptionAPI::testOptFileSuffix()
{
    QFETCH(QString, fileName);
    QFETCH(bool, validAsOpt);
    QFETCH(FileKind, expectedKind);

    QCOMPARE(validAsOpt, FileKind::Opt == FileType::from(fileName).kind());
    QCOMPARE(validAsOpt, FileType::from(FileKind::Opt) == FileType::from(fileName));
    QCOMPARE(validAsOpt, QString::compare(FileType::from(fileName).defaultSuffix(), "opt", Qt::CaseInsensitive)==0 );
    QCOMPARE(FileType::from(expectedKind), FileType::from(fileName));
}

void TestOptionAPI::testEOLChars()
{
    char eolchars[256];
    int numchar = optEOLChars( mOPTHandle, eolchars);

    QCOMPARE( 1, numchar );
    QCOMPARE( "*", QString::fromLatin1(eolchars) );
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
    QTest::newRow("*--------------------------------------------------------------------------*")
            << "*--------------------------------------------------------------------------*"
            << "" << false << false << "" << false << -1;
    QTest::newRow("* ask MINOS to construct an OPT file tuned for the problem")
            << "* ask CPLEX to construct an OPT file tuned for the problem"
            << "" << false << false << "" << false << -1;
    QTest::newRow("*---------------------------------------------------------------------------*")
            << "*---------------------------------------------------------------------------*"
            << "" << false << false << "" << false << -1;

    QTest::newRow("*major iterations 400")  << "*major iterations 400"  << ""                     << false << false << "n/a"        << false    << -1;
    QTest::newRow("*  multiple pric 1" )    << "*  multiple pric 1"     << ""                     << false << false << "n/a"        << false    << -1;

    // empty string
    QTest::newRow("                    ")  << "                    "  << "                    " << false << false << "n/a"        << false   << -1;
    QTest::newRow(" ")                     << " "                     << " "                    << false << false << "n/a"        << false   << -1;
    QTest::newRow("" )                     << ""                      << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgValueWarning);

    QTest::newRow("=" )                    << "="                     << ""                     << false << false << "n/a"        << true    << getErrorCode(optMsgUserError);

    // unknown option
    QTest::newRow("unknown -1")                 << "unknown -1"                 << "unknown"                << false  << false << "n/a"       << true    << getErrorCode(optMsgUserError);
    QTest::newRow("what 0.1234")                << "what 0.1234"                << "what"                   << false  << false << "n/a"       << true    << getErrorCode(optMsgUserError);
    QTest::newRow("feasibility_tolerance 0.9")  << "feasibility_tolerance 0.9"  << "feasibility_tolerance"  << false  << false << "n/a"       << true    << getErrorCode(optMsgUserError);
    QTest::newRow("scale_no")                   << "scale_no"                   << "scale_no"               << false  << false << "n/a"       << true    << getErrorCode(optMsgUserError);

    // integer option
    QTest::newRow("summary frequency 2")        << "summary frequency 2"    << "summary frequency"  << true    << true     << "2"       << false       << -1;
    QTest::newRow("hessian dimension 9")        << "hessian dimension 9"    << "hessian dimension"  << true    << true     << "9"       << false       << -1;

    QTest::newRow("hessian dimension 9 4")      << "hessian dimension 9 4"  << "hessian dimension"   << false   << false   << "n/a"     << true        << getErrorCode(optMsgValueError);

    // enumimt option
    QTest::newRow("crash option 1")             << "crash option 1"         << "crash option"      << true    << true     << "1"        << false       << -1;
    QTest::newRow("scale option=2")             << "scale option=2"         << "scale option"      << true    << true     << "2"        << false       << -1;
    QTest::newRow("verify level 99")            << "verify level 99"        << "verify level"      << false   << false    << "n/a"      << true        << getErrorCode(optMsgValueError);

    // enumstr option
    QTest::newRow("completion full")                   << "completion full"                 << "completion"                << true    << true     << "full"        << false       << -1;
    QTest::newRow("start assigned nonlinears BASIC")   << "start assigned nonlinears BASIC" << "start assigned nonlinears" << true    << true     << "Basic"       << false       << -1;

    QTest::newRow("lagrangian completion full")        << "lagrangian completion full"      << "lagrangian"                << false   << false    << "n/a"         << true       <<  getErrorCode(optMsgValueError);

    // double option
    QTest::newRow("weight on linear objective 1e-5")  << "weight on linear objective 1e-5"   << "weight on linear objective"  << true    << true    << "1e-5"       << false      << -1;
    QTest::newRow("crash tolerance 0.00001")          << "crash tolerance 0.00001"           << "crash tolerance"             << true    << true    << "0.00001"    << false      << -1;
    QTest::newRow("feasibility tolerance 0.1")        << "feasibility tolerance 0.1"         << "feasibility tolerance"       << true    << true    << "0.1"        << false      << -1;
    QTest::newRow("scale tolerance=0.1")              << "scale tolerance=0.1"               << "scale tolerance"             << true    << true    << "0.1"        << false      << -1;

    // string no value option
    QTest::newRow("Verify Constraint Gradients")      << "Verify Constraint Gradients"       << "Verify Constraint Gradients"  << true    << true    << ""          << false      << -1;
    QTest::newRow("LU partial pivoting")              << "LU partial pivoting"               << "LU partial pivoting"          << true    << true    << ""          << false      << -1;
    QTest::newRow("scale NO")                         << "scale NO"                          << "scale"                        << true    << true    << ""          << false      << -1;
    QTest::newRow("verify objective gradients")       << "verify objective gradients"        << "verify objective gradients"   << true    << true    << ""          << false      << -1;

    // too many value
    QTest::newRow("Verify Constraint Gradients 0.1")  << "Verify Constraint Gradients 0.1"   << "Verify Constraint Gradients"  << true    << true    << ""          << true       << getErrorCode(optMsgUserError);
    QTest::newRow("scale NO never 0.1")               << "scale NO never 0.1"                << "scale"                        << true    << true    << ""          << true       << getErrorCode(optMsgUserError);

    // mssing value
    QTest::newRow("start assigned nonlinears")        << "start assigned nonlinears"         << "start assigned nonlinears"    << false   << false   << "n/a"        << true       << getErrorCode(optMsgUserError);
    QTest::newRow("feasibility tolerance")            << "feasibility tolerance"             << "feasibility tolerance"        << false   << false   << "n/a"        << true       << getErrorCode(optMsgUserError);
    QTest::newRow("scale option=")                    << "scale option="                     << "scale option"                 << false   << false   << "n/a"        << true       << getErrorCode(optMsgUserError);
    QTest::newRow("what scale option=")               << "what scale option="                << "what scale option"            << false   << false   << "n/a"        << true       << getErrorCode(optMsgUserError);

}

void TestOptionAPI::testReadFromStr()
{
    QFETCH(QString, optionStr);
    //QFETCH(QString, optionName);
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

void TestOptionAPI::cleanupTestCase()
{
    if (mOPTHandle)
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
