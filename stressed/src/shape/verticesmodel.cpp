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

VerticesModel::VerticesModel(const VerticesList& vertices, QObject* parent)
: QAbstractTableModel(parent),
  vertices(vertices)
{
}

Qt::ItemFlags VerticesModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  return QAbstractItemModel::flags(index);
}

QVariant VerticesModel::data(const QModelIndex& index, int role) const
{
  int row = index.row(), col = index.column();

  if (!index.isValid() ||
      row < 0 || row >= rowCount() ||
      col < 0 || col >= columnCount()) {
    return QVariant();
  }

  switch (role) {
    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    case Qt::DisplayRole:
      if (col == 0) {
        return QString("%1").arg(vertices[row].x);
      }
      else if (col == 1) {
        return QString("%1").arg(vertices[row].y);
      }
      else if (col == 2) {
        return QString("%1").arg(vertices[row].z);
      }
    default:
      return QVariant();
  }
}

QVariant VerticesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
        return QString("x");
      case 1:
        return QString("y");
      case 2:
      default:
        return QString("z");
    }
  }
  else {
    return QString("%1").arg(section + 1);
  }
}
