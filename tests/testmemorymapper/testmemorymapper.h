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
#ifndef TESTMEMORYMAPPER_H
#define TESTMEMORYMAPPER_H

#include "editors/memorymapper.h"
#include <QTest>

using gams::studio::MemoryMapper;

class TestMemoryMapper : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testAddLine();

//    void testReadChunk0();
//    void testReadChunk1();
//    void testMoveBackAChunk();
//    void testFetchBeyondChunk();
//    void testReadLines();
//    void testUpdateLineCounting();
//    void testPeekChunkLineNrs();
//    void testLineNrEstimation();
//    void testPosAndAnchor();

private:
    MemoryMapper *mMapper;
};

#endif // TESTMEMORYMAPPER_H
