// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2012 Daniel Stien <daniel@stien.org>
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

#ifndef SHAPEMODEL_H
#define SHAPEMODEL_H

#include <QAbstractTableModel>

#include "types.h"

class QItemSelectionModel;

#define PRIM_TYPE_PARTICLE 1
#define PRIM_TYPE_LINE     2
#define PRIM_TYPE_SPHERE   11
#define PRIM_TYPE_WHEEL    12

#define PRIM_FLAG_TWOSIDED (1 << 0)
#define PRIM_FLAG_ZBIAS    (1 << 1)

#define PRIM_CULL_POS       0xFFFE0000
#define PRIM_CULL_NEG       0x0001FFFC
#define PRIM_CULL_POS_SHIFT 17
#define PRIM_CULL_NEG_SHIFT 2
#define PRIM_CULL_POS_FLAG  (1 << 1)
#define PRIM_CULL_NEG_FLAG  (1 << 0)

#define PRIM_CULL_POS_GET(c)    ((c & PRIM_CULL_POS) >> PRIM_CULL_POS_SHIFT)
#define PRIM_CULL_NEG_GET(c)    ((c & PRIM_CULL_NEG) >> PRIM_CULL_NEG_SHIFT)
#define PRIM_CULL_POS_SET(c, v) (c = (v << PRIM_CULL_POS_SHIFT) | (c & ~PRIM_CULL_POS))
#define PRIM_CULL_NEG_SET(c, v) (c = (v << PRIM_CULL_NEG_SHIFT) | (c & ~PRIM_CULL_NEG))

#define PRIM_CULL_3BITS  0x001C
#define PRIM_CULL_5BITS  0x003E
#define PRIM_CULL_7BITS  0x007F
#define PRIM_CULL_9BITS  0x40FF
#define PRIM_CULL_11BITS 0x61FF
#define PRIM_CULL_13BITS 0x73FF
#define PRIM_CULL_15BITS 0x7FFF
#define PRIM_CULL_ROTATE(v, r) (((v << r) | (v >> (15 - r))) & PRIM_CULL_15BITS)

class ShapeModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  ShapeModel(QObject* parent = 0);
  ShapeModel(const ShapeModel& mod, QObject* parent = 0);

  Qt::ItemFlags     flags(const QModelIndex& index) const;
  QVariant          data(const QModelIndex& index, int role) const;
  bool              setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
  QVariant          headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  bool              insertRows(int position, int rows, const QModelIndex& index = QModelIndex());
  bool              removeRows(int position, int rows, const QModelIndex& index = QModelIndex());
  void              removeRows(const QModelIndexList& rows);
  void              moveRows(QItemSelectionModel* selectionModel, int direction);
  void              duplicateRow(int position);
  void              mirrorXRow(int position);
  void              computeCullRows(const QModelIndexList& rows);
  void              computeCull();
  void              computeCull(Primitive& primitive);

  int               rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const    { return m_primitives.size(); }
  int               columnCount(const QModelIndex& /*parent*/ = QModelIndex()) const { return 7; }

  void              setShape(PrimitivesList& primitives);
  PrimitivesList*   primitivesList()                                                 { return &m_primitives; }
  Vertex*           boundBox();
  void              replaceVertices(const Vertex& curVert, const Vertex& newVert);

  bool              setNumPaintJobs(int& num);
  int               numPaintJobs() const                                             { return m_numPaintJobs; }
  void              replaceMaterials(quint8 paintJob, quint8 curMaterial, quint8 newMaterial);
  void              movePaintJobs(QItemSelectionModel* selectionModel, int direction);

  static const QStringList TYPES;

  static const int  ROWS_MAX = 256;

signals:
  void              paintJobMoved(int oldPosition, int newPosition);

public slots:
  void              isModified();

private:
  PrimitivesList    m_primitives;
  Vertex            m_bound[8];
  int               m_numPaintJobs;

  static const int  TYPE_MIN = 1;
  static const int  TYPE_MAX = 12;

  static const int  PAINTJOBS_MIN = 1;
  static const int  PAINTJOBS_MAX = 127;
};

#endif
