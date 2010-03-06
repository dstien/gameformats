// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2010 Daniel Stien <daniel@stien.org>
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

#include <cmath>

#include "vector3.h"

Vector3::Vector3(float x, float y, float z)
: x(x), y(y), z(z)
{
}

Vector3::Vector3(const Vertex& vert)
: x(vert.x), y(vert.y), z(vert.z)
{
}

Vector3::Vector3(const VertexF& vert)
: x(vert.x), y(vert.y), z(vert.z)
{
}

float Vector3::magnitude() const
{
  return sqrt(x * x + y * y + z * z);
}

Vector3& Vector3::normalize()
{
  return *this /= magnitude();
}

Vector3 Vector3::crossProduct(const Vector3& vec) const
{
  return Vector3(
      y * vec.z - z * vec.y,
      z * vec.x - x * vec.z,
      x * vec.y - y * vec.x);
}

float Vector3::dotProduct(const Vector3& vec) const
{
  return x * vec.x + y * vec.y + z * vec.z;
}

float Vector3::angle(const Vector3& vec) const
{
  return acos(dotProduct(vec));
}

Vector3 Vector3::operator-(const Vector3& vec) const
{
  return Vector3(x - vec.x, y - vec.y, z - vec.z);
}

Vector3& Vector3::operator/=(const float num)
{
  x /= num;
  y /= num;
  z /= num;

  return *this;
}
