// KalQlator - test_flagscope.cpp
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

#include "../src/tools/FlagScope.h"
#include <QtTest/QtTest>

class FlagScopeTests : public QObject {
  Q_OBJECT

private slots:
  static void setsFlagOnConstruction() {
    bool flag = false;
    {
      FlagScope scope(&flag);
      QVERIFY(flag);
    }
  }

  static void resetsFlagOnDestruction() {
    bool flag = false;
    {
      FlagScope scope(&flag);
      QVERIFY(flag);
    }
    QVERIFY(!flag);
  }

  static void handlesMultipleFlagScopes() {
    bool flag1 = false;
    bool flag2 = false;
    {
      FlagScope scope1(&flag1);
      QVERIFY(flag1);
      QVERIFY(!flag2);
      {
        FlagScope scope2(&flag2);
        QVERIFY(flag1);
        QVERIFY(flag2);
      }
      QVERIFY(flag1);
      QVERIFY(!flag2);
    }
    QVERIFY(!flag1);
    QVERIFY(!flag2);
  }
};

QTEST_MAIN(FlagScopeTests)
#include "test_flagscope.moc"