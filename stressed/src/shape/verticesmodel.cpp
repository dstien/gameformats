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

#include "verticesmodel.h"

const int VerticesModel::VAL_MIN;
const int VerticesModel::VAL_MAX;

VerticesModel::VerticesModel(const VerticesList& vertices, QObject* parent)
: QAbstractTableModel(parent),
  vertices(vertices)
{
}

Qt::ItemFlags VerticesModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return 0;
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant VerticesModel::data(const QModelIndex& index, int role) const
{
  int row = index.row(), col = index.column();

  if (!index.isValid() ||
      row >= rowCount() || col >= columnCount()) {
    return QVariant();
  }

  switch (role) {
    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    case Qt::DisplayRole:
    case Qt::EditRole:
      if (col == 0) {
        return vertices[row].x;
      }
      else if (col == 1) {
        return vertices[row].y;
      }
      else if (col == 2) {
        return vertices[row].z;
      }
    default:
      return QVariant();
  }
}

bool VerticesModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
  int row = index.row(), col = index.column();

  if (!index.isValid() || role != Qt::EditRole ||
      row >= rowCount() || col >= columnCount()) {
    return false;
  }

  bool success;
  qint16 result = qBound(VAL_MIN, value.toInt(&success), VAL_MAX);

  if (!success) {
    return false;
  }

  switch (col) {
    case 0:
      if (result == vertices[row].x) {
        return false;
      }
      vertices[row].x = result;
      break;
    case 1:
      if (result == vertices[row].y) {
        return false;
      }
      vertices[row].y = result;
      break;
    case 2:
      if (result == vertices[row].z) {
        return false;
      }
      vertices[row].z = result;
  }

  emit dataChanged(index, index);
  return true;
}

QVariant VerticesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return "x";
      case 1:
        return "y";
      case 2:
      default:
        return "z";
    }
  }
  else {
    return section + 1;
  }
}

bool VerticesModel::insertRows(int position, int rows, const QModelIndex& index)
{
  beginInsertRows(index, position, position + rows - 1);

  for (int row = 0; row < rows; row++) {
    Vertex vertex;
    vertex.x = 0;
    vertex.y = 0;
    vertex.z = 0;
    vertices.insert(position, vertex);
  }

  endInsertRows();
  return true;
}

bool VerticesModel::removeRows(int position, int rows, const QModelIndex& index)
{
  beginRemoveRows(index, position, position + rows - 1);
  
  for (int row = 0; row < rows; row++) {
    vertices.removeAt(position);
  }

  endRemoveRows();

  return true;
}

void VerticesModel::resize(int type)
{
  int num;
  verticesNeeded(type, num);

  if (num == rowCount()) {
    return;
  }
  else if (num < rowCount()) {
    int diff = rowCount() - num;
    removeRows(rowCount() - diff, diff);
  }
  else {
    insertRows(rowCount(), num - rowCount());
  }
}

bool VerticesModel::verticesNeeded(int type, int& num)
{
  if (type < 1 || type > 12) {
    num = 0;
    return false;
  }
  else if (type == 11) {
    num = 2;
  }
  else if (type == 12) {
    num = 6;
  }
  else {
    num = type;
  }

  return true;
}
