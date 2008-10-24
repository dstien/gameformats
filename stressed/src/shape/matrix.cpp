// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008 Daniel Stien <daniel@stien.org>
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

#include <QGLWidget>

#include "matrix.h"

Matrix::Matrix(float x, float y, float z)
{
  reset(x, y, z);
}

void Matrix::reset(float x, float y, float z)
{
  matrix[ 0] = 1.0f; matrix[ 1] = 0.0f; matrix[ 2] = 0.0f; matrix[ 3] = 0.0f;
  matrix[ 4] = 0.0f; matrix[ 5] = 1.0f; matrix[ 6] = 0.0f; matrix[ 7] = 0.0f;
  matrix[ 8] = 0.0f; matrix[ 9] = 0.0f; matrix[10] = 1.0f; matrix[11] = 0.0f;
  matrix[12] = x;    matrix[13] = y;    matrix[14] = z;    matrix[15] = 1.0f;
}

Matrix Matrix::transpose() const
{

  Matrix m;
  m.matrix[ 0] = matrix[ 0]; m.matrix[ 1] = matrix[ 4]; m.matrix[ 2] = matrix[ 8]; m.matrix[ 3] = 0.0f;
  m.matrix[ 4] = matrix[ 1]; m.matrix[ 5] = matrix[ 5]; m.matrix[ 6] = matrix[ 9]; m.matrix[ 7] = 0.0f;
  m.matrix[ 8] = matrix[ 2]; m.matrix[ 9] = matrix[ 6]; m.matrix[10] = matrix[10]; m.matrix[11] = 0.0f;
  m.matrix[12] = 0.0f;       m.matrix[13] = 0.0f;       m.matrix[14] = 0.0f;       m.matrix[15] = 1.0f;

  return m;
}

void Matrix::move(float distance, Axis axis)
{
  float x, y, z;
  getAxis(axis, x, y, z);

  glPushMatrix();
    loadMatrix();
    glTranslatef(distance * x, distance * y, distance * z);
    getMatrix();
  glPopMatrix();
}

void Matrix::rotate(float angle, Axis axis)
{
  float x, y, z;
  getAxis(axis, x, y, z);

  glPushMatrix();
    loadMatrix();
    glRotatef(angle, x, y, z);
    getMatrix();
  glPopMatrix();
}

void Matrix::multMatrix() const
{
  glMultMatrixf(matrix);
}

inline void Matrix::loadMatrix() const
{
  glLoadMatrixf(matrix);
}

inline void Matrix::getMatrix()
{
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
}

inline void Matrix::getAxis(Axis axis, float &x, float &y, float &z) const
{
  x = (axis == Matrix::AXIS_X ? 1.0f : 0.0f);
  y = (axis == Matrix::AXIS_Y ? 1.0f : 0.0f);
  z = (axis == Matrix::AXIS_Z ? 1.0f : 0.0f);
}
