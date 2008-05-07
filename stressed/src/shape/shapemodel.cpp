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

#include "materialsmodel.h"
#include "shapemodel.h"
#include "verticesmodel.h"

const int ShapeModel::DEPTH_MIN;
const int ShapeModel::DEPTH_MAX;

ShapeModel::ShapeModel(QObject* parent)
: QAbstractTableModel(parent)
{
}

Qt::ItemFlags ShapeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return 0;
  }

  if (index.column() > 0) {
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  return QAbstractItemModel::flags(index);
}

QVariant ShapeModel::data(const QModelIndex& index, int role) const
{
  int row = index.row(), col = index.column();

  if (!index.isValid() ||
      row >= rowCount() ||
      col >= columnCount()) {
    return QVariant();
  }

  switch (role) {
    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    case Qt::FontRole:
      if (col > 1) {
        return "Monospace";
      }
      else {
        return QVariant();
      }
    case Qt::DisplayRole:
    case Qt::EditRole:
      if (col == 0) {
        return primitives[row].type;
      }
      else if (col == 1) {
        return primitives[row].depthIndex;
      }
      else if (col == 2) {
        return QString("%1").arg(primitives[row].unknown1, 8, 16, QChar('0')).toUpper();
      }
      else if (col == 3) {
        return QString("%1").arg(primitives[row].unknown2, 8, 16, QChar('0')).toUpper();
      }
    default:
      return QVariant();
  }
}

bool ShapeModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
  int row = index.row(), col = index.column();

  if (!index.isValid() || role != Qt::EditRole ||
      row >= rowCount() || col >= columnCount() || col < 1) {
    return false;
  }

  if (col == 1) {
    bool success;
    quint8 result = qBound(DEPTH_MIN, value.toInt(&success), DEPTH_MAX);

    if (!success || (result == primitives[row].depthIndex)) {
      return false;
    }

    primitives[row].depthIndex = result;
  }
  else {
    bool success;
    quint32 result = value.toString().toUInt(&success, 16);

    if (!success) {
      return false;
    }

    if (col == 2) {
      if (result == primitives[row].unknown1) {
        return false;
      }
      primitives[row].unknown1 = result;
    }
    else if (col == 3) {
      if (result == primitives[row].unknown2) {
        return false;
      }
      primitives[row].unknown2 = result;
    }
    else {
      return false;
    }
  }

  emit dataChanged(index, index);
  return true;
}

QVariant ShapeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return "Type";
      case 1:
        return "Depth";
      case 2:
        return "Unknown 1";
      case 3:
      default:
        return "Unknown 2";
    }
  }
  else {
    return section + 1;
  }
}

bool ShapeModel::removeRows(int position, int rows, const QModelIndex& /*index*/)
{
  beginRemoveRows(QModelIndex(), position, position + rows - 1);

  for (int row = 0; row < rows; row++) {
    primitives.removeAt(position);
  }

  endRemoveRows();

  return true;
}

void ShapeModel::removeRows(const QModelIndexList& rows)
{
  // Using persistent indices since row removal will invalidate current selection.
  QList<QPersistentModelIndex> persistentRows;
  foreach (QModelIndex row, rows) {
    persistentRows.append(QPersistentModelIndex(row));
  }

  foreach (QPersistentModelIndex row, persistentRows) {
    removeRows(row.row(), 1);
  }
}

void ShapeModel::setShape(PrimitivesList& primitives)
{
  if (!primitives.isEmpty()) {
    this->primitives = primitives;

    beginInsertRows(QModelIndex(), 0, primitives.size() - 1);

    foreach (Primitive primitive, primitives) {
      primitive.verticesModel->setParent(this);
      primitive.materialsModel->setParent(this);
    }

    endInsertRows();
  }
}

Vertex* ShapeModel::boundBox()
{
  if (primitives.isEmpty()) {
    for (int i = 0; i < 8; i++) {
      bound[i].x = 0; bound[i].y = 0; bound[i].z = 0;
    }
  }
  else {
    Vertex first = primitives.at(0).verticesModel->verticesList()->at(0);
    qint16 minX = first.x, minY = first.y, minZ = first.z, maxX = minX, maxY = minY, maxZ = minZ;

    foreach (Primitive primitive, primitives) {
      foreach (Vertex vertex, *(primitive.verticesModel->verticesList())) {
        if (vertex.x < minX) minX = vertex.x;
        else if (vertex.x > maxX) maxX = vertex.x;
        if (vertex.y < minY) minY = vertex.y;
        else if (vertex.y > maxY) maxY = vertex.y;
        if (vertex.z < minZ) minZ = vertex.z;
        else if (vertex.z > maxZ) maxZ = vertex.z;
      }
    }

    bound[0].x = minX; bound[0].y = minY; bound[0].z = maxZ;
    bound[1].x = maxX; bound[1].y = minY; bound[1].z = maxZ;
    bound[2].x = minX; bound[2].y = minY; bound[2].z = minZ;
    bound[3].x = maxX; bound[3].y = minY; bound[3].z = minZ;
    bound[4].x = minX; bound[4].y = maxY; bound[4].z = maxZ;
    bound[5].x = maxX; bound[5].y = maxY; bound[5].z = maxZ;
    bound[6].x = minX; bound[6].y = maxY; bound[6].z = minZ;
    bound[7].x = maxX; bound[7].y = maxY; bound[7].z = minZ;
  }

  return bound;
}
