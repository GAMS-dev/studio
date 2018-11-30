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
#include "testcheckforupdatewrapper.h"
#include "checkforupdatewrapper.h"
#include "commonpaths.h"

#include <QDebug>

using namespace gams::studio;
using namespace gams::studio::support;

void TestCheckForUpdateWrapper::initTestCase()
{
    CommonPaths::setSystemDir();
}

void TestCheckForUpdateWrapper::testCheckForUpdateWrapper()
{
    CheckForUpdateWrapper c4uWrapper;
}

void TestCheckForUpdateWrapper::testIsValid()
{
    CheckForUpdateWrapper c4uWrapper;
    QVERIFY(c4uWrapper.isValid());
}

void TestCheckForUpdateWrapper::testMessage()
{
    CheckForUpdateWrapper c4uWrapper;
    QVERIFY(c4uWrapper.message().isEmpty());
}

void TestCheckForUpdateWrapper::testClearMessage()
{
    CheckForUpdateWrapper c4uWrapper;
    c4uWrapper.clearMessages();
    QVERIFY(c4uWrapper.message().isEmpty());
}

void TestCheckForUpdateWrapper::testCheckForUpdateWrapper()
{

}

QTEST_MAIN(TestCheckForUpdateWrapper)
