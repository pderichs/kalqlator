// KalQlator - test_location.cpp
// Copyright (C) 2026  pderichs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <QtTest/QtTest>
#include "../src/tools/location.h"

class LocationTests : public QObject
{
    Q_OBJECT

private slots:
    static void initialLocationIsAtX0Y0()
    {
        const Location location;
        QCOMPARE(location.x(), 0);
        QCOMPARE(location.y(), 0);
    }

    static void locationAcceptsCoordinates()
    {
        const Location location(10, 42);
        QCOMPARE(location.x(), 10);
        QCOMPARE(location.y(), 42);
    }

    static void locationLowerThanRespectsX()
    {
        const Location location1(9, 42);
        const Location location2(10, 42);
        QVERIFY(location1 < location2);
    }

    static void locationLowerThanRespectsY()
    {
        const Location location1(10, 42);
        const Location location2(10, 43);
        QVERIFY(location1 < location2);
    }

    static void locationLowerThanEquality()
    {
        const Location location1(10, 42);
        const Location location2(10, 42);
        QVERIFY(!(location1 < location2));
    }

    static void locationLowerThanMismatchX()
    {
        const Location location1(11, 42);
        const Location location2(10, 42);
        QVERIFY(!(location1 < location2));
    }

    static void locationLowerThanMismatchY()
    {
        const Location location1(10, 43);
        const Location location2(10, 42);
        QVERIFY(!(location1 < location2));
    }

    static void locationEqualityNotEqualX()
    {
        const Location location1(10, 42);
        const Location location2(11, 42);
        QVERIFY(location1 != location2);
    }

    static void locationEqualityNotEqualY()
    {
        const Location location1(10, 43);
        const Location location2(10, 42);
        QVERIFY(location1 != location2);
    }

    static void locationEqualityEqual()
    {
        const Location location1(10, 42);
        const Location location2(10, 42);
        QVERIFY(location1 == location2);
    }

    static void locationInEqualityNotEqualX()
    {
        const Location location1(10, 42);
        const Location location2(11, 42);
        QVERIFY(location1 != location2);
    }

    static void locationInEqualityNotEqualY()
    {
        const Location location1(10, 43);
        const Location location2(10, 42);
        QVERIFY(location1 != location2);
    }

    static void locationInEqualityEqual()
    {
        const Location location1(10, 42);
        const Location location2(10, 42);
        QVERIFY(location1 == location2);
    }

    static void addition()
    {
        const Location location1(10, 42);
        const Location location2(1, 2);
        const auto result = location1 + location2;
        QCOMPARE(result, Location(11, 44));
    }
};

QTEST_MAIN(LocationTests)
#include "test_location.moc"