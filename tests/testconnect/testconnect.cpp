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
#include "testconnect.h"

using namespace gams::studio::connect;

void TestConnect::initTestCase()
{
    rootData << "Key" << "Value" ;

    lastid = 0;
    rootItem = new ConnectDataItem(rootData, lastid++);
}

void TestConnect::testConnectItemAttribute()
{
    QCOMPARE(rootItem->columnCount(), rootData.size());
    QCOMPARE(rootItem->childNumber(), 0);
    QVERIFY(!rootItem->parentItem());
    QVERIFY(rootItem->isFirstChild());
    QVERIFY(rootItem->isLastChild());
}

void TestConnect::testConnectDataItem_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("parent");
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("value");
    QTest::addColumn<int>("id");
    QTest::addColumn<int>("action");  // 1 : append, 2 : insert, 3 : remove, 4 : append granChild

    QTest::newRow("Append Key1")  << 0 << -1 << "Key 1" << "Value 1" << lastid++  << 1;
    QTest::newRow("Append Key2")  << 1 << -1 << "Key 2" << "Value 2" << lastid++  << 1;
    QTest::newRow("Append Key3")  << 2 << -1 << "Key 3" << "Value 3" << lastid++  << 1;
    QTest::newRow("Append Key4")  << 3 << -1 << "Key 4" << "Value 4" << lastid++  << 1;

    QTest::newRow("Insert Key1Key2") << 1 << -1 << "Key 1 Key2" << "Value 1 Value2" << lastid++ << 2;
    QTest::newRow("Insert Key3Key4") << 4 << -1 << "Key 3 Key4" << "Value 3 Value4" << lastid++ << 2;

    QTest::newRow("Remove Key1Key2") << 1 << -1 << "Key 2"  << "Value 2"  << -1 << 3;
    QTest::newRow("Remove Key3Key4") << 3 << -1 << "Key 4"  << "Value 4"  << -1 << 3;

    QTest::newRow("Add Key11") << 0 << 0 << "Key 11" << "Value 11" << lastid++  << 4;
    QTest::newRow("Add Key12") << 1 << 0 << "Key 12" << "Value 12" << lastid++  << 4;
    QTest::newRow("Add Key13") << 2 << 0 << "Key 13" << "Value 13" << lastid++  << 4;
    QTest::newRow("Add Key21") << 0 << 1 << "Key 21" << "Value 21" << lastid++  << 4;
    QTest::newRow("Add Key22") << 1 << 1 << "Key 22" << "Value 22" << lastid++  << 4;

    // remove child and its granChildren
    QTest::newRow("Remove Key2") << 1 << -1 << "Key 3"  << "Value 3"  << -1 << 3;
}

void TestConnect::testConnectDataItem()
{
    QFETCH(int, row);
    QFETCH(int, parent);
    QFETCH(QString, key);
    QFETCH(QString, value);
    QFETCH(int, id);
    QFETCH(int, action);  // 1 : append, 2 : insert, 3 : remove, 4 : append granChild

    QList<QVariant> data;
    data << key << value;
    if (action==1) { // append
        ConnectDataItem *item = new ConnectDataItem(data, id, rootItem);
        rootItem->appendChild( item );

        QCOMPARE(item->id(), id);
        QCOMPARE(item->row(), row);
        QCOMPARE(item->columnCount(), rootItem->columnCount());
        QCOMPARE(rootItem->childCount(), id);

        for (int i=0; i<rootItem->columnCount(); ++i) {
            QCOMPARE(rootItem->child(row)->data(i).toString(), item->data(i).toString());
            QVERIFY(item->parentItem());
            QCOMPARE(item->parentItem(), rootItem);
        }
    } else if (action==2) { // insert
        ConnectDataItem *item = new ConnectDataItem(data, id, rootItem);
        rootItem->insertChild( row, item );

        QCOMPARE(item->id(), id);
        QCOMPARE(item->row(), row);
        QCOMPARE(item->columnCount(), rootItem->columnCount());
        QCOMPARE(rootItem->childCount(), id);

        for (int i=0; i<rootItem->columnCount(); ++i) {
            QCOMPARE(rootItem->child(row)->data(i).toString(), item->data(i).toString());
            QVERIFY(item->parentItem());
            QCOMPARE(item->parentItem(), rootItem);
        }
    } else if (action==3) { // remove
              int children = rootItem->childCount();
              rootItem->removeChildren(row, 1);
              QCOMPARE(rootItem->childCount(), children-1);
              QVERIFY( rootItem->child(row)->data(0).toString().compare( key )==0 );
              QVERIFY( rootItem->child(row)->data(1).toString().compare( value )==0 );
    } else if (action==4) { // append grandchild
               int children = rootItem->child(parent)->childCount();
               ConnectDataItem *item = new ConnectDataItem(data, id, rootItem->child(parent));
               rootItem->child(parent)->appendChild( item );
               QCOMPARE(rootItem->child(parent)->childCount(), children+1);
               QCOMPARE(rootItem->child(parent)->child(row)->columnCount(), rootItem->columnCount());
               for (int i=0; i<rootItem->child(parent)->columnCount(); ++i) {
                   QCOMPARE(rootItem->child(parent)->child(row)->data(i).toString(), item->data(i).toString());
                   QVERIFY(item->parentItem());
                   QCOMPARE(item->parentItem(), rootItem->child(parent));
               }
    }
}

void TestConnect::cleanupTestCase()
{
    rootData.clear();
    if (rootItem)
       delete rootItem;
}

QTEST_MAIN(TestConnect)
