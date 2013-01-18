// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2013 Daniel Stien <daniel@stien.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef MATRIX_H
#define MATRIX_H

class Matrix
{
public:
  enum Axis {
    AXIS_X,
    AXIS_Y,
    AXIS_Z
  };

  Matrix(float x = 0.0f, float y = 0.0f, float z = 0.0f);

  void              reset(float x = 0.0f, float y = 0.0f, float z = 0.0f);

  void              move(float distance, Axis axis);
  void              rotate(float angle, Axis axis);

  Matrix            transpose() const;

  void              multMatrix() const;

private:
  inline void       loadMatrix() const;
  inline void       getMatrix();
  inline void       getAxis(Axis axis, float &x, float &y, float &z) const;

  float             matrix[16];
};

#endif
