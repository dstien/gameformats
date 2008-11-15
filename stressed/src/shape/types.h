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

#ifndef TYPES_H
#define TYPES_H

#include <QList>

typedef struct {
  qint16 x;
  qint16 y;
  qint16 z;
} Vertex;

typedef QList<Vertex> VerticesList;

class VerticesModel;

typedef QList<quint8> MaterialsList;

class MaterialsModel;

typedef struct {
  quint8            type;
  bool              twoSided;
  bool              zBias;
  VerticesModel*    verticesModel;
  MaterialsModel*   materialsModel;
  quint32           cull1;
  quint32           cull2;
} Primitive;


typedef QList<Primitive> PrimitivesList;

#endif
