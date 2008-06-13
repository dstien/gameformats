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

#include "resource.h"
#include "resourcesmodel.h"

ResourcesModel::ResourcesModel(QObject* parent)
: QAbstractListModel(parent)
{
}

ResourcesModel::ResourcesModel(const ResourcesList& resources, QObject* parent)
: QAbstractListModel(parent),
  m_resources(resources)
{
}

QVariant ResourcesModel::data(const QModelIndex& index, int role) const
{
  int row = index.row(), col = index.column();

  if (!index.isValid() || row >= rowCount() || col != 0) {
    return QVariant();
  }

  switch (role) {
    case Qt::DisplayRole:
      return m_resources[row]->id();

    default:
      return QVariant();
  }
}

Resource* ResourcesModel::at(const QModelIndex& index) const
{
  if (!index.isValid() || index.row() >= rowCount()) {
    return NULL;
  }

  return m_resources[index.row()];
}

void ResourcesModel::append(Resource* resource)
{
  beginInsertRows(QModelIndex(), m_resources.size(), m_resources.size());
  m_resources.append(resource);
  endInsertRows();
}

bool ResourcesModel::removeRows(int position, int rows, const QModelIndex& index)
{
  beginRemoveRows(index, position, position + rows - 1);
  
  for (int row = 0; row < rows; row++) {
    m_resources.removeAt(position);
  }

  endRemoveRows();

  return true;
}

void ResourcesModel::clear()
{
  foreach(Resource* resource, m_resources) {
    delete resource;
  }

  m_resources.clear();

  reset();
}
