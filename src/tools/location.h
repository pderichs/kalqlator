// KalQlator - location.h
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

#include <cmath>
#include <set>
#include <vector>
#include <ostream>

/**
 * Simple Location class - combines x and y coordinates and
 * provides some functionality in dealing with these coordinates.
 */
class Location {
private:
  long x_, y_;

public:
  /**
   * Default constructor - initializes x and y to zero.
   */
  Location() : x_(0), y_(0) {
  }

  /**
   * Provides a way to construct a Location with explicit
   * coordinates.
   *
   * @param x Value of x coordinate
   * @param y Value of y coordinate
   */
  explicit Location(long x, long y) : x_(x), y_(y) {
  }

  /**
   * Accessor to x coordinate
   *
   * @return Current value of x coordinate
   */
  [[nodiscard]] long x() const { return x_; }

  /**
   * Accessor to y coordinate
   *
   * @return Current value of y coordinate
   */
  [[nodiscard]] long y() const { return y_; }

  /**
   * Accessor to x coordinate
   *
   * @return Current value of x coordinate
   */
  [[nodiscard]] long column() const { return x_; }

  /**
   * Accessor to y coordinate
   *
   * @return Current value of y coordinate
   */
  [[nodiscard]] long row() const { return y_; }

  /**
   * Moves the coordinate one step to the left
   */
  void moveLeft() { x_--; }

  /**
   * Moves the coordinate one step to the right
   */
  void moveRight() { x_++; }

  /**
   * Moves the coordinate one step down
   */
  void moveDown() { y_++; }

  /**
   * Moves the coordinate one step up
   */
  void moveUp() { y_--; }

  /**
   * Moves the coordinate amount steps up
   * @param amount Amount steps to move coordinate up
   */
  void moveUp(int amount) { y_ -= amount; }

  /**
   * Moves the coordinate amount steps down
   * @param amount Amount of steps to move coordinate down
   */
  void moveDown(int amount) { y_ += amount; }

  /**
   * Equality operator
   *
   * @param other Comparison Location
   * @return true if both locations contain the same coordinate values, false otherwise
   */
  bool operator==(const Location &other) const {
    return x_ == other.x_ && y_ == other.y_;
  }

  /**
   * Inequality operator
   *
   * @param other Comparison Location
   * @return true if both locations do not contain the same coordinate values, false otherwise
   */
  bool operator!=(const Location &other) const {
    return x_ != other.x_ || y_ != other.y_;
  }

  /**
   * Adds the coordinate values of another Location instance to the coordinate
   * values of this instance.
   *
   * @param other Location which contains values to be added
   * @return New location which contains the result of the addition
   */
  Location operator+(const Location &other) const {
    return Location(x_ + other.x_, y_ + other.y_);
  };

  /**
   * Smaller than operator to be used for e.g. maps.
   * @param other Location instance to be compared
   * @return true if this instance is smaller than other, false otherwise
   */
  bool operator<(const Location &other) const {
    return (x_ < other.x_) || (x_ == other.x_ && y_ < other.y_);
  }

  /**
   * Calculates the manhattan distance to another Location.
   * @param other Calculate manhattan distance to this location
   * @return The manhattan distance from this Location to other
   */
  [[nodiscard]] long distance_to(const Location &other) const {
    return std::abs(x_ - other.x()) + std::abs(y_ - other.y());
  }

  /**
   * Provides a nice to read stream output for this location
   * @param os Stream to represent this instance of Location
   * @param m Location to be displayed
   * @return the stream so chaining outputs is possible
   */
  friend std::ostream &operator<<(std::ostream &os, Location const &m) {
    return os << "(col: " << m.x() << ", row: " << m.y() << ")";
  }
};

typedef std::vector<Location> LocationVector;
typedef std::set<Location> LocationSet;

