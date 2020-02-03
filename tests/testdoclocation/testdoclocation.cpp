/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2020 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2020 GAMS Development Corp. <support@gams.com>
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
#include "testdoclocation.h"
#include "commonpaths.h"
#include "help/helpdata.h"

#include <QStandardPaths>

using gams::studio::CommonPaths;

void TestDocLocation::testSolverAnchor_data()
{
    QTest::addColumn<QString>("solverName");
    QTest::addColumn<QString>("optionName");
    QTest::addColumn<QString>("location");
    QTest::addColumn<QString>("anchor");

    auto docs = CommonPaths::documentationDir();

    QTest::newRow("BARON_AbsConFeasTol")    << "BARON" << "AbsConFeasTol" << docs + "/S_BARON.html" << "BARONAbsConFeasTol";
    QTest::newRow("BARON_NoutIter")         << "BARON" << "NoutIter"      << docs + "/S_BARON.html" << "BARONNoutIter";
    QTest::newRow("BARON_OBTTDo")           << "BARON" << "OBTTDo"        << docs + "/S_BARON.html" << "BARONOBTTDo";
    QTest::newRow("BARON_BrPtStra")         << "BARON" << "BrPtStra"      << docs + "/S_BARON.html" << "BARONBrPtStra";
    QTest::newRow("BARON_.EquClass")        << "BARON" << ".EquClass"     << docs + "/S_BARON.html" << "BARONDOTEquClass";

    QTest::newRow("CONOPT4_Flg_Crash_Basis")    << "CONOPT4" << "Flg_Crash_Basis" << docs + "/S_CONOPT4.html" << "CONOPT4Flg_Crash_Basis";
    QTest::newRow("CONOPT4_Lim_Dbg_1Drv")       << "CONOPT4" << "Lim_Dbg_1Drv"    << docs + "/S_CONOPT4.html" << "CONOPT4Lim_Dbg_1Drv";
    QTest::newRow("CONOPT4_Frq_Log_SlpSqp")     << "CONOPT4" << "Frq_Log_SlpSqp"  << docs + "/S_CONOPT4.html" << "CONOPT4Frq_Log_SlpSqp";
    QTest::newRow("CONOPT4_HessianMemFac")      << "CONOPT4" << "HessianMemFac"   << docs + "/S_CONOPT4.html" << "CONOPT4HessianMemFac";

    QTest::newRow("CPLEX_advind")    << "CPLEX" << "advind" << docs + "/S_CPLEX.html" << "CPLEXadvind";
    QTest::newRow("CPLEX_lpmethod")  << "CPLEX" << "lpmethod" << docs + "/S_CPLEX.html" << "CPLEXlpmethod";
    QTest::newRow("CPLEX_tuning")    << "CPLEX" << "tuning" << docs + "/S_CPLEX.html" << "CPLEXtuning";
    QTest::newRow("CPLEX_advind")    << "CPLEX" << "advind" << docs + "/S_CPLEX.html" << "CPLEXadvind";
    QTest::newRow("CPLEX_advind")    << "CPLEX" << "advind" << docs + "/S_CPLEX.html" << "CPLEXadvind";

    QTest::newRow("EXAMINER_dumpGamsPoint")  << "EXAMINER" << "dumpGamsPoint" << docs + "/S_EXAMINER.html" << "EXAMINERdumpGamsPoint";
    QTest::newRow("EXAMINER_fCheckDVAR")     << "EXAMINER" << "fCheckDVAR"    << docs + "/S_EXAMINER.html" << "EXAMINERfCheckDVAR";
    QTest::newRow("EXAMINER_scaleLB")        << "EXAMINER" << "scaleLB"       << docs + "/S_EXAMINER.html" << "EXAMINERscaleLB";
    QTest::newRow("EXAMINER_primalCSTol")    << "EXAMINER" << "primalCSTol"   << docs + "/S_EXAMINER.html" << "EXAMINERprimalCSTol";

    QTest::newRow("MINOS_crash option")         << "MINOS" << "crash option"         << docs + "/S_MINOS.html" << "MINOScrash_option";
    QTest::newRow("MINOS_expand frequency")     << "MINOS" << "expand frequency"     << docs + "/S_MINOS.html" << "MINOSexpand_frequency";
    QTest::newRow("MINOS_LU density tolerance") << "MINOS" << "LU density tolerance" << docs + "/S_MINOS.html" << "MINOSLU_density_tolerance";
    QTest::newRow("MINOS_unbounded objective value")  << "MINOS" << "unbounded objective value"  << docs + "/S_MINOS.html" << "MINOSunbounded_objective_value";
    QTest::newRow("MINOS_major damping parameter")    << "MINOS" << "major damping parameter"    << docs + "/S_MINOS.html" << "MINOSmajor_damping_parameter";
    QTest::newRow("MINOS_scale yes")                  << "MINOS" << "scale yes"                  << docs + "/S_MINOS.html" << "MINOSscale_yes";
    QTest::newRow("MINOS_weight on linear objective") << "MINOS" << "weight on linear objective" << docs + "/S_MINOS.html" << "MINOSweight_on_linear_objective";

    QTest::newRow("MOSEK_MSK_DPAR_ANA_SOL_INFEAS_TOL")                 << "MOSEK" << "MSK_DPAR_ANA_SOL_INFEAS_TOL"                 << docs + "/S_MOSEK.html" << "MOSEKMSK_DPAR_ANA_SOL_INFEAS_TOL";
    QTest::newRow("MOSEK_MSK_DPAR_INTPNT_CO_TOL_REL_GAP")              << "MOSEK" << "MSK_DPAR_INTPNT_CO_TOL_REL_GAP"              << docs + "/S_MOSEK.html" << "MOSEKMSK_DPAR_INTPNT_CO_TOL_REL_GAP";
    QTest::newRow("MOSEK_MSK_DPAR_LOWER_OBJ_CUT")                      << "MOSEK"  << "MSK_DPAR_LOWER_OBJ_CUT"                     << docs + "/S_MOSEK.html" << "MOSEKMSK_DPAR_LOWER_OBJ_CUT";
    QTest::newRow("MOSEK_MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT") << "MOSEK" << "MSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT" << docs + "/S_MOSEK.html" << "MOSEKMSK_DPAR_MIO_TOL_REL_DUAL_BOUND_IMPROVEMENT";
    QTest::newRow("MOSEK_SOLVEFINAL")                                  << "MOSEK" << "SOLVEFINAL"                                  << docs + "/S_MOSEK.html" << "MOSEKSOLVEFINAL";

    QTest::newRow("SCIP_gams/dumpsolutions")               << "SCIP" << "gams/dumpsolutions"                << docs + "/S_SCIP.html" << "SCIPgams_dumpsolutions";
    QTest::newRow("SCIP_gams/solvetrace/timefreq")         << "SCIP" << "gams/solvetrace/timefreq"          << docs + "/S_SCIP.html" << "SCIPgams_solvetrace_timefreq";
    QTest::newRow("SCIP_branching/cloud/maxdepthunion")    << "SCIP" << "branching/cloud/maxdepthunion"     << docs + "/S_SCIP.html" << "SCIPbranching_cloud_maxdepthunion";
    QTest::newRow("SCIP_branching/allfullstrong/priority") << "SCIP" << "branching/allfullstrong/priority"  << docs + "/S_SCIP.html" << "SCIPbranching_allfullstrong_priority";
    QTest::newRow("SCIP_conflict/conflictgraphweight")     << "SCIP" << "conflict/conflictgraphweight"      << docs + "/S_SCIP.html" << "SCIPconflict_conflictgraphweight";
    QTest::newRow("SCIP_conflict/uplockscorefac")          << "SCIP" << "conflict/uplockscorefac"           << docs + "/S_SCIP.html" << "SCIPconflict_uplockscorefac";
    QTest::newRow("SCIP_constraints/SOS1/addcomps")        << "SCIP" << "constraints/SOS1/addcomps"         << docs + "/S_SCIP.html" << "SCIPconstraints_SOS1_addcomps";
    QTest::newRow("SCIP_constraints/SOS2/eagerfreq")       << "SCIP" << "constraints/SOS2/eagerfreq"        << docs + "/S_SCIP.html" << "SCIPconstraints_SOS2_eagerfreq";
    QTest::newRow("SCIP_display/headerfreq")               << "SCIP" << "display/headerfreq"                << docs + "/S_SCIP.html" << "SCIPdisplay_headerfreq";
    QTest::newRow("SCIP_display/curconss/active")          << "SCIP" << "display/curconss/active"           << docs + "/S_SCIP.html" << "SCIPdisplay_curconss_active";
    QTest::newRow("SCIP_heuristics/actconsdiving/freq")    << "SCIP" << "heuristics/actconsdiving/freq"     << docs + "/S_SCIP.html" << "SCIPheuristics_actconsdiving_freq";
    QTest::newRow("SCIP_heuristics/zeroobj/priority")      << "SCIP" << "heuristics/zeroobj/priority"       << docs + "/S_SCIP.html" << "SCIPheuristics_zeroobj_priority";

    QTest::newRow("XPRESS_extraPresolve")     << "XPRESS" << "extraPresolve"     << docs + "/S_XPRESS.html" << "XPRESSextraPresolve";
    QTest::newRow("XPRESS_bigMMethod")        << "XPRESS" << "bigMMethod"        << docs + "/S_XPRESS.html" << "XPRESSbigMMethod";
    QTest::newRow("XPRESS_breadthFirst")      << "XPRESS" << "breadthFirst"      << docs + "/S_XPRESS.html" << "XPRESSbreadthFirst";
    QTest::newRow("XPRESS_solnpoolCapacity")  << "XPRESS" << "solnpoolCapacity"  << docs + "/S_XPRESS.html" << "XPRESSsolnpoolCapacity";
    QTest::newRow("XPRESS_eigenvalueTol")     << "XPRESS" << "eigenvalueTol"     << docs + "/S_XPRESS.html" << "XPRESSeigenvalueTol";
    QTest::newRow("XPRESS_cpuPlatform")       << "XPRESS" << "cpuPlatform"       << docs + "/S_XPRESS.html" << "XPRESScpuPlatform";
}

