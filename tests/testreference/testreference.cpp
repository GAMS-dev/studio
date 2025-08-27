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
#include "testreference.h"
#include "reference/referencedatatype.h"

using gams::studio::reference::ReferenceDataType;

void TestReference::testRefrenceHighlight_data()
{
    QTest::addColumn<QString>("blockText");
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("referenceType");
    QTest::addColumn<int>("column");
    QTest::addColumn<QString>("toBeMatched");
    QTest::addColumn<int>("start_column_toBeMatched");

    QTest::newRow("declare_1_b")        << "Set b / zero, one /;"    << "b"  << static_cast<int>(ReferenceDataType::ReferenceType::Declare) << 6  << "b"  << 5;
    QTest::newRow("declare_2_b")        << "Alias (b,b1,b2,b3 ,b4);" << "b3" << static_cast<int>(ReferenceDataType::ReferenceType::Declare) << 18 << "b3" << 16;
    QTest::newRow("declare_3_b")        << "Alias (b,b1,b2,b3 ,b4);" << "b4" << static_cast<int>(ReferenceDataType::ReferenceType::Declare) << 22 << "b4" << 20;

    QTest::newRow("define_1_b")         << "Set b / zero, one /;"                     << "b"          << static_cast<int>(ReferenceDataType::ReferenceType::Define) << 7  << "/"  << 7;
    QTest::newRow("define_1_portfolio") << "Model portfolio / fsum, dmean, dvar /;"   << "portfolio"  << static_cast<int>(ReferenceDataType::ReferenceType::Define) << 17 << "/"  << 17;
    QTest::newRow("define_1_v")         << "Table v(i,j) 'variance-covariance array   (%-squared annual return)'"      << "v"      << static_cast<int>(ReferenceDataType::ReferenceType::Define)   << 69 << "" << 69; // last character?
    QTest::newRow("define_1_revfac")    << "   revfac(m) 'revfac is revenue factor' / (january,february) 1,  (march,april) 1.1 /;"  << "revfac" << static_cast<int>(ReferenceDataType::ReferenceType::Define)   << 41 << "/"  << 4;
    QTest::newRow("define_1_dmean")     << "dmean.. sum(i, mean(i)*x(i))            =e= target;"                                    << "dmean" << static_cast<int>(ReferenceDataType::ReferenceType::Define)   << 12  << "("  << 12;
    QTest::newRow("define_1_param_a")   << "        / seattle    350"                                                               << "a"     << static_cast<int>(ReferenceDataType::ReferenceType::Define)   << 9   << "/"  << 9;

    QTest::newRow("assign_1_submit")  << "   submit(s) = no;"                                                                    << "submit"  << static_cast<int>(ReferenceDataType::ReferenceType::Assign )   << 18 << "submit" << 4;
    QTest::newRow("assign_2_submit")  << "   loop(s$(not (done(s) or h(s))), submit(s) = yes$(card(submit) + card(h) < maxS););" << "submit"  << static_cast<int>(ReferenceDataType::ReferenceType::Assign )   << 51 << "submit" << 36;
    QTest::newRow("control_1_submit") << "   loop(submit(s),"                                                                    << "submit"  << static_cast<int>(ReferenceDataType::ReferenceType::Control)   << 16 << "submit" << 9;
    QTest::newRow("ref_1_submit")  << "   loop(s$(not (done(s) or h(s))), submit(s) = yes$(card(submit) + card(h) < maxS););"    << "submit"  << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 64 << "submit" << 58;

    QTest::newRow("variance_ImplAssIn_1")  << "solve portfolio using nlp minimizing variance;"  << "variance" << static_cast<int>(ReferenceDataType::ReferenceType::ImplicitAssign) << 47 << "variance" << 38;
    QTest::newRow("variance_ImplAssIn_2")  << "   solve p1 minimizing variance using rminlp;"   << "variance" << static_cast<int>(ReferenceDataType::ReferenceType::ImplicitAssign) << 46 << "variance" << 24;

    QTest::newRow("control_1_i")      <<  "supply(i).. sum(j, x(i,j)) =l= a(i);"          << "i" << static_cast<int>(ReferenceDataType::ReferenceType::Control  ) << 10 << "i" << 8;

    QTest::newRow("ref_1_dcd")         << "ah.. holding    =e= sum((d,m), dcd(d,\"hold-cost\")*s(d,m));" << "dcd" << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 35 << "dcd" << 32;
    QTest::newRow("ref_1_i")       <<  "cost..      z =e= sum((i,j), c(i,j)*x(i,j))"   << "i" << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 34 << "i" << 32;
    QTest::newRow("ref_2_i")       <<  "cost..      z =e= sum((i,j), c(i,j)*x(i,j));"  << "i" << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 41 << "i" << 39;
    QTest::newRow("ref_3_i")       <<  "supply(i).. sum(j, x(i,j)) =l= a(i);"          << "i" << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 36 << "i" << 34;

    QTest::newRow("ref_1_h" )      << "display h.up, pn.lo;"  << "h"  << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 13 << "h" << 9;
    QTest::newRow("ref_1_db")      << "Model pdi / all /;"    << "db" << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 16 << "Model pdi / all /;"  << 0;

    QTest::newRow("ref_func_1")    << "pc(p,m) = pfd(p,\"prod-cost\") + floor(2**(ord(m)-1)/2);"                 << "floor"       << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 38 << "floor"       << 32;
    QTest::newRow("ref_func_2")    << "abort$errorLevel 'problems with after_loop';"                             << "errorLevel"  << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 19 << "errorLevel"  << 7;
    QTest::newRow("ref_func_3")    << "      h(s) = JobHandle;"                                                  << "JobHandle"   << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 24 << "JobHandle"   << 14;
    QTest::newRow("ref_func_4")    << "until card(done) = card(s) or timeelapsed > 100;  // wait until all jobs" << "TimeElapsed" << static_cast<int>(ReferenceDataType::ReferenceType::Reference) << 44 << "timeelapsed" << 31;

    QTest::newRow("file_1")        << "$include ct.gms"       << "$include ct.gms"   << static_cast<int>(ReferenceDataType::ReferenceType::Unknown)   << 15   << "$include ct.gms"   << 1;
}

void TestReference::testRefrenceHighlight()
{
    QFETCH(QString, blockText);
    QFETCH(QString, name);
    QFETCH(int, referenceType);
    QFETCH(int, column);
    QFETCH(QString, toBeMatched);
    QFETCH(int, start_column_toBeMatched);

    QRegularExpression re(toBeMatched);
    switch (referenceType) {
        case ReferenceDataType::Control:
        case ReferenceDataType::ImplicitAssign:
        case ReferenceDataType::Assign:
        case ReferenceDataType::Reference:
        case ReferenceDataType::Declare:
        case ReferenceDataType::Unknown: {
            QString text = blockText.left(column);
            qDebug()<< "--- match Declare [" << toBeMatched <<"] at column=" << column;
            int index = text.lastIndexOf(toBeMatched, -1, Qt::CaseInsensitive);
            QString matchedText = (index==-1 ? blockText: text.mid(index, toBeMatched.length()));
            qDebug()<< "    index="<< index<< ", matchedText=["<< matchedText<<"]";
            QCOMPARE(matchedText, toBeMatched);
            QCOMPARE(index, start_column_toBeMatched-1);
            break;
        }
        case ReferenceDataType::Define: {
            QString matchedText = blockText.mid(column-1, 1);
            QCOMPARE(matchedText, toBeMatched);
            break;
        }
        default:
            break;
    }
}


QTEST_MAIN(TestReference)
