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
class QItemSelectionModel;

typedef QList<Resource*> ResourcesList;

class ResourcesModel : public QAbstractListModel
{
  Q_OBJECT

public:
  ResourcesModel(QObject* parent = 0);
  ResourcesModel(const ResourcesList& resources, QObject* parent = 0);

  Qt::ItemFlags     flags(const QModelIndex& index) const;
  QVariant          data(const QModelIndex& index, int role) const;
  bool              setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  void              append(Resource* resource);
  bool              removeRows(int position, int rows, const QModelIndex& index = QModelIndex());
  void              removeRows(const QModelIndexList& rows);
  void              moveRows(QItemSelectionModel* selectionModel, int direction);
  void              duplicateRow(int position);
  void              clear();

  void              sort(int column = 0, Qt::SortOrder order = Qt::AscendingOrder);
  int               rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const    { return m_resources.size(); }

  Resource*         at(int index) const                                              { return m_resources[index]; }
  Resource*         at(const QModelIndex& index) const;

  static const int  ROWS_MAX = 65536;

private:
  ResourcesList     m_resources;
};

#endif