void TestDocLocation::testSolverAnchor()
{
    QFETCH(QString, solverName);
    QFETCH(QString, optionName);
    QFETCH(QString, location);
    QFETCH(QString, anchor);

    QCOMPARE( help::HelpData::getSolverChapterLocation(solverName), location);
    if (optionName.isEmpty())
        QVERIFY( help::HelpData::getSolverOptionAnchor(solverName, optionName).isEmpty() );
    else
        QCOMPARE( help::HelpData::getSolverOptionAnchor(solverName, optionName), anchor);
}

void TestDocLocation::testUrlLocalFile()
{
    // given
    CommonPaths::setSystemDir();
    QString docdir = CommonPaths::helpDocumentsDir();
    QString indexFile = QFileInfo(QDir(docdir), "index.html").canonicalFilePath();
    QUrl url = QUrl::fromLocalFile(indexFile);
    // when
    QString urlIndexFile = url.toLocalFile();
    // then
    QVERIFY2(QFileInfo(urlIndexFile).exists(), QString("directory: '%1' does not exist").arg(urlIndexFile).toLatin1());
}

void TestDocLocation::testLocalFileToOnlineUrl_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("fragment");
    QTest::addColumn<QString>("expectedurlstr");

    QTest::newRow("index")    << "index.html"    << ""   << "https://www.gams.com/latest/docs/index.html";
    QTest::newRow("RN 29")    << "RN_29.html"    << "RN_2910"
                              << "https://www.gams.com/latest/docs/RN_29.html#RN_2910";
    QTest::newRow("set definition")   << "UG_SetDefinition.html" << "UG_SetDefinition_Introduction"
                                      << "https://www.gams.com/latest/docs/UG_SetDefinition.html#UG_SetDefinition_Introduction";
    QTest::newRow("Gams param List")  << "UG_GamsCall.html"      << "UG_GamsCall_ListOfCommandLineParameters"
                                      << "https://www.gams.com/latest/docs/UG_GamsCall.html#UG_GamsCall_ListOfCommandLineParameters";
    QTest::newRow("Cplex Intro")      << "S_CPLEX.html"          << "CPLEX_INTRODUCTION"
                                      << "https://www.gams.com/latest/docs/S_CPLEX.html#CPLEX_INTRODUCTION";
    QTest::newRow("studio")           << "T_STUDIO.html" << ""   << "https://www.gams.com/latest/docs/T_STUDIO.html";
    QTest::newRow("C++ TransportGDX") << "apis/examples_cpp/transportGDX_8cpp.html"  << ""
                                      << "https://www.gams.com/latest/docs/apis/examples_cpp/transportGDX_8cpp.html";
    QTest::newRow("OPT API#optcount") << "apis/expert-level/optqdrep.html"           << "optCount"
                                      << "https://www.gams.com/latest/docs/apis/expert-level/optqdrep.html#optCount";
}

