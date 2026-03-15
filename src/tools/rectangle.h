// KalQlator - rectangle.h
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

#pragma once

#include "location.h"
#include <stdexcept>

class Rectangle {
private:
  Location topLeft_;
  Location bottomRight_;

  LocationSet cachedLocations_;

public:
  Rectangle(const Location &topLeft, const Location &bottomRight)
      : topLeft_(topLeft), bottomRight_(bottomRight) {
    if (!(topLeft_ < bottomRight_)) {
      throw std::runtime_error("Invalid rectangle");
    }
  }

  [[nodiscard]] bool contains(const Location &location) const {
    if (location.x() < left()) {
      return false;
    }

    if (location.x() > right()) {
      return false;
    }

    if (location.y() < top()) {
      return false;
    }

    if (location.y() > bottom()) {
      return false;
    }

    return true;
  }

  [[nodiscard]] int left() const { return topLeft_.x(); }

  [[nodiscard]] int right() const { return bottomRight_.x(); }

  [[nodiscard]] int top() const { return topLeft_.y(); }

  [[nodiscard]] int bottom() const { return bottomRight_.y(); }

  [[nodiscard]] int width() const { return right() - left(); }

  [[nodiscard]] int height() const { return bottom() - top(); }

  void extend_all_directions(int amount) {
    if (amount <= 0) {
      return;
    }

    topLeft_ = Location(topLeft_.x() - amount, topLeft_.y() - amount);
    bottomRight_ =
        Location(bottomRight_.x() + amount, bottomRight_.y() + amount);
    cachedLocations_.clear();
  }

  void extend_down() {
    bottomRight_.moveDown();
    cachedLocations_.clear();
  }

  void extend_up() {
    topLeft_.moveUp();
    cachedLocations_.clear();
  }

  void extend_left() {
    topLeft_.moveLeft();
    cachedLocations_.clear();
  }

  void extend_right() {
    bottomRight_.moveRight();
    cachedLocations_.clear();
  }

  LocationSet all_locations() {
    if (!cachedLocations_.empty()) {
      return cachedLocations_;
    }

    LocationSet result;

    for (long r = top(); r <= bottom(); r++) {
      for (long c = left(); c <= right(); c++) {
        result.insert(Location(c, r));
      }
    }

    cachedLocations_ = result;

    return result;
  }
};
