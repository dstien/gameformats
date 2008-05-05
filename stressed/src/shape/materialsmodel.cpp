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

const int MaterialsModel::VAL_MIN;
const int MaterialsModel::VAL_MAX;

MaterialsModel::MaterialsModel(const MaterialsList& materials, QObject* parent)
: QAbstractTableModel(parent),
  materials(materials)
{
}

Qt::ItemFlags MaterialsModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return 0;
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant MaterialsModel::data(const QModelIndex& index, int role) const
{
  int row = index.row(), col = index.column();

  if (!index.isValid() || row >= rowCount() || col != 0) {
    return QVariant();
  }

  switch (role) {
    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    case Qt::DisplayRole:
    case Qt::EditRole:
      return materials[row];
    default:
      return QVariant();
  }
}

bool MaterialsModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
  int row = index.row();

  if (!index.isValid() || role != Qt::EditRole ||
      row >= rowCount() || index.column() != 0) {
    return false;
  }

  bool success;
  quint8 result = qBound(VAL_MIN, value.toInt(&success), VAL_MAX);

  if (!success || (result == materials[index.row()])) {
    return false;
  }

  materials[index.row()] = result;
  emit dataChanged(index, index);
  return true;
}

QVariant MaterialsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (orientation == Qt::Horizontal) {
    return "Material";
  }
  else {
    return section + 1;
  }
}