void TestDocLocation::testLocalFileToOnlineUrl()
{
    QFETCH(QString, filename);
    QFETCH(QString, fragment);
    QFETCH(QString, expectedurlstr);

    // given
    QString baseLocation = QDir(CommonPaths::systemDir()).canonicalPath();
    QString docdir = CommonPaths::helpDocumentsDir();
    QString docdirStr = QDir(docdir).canonicalPath();
    QString indexFileStr = QFileInfo(docdirStr, filename).canonicalFilePath();
    qDebug() << "baseLocation:" << baseLocation;
    qDebug() << "docdirstr:" << docdirStr;
    qDebug() << "indexFileStr:" << indexFileStr;

    QUrl url = QUrl::fromLocalFile(indexFileStr);
    QString urlLocalFile = url.toLocalFile();
    qDebug() << "urlLocalFile:" << urlLocalFile;
     QUrl onlineStartPageUrl = QUrl("https://www.gams.com/latest", QUrl::TolerantMode);

    // when
    int newSize = urlLocalFile.size() - urlLocalFile.indexOf(baseLocation) - baseLocation.size();
    QString newPath = urlLocalFile.right(newSize);

    QString onlinepath = onlineStartPageUrl.path();
    QStringList pathList = onlinepath.split("/", QString::SkipEmptyParts);
    pathList << newPath.split("/", QString::SkipEmptyParts) ;

    QUrl onlineUrl;
    onlineUrl.setScheme(onlineStartPageUrl.scheme());
    onlineUrl.setHost(onlineStartPageUrl.host());
    onlineUrl.setPath("/" + pathList.join("/"));

    if (!fragment.isEmpty())
        onlineUrl.setFragment(fragment);

    // then
    QUrl expectUrl(expectedurlstr, QUrl::TolerantMode);
    QVERIFY2(expectUrl==onlineUrl, QString("Expect two urls:'%1' and '%2' to be equal")
                                      .arg(expectUrl.toDisplayString())
                                      .arg(onlineUrl.toDisplayString())
                                   .toLatin1()
             );
}

