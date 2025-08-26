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
#ifndef TESTABSTRACTPROCESS_H
#define TESTABSTRACTPROCESS_H

#include <QtTest/QTest>

#include "abstractprocess.h"

namespace gams {
namespace studio {
class AbstractProcess;
}
}

class DummyProcess : public gams::studio::AbstractProcess {
    Q_OBJECT

public:
    DummyProcess(QObject *parent = nullptr);

    virtual ~DummyProcess() override;

    virtual void execute() override;

    virtual QProcess::ProcessState state() const override;

protected slots:
    virtual void readStdOut() override;
    virtual void readStdErr() override;
};

class TestAbstractProcess : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInputFile();
    void testExecute();
    void testInterrupt();
    void testTerminate();
    void testState();
    void testApplication();

    void testParameters_data();
    void testParameters();

    void testDefaultParameters();
    void testWorkingDirectory();
    void testGroupId();
    void testExitCode();

    void testFinishConnectable();
    void testNewStdChannelDataConnectable();
    void testStateChangedConnectable();
    void testNewProcessCallConnectable();

private:
    gams::studio::AbstractProcess *process;
};

#endif // TESTABSTRACTPROCESS_H
