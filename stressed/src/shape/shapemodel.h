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

#ifndef SHAPEMODEL_H
#define SHAPEMODEL_H

#include <QAbstractTableModel>

#include "verticesmodel.h"

class MaterialsModel;
class QItemSelectionModel;

typedef struct {
  quint8            type;
  quint8            depthIndex;
  VerticesModel*    verticesModel;
  MaterialsModel*   materialsModel;
  quint32           unknown1;
  quint32           unknown2;
} Primitive;

typedef QList<Primitive> PrimitivesList;

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

  int               rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const    { return m_primitives.size(); }
  int               columnCount(const QModelIndex& /*parent*/ = QModelIndex()) const { return 4; }

  void              setShape(PrimitivesList& primitives);
  PrimitivesList*   primitivesList()                                                 { return &m_primitives; }
  Vertex*           boundBox();

  bool              setNumPaintJobs(int& num);
  int               numPaintJobs() const                                             { return m_numPaintJobs; }
  void              replaceMaterials(quint8 paintJob, quint8 curMaterial, quint8 newMaterial);

  static const QStringList TYPES;

  static const int  ROWS_MAX = 256;

public slots:
  void              isModified();

private:
  PrimitivesList    m_primitives;
  Vertex            m_bound[8];
  int               m_numPaintJobs;

  static const int  TYPE_MIN = 1;
  static const int  TYPE_MAX = 12;

  static const int  DEPTH_MIN = 0;
  static const int  DEPTH_MAX = 255;

  static const int  PAINTJOBS_MIN = 1;
  static const int  PAINTJOBS_MAX = 127;
};

#endif
