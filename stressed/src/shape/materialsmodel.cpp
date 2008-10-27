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

#include "materialdelegate.h"
#include "materialsmodel.h"
#include "shapemodel.h"

const int MaterialsModel::VAL_MIN;
const int MaterialsModel::VAL_MAX;

MaterialsModel::MaterialsModel(const MaterialsList& materials, QObject* parent)
: QAbstractTableModel(parent),
  m_materials(materials)
{
  setup();
}

MaterialsModel::MaterialsModel(quint8 material, QObject* parent)
: QAbstractTableModel(parent)
{
  setup();
  m_materials.append(material);
}

MaterialsModel::MaterialsModel(int num, QObject* parent)
: QAbstractTableModel(parent)
{
  setup();
  resize(num);
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

    case Qt::DecorationRole:
      return MaterialDelegate::getIcon(m_materials[row]);

    case Qt::DisplayRole:
    case Qt::EditRole:
      return m_materials[row];

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

  if (!success || (result == m_materials[index.row()])) {
    return false;
  }

  m_materials[index.row()] = result;
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

bool MaterialsModel::insertRows(int position, int rows, const QModelIndex& index)
{
  beginInsertRows(index, position, position + rows - 1);

  for (int row = 0; row < rows; row++) {
    // Expand by copying last material if available.
    m_materials.insert(position, (m_materials.isEmpty() ? 0 : m_materials.last()));
  }

  endInsertRows();

  return true;
}

bool MaterialsModel::removeRows(int position, int rows, const QModelIndex& index)
{
  beginRemoveRows(index, position, position + rows - 1);
  
  for (int row = 0; row < rows; row++) {
    m_materials.removeAt(position);
  }

  endRemoveRows();

  return true;
}

void MaterialsModel::resize(int num)
{
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

void MaterialsModel::setup()
{
  connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
      qobject_cast<ShapeModel*>(QObject::parent()), SLOT(isModified()));
}
