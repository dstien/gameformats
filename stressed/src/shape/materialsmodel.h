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

#ifndef MATERIALSMODEL_H
#define MATERIALSMODEL_H

#include <QAbstractTableModel>

#include "types.h"

class MaterialsModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  MaterialsModel(const MaterialsList& materials, QObject* parent = 0);
  MaterialsModel(quint8 material, QObject* parent = 0);
  MaterialsModel(int num, QObject* parent = 0);

  Qt::ItemFlags     flags(const QModelIndex& index) const;
  QVariant          data(const QModelIndex& index, int role) const;
  bool              setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
  QVariant          headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  bool              insertRows(int position, int rows, const QModelIndex& index = QModelIndex());
  bool              removeRows(int position, int rows, const QModelIndex& index = QModelIndex());

  int               rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const    { return m_materials.size(); }
  int               columnCount(const QModelIndex& /*parent*/ = QModelIndex()) const { return 1; }

  void              resize(int num);
  MaterialsList*    materialsList()                                                  { return &m_materials; }

  static const int  VAL_MIN = 0;
  static const int  VAL_MAX = 255;

private:
  void              setup();

  MaterialsList     m_materials;
};

#endif