void TestDocLocation::testOnlineUrlToLocalFile_data()
{
    QTest::addColumn<QString>("urlstr");
    QTest::addColumn<QString>("localfile");
    QTest::addColumn<QString>("fragment");

    QTest::newRow("index")            << "https://www.gams.com/latest/docs/index.html" << "index.html"    << "";
    QTest::newRow("RN 29")            << "https://www.gams.com/latest/docs/RN_29.html#RN_2910"
                                      << "RN_29.html"    << "RN_2910";
    QTest::newRow("set definition")   << "https://www.gams.com/latest/docs/UG_SetDefinition.html#UG_SetDefinition_Introduction"
                                      << "UG_SetDefinition.html" << "UG_SetDefinition_Introduction";
    QTest::newRow("Gams param List")  << "https://www.gams.com/latest/docs/UG_GamsCall.html#UG_GamsCall_ListOfCommandLineParameters"
                                      << "UG_GamsCall.html"      << "UG_GamsCall_ListOfCommandLineParameters";
    QTest::newRow("Cplex Intro")      << "https://www.gams.com/latest/docs/S_CPLEX.html#CPLEX_INTRODUCTION"
                                      << "S_CPLEX.html"          << "CPLEX_INTRODUCTION";
    QTest::newRow("studio")           << "https://www.gams.com/latest/docs/T_STUDIO.html" << "T_STUDIO.html" << "";
    QTest::newRow("C++ TransportGDX") << "https://www.gams.com/latest/docs/apis/examples_cpp/transportGDX_8cpp.html"
                                      << "apis/examples_cpp/transportGDX_8cpp.html"  << "";
    QTest::newRow("OPT API#optcount") << "https://www.gams.com/latest/docs/apis/expert-level/optqdrep.html#optCount"
                                      << "apis/expert-level/optqdrep.html"           << "optCount";
}

void TestDocLocation::testOnlineUrlToLocalFile()
{
    QFETCH(QString, urlstr);
    QFETCH(QString, localfile);
    QFETCH(QString, fragment);

    // given
    QString baseLocation = QDir(CommonPaths::systemDir()).canonicalPath();
    QString docdir = CommonPaths::helpDocumentsDir();
    QString docdirStr = QDir(docdir).canonicalPath();
    QString indexFileStr = QFileInfo(docdirStr, localfile).canonicalFilePath();
    qDebug() << "baseLocation:" << baseLocation;
    qDebug() << "docdirstr:" << docdirStr;
    qDebug() << "indexFileStr:" << indexFileStr;

    QUrl url(urlstr, QUrl::TolerantMode);

    // when
    int docsidx = gams::studio::help::HelpData::getURLIndexFrom(urlstr);
    QVERIFY(docsidx > -1);

    QStringList pathStrList = gams::studio::help::HelpData::HelpData::getPathList();
    QString pathStr = pathStrList.at(docsidx);
    int pathIndex = url.path().indexOf( pathStr );
    QString newPath = url.path().mid( pathIndex, url.path().size());
    QStringList pathList;
    pathList << baseLocation.split("/", QString::SkipEmptyParts) << newPath.split("/", QString::SkipEmptyParts);

    QUrl localUrl = QUrl::fromLocalFile(QString());
    localUrl.setScheme("file");
    localUrl.setPath("/" + pathList.join("/"));

    QVERIFY2(QFileInfo(localUrl.toLocalFile())==QFileInfo(indexFileStr), QString("Expect two files:'%1' and '%2' to be equal")
                                                                             .arg(localUrl.toLocalFile())
                                                                             .arg(indexFileStr)
                                                                        .toLatin1()
             );

    if (!url.fragment().isEmpty())
        localUrl.setFragment(url.fragment());
    QVERIFY(localUrl.isValid());

}

QTEST_MAIN(TestDocLocation)
