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

#ifndef RESOURCESMODEL_H
#define RESOURCESMODEL_H

#include <QAbstractListModel>

class Resource;

typedef QList<Resource*> ResourcesList;

class ResourcesModel : public QAbstractListModel
{
  Q_OBJECT

public:
  ResourcesModel(QObject* parent = 0);
  ResourcesModel(const ResourcesList& resources, QObject* parent = 0);

  QVariant          data(const QModelIndex& index, int role) const;

  void              append(Resource* resource);
  bool              removeRows(int position, int rows, const QModelIndex& index = QModelIndex());
  void              clear();

  int               rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const    { return m_resources.size(); }

  Resource*         at(int index) const                                              { return m_resources[index]; }
  Resource*         at(const QModelIndex& index) const;

private:
  ResourcesList     m_resources;
};

#endif
