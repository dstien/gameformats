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

#include <QItemSelectionModel>
#include <QStringList>

#include "materialsmodel.h"
#include "shapemodel.h"
#include "verticesmodel.h"

const int ShapeModel::ROWS_MAX;

const int ShapeModel::TYPE_MIN;
const int ShapeModel::TYPE_MAX;

const int ShapeModel::DEPTH_MIN;
const int ShapeModel::DEPTH_MAX;

const int ShapeModel::PAINTJOBS_MIN;
const int ShapeModel::PAINTJOBS_MAX;

const QStringList ShapeModel::TYPES = (QStringList()
    << tr("Particle")    << tr("Line")         << tr("Polygon (3)") << tr("Polygon (4)")
    << tr("Polygon (5)") << tr("Polygon (6)")  << tr("Polygon (7)") << tr("Polygon (8)")
    << tr("Polygon (9)") << tr("Polygon (10)") << tr("Sphere")      << tr("Wheel"));

ShapeModel::ShapeModel(QObject* parent)
: QAbstractTableModel(parent)
{
  m_numPaintJobs = 1;
}

ShapeModel::ShapeModel(const ShapeModel& mod, QObject* parent)
: QAbstractTableModel(parent)
{
  if (!mod.m_primitives.empty()) {
    m_primitives = mod.m_primitives;
    beginInsertRows(QModelIndex(), 0, m_primitives.size() - 1);

    for (int i = 0; i < m_primitives.size(); i++) {
      m_primitives[i].verticesModel = new VerticesModel(*(m_primitives[i].verticesModel->verticesList()), this);
      m_primitives[i].materialsModel = new MaterialsModel(*(m_primitives[i].materialsModel->materialsList()), this);
    }

    endInsertRows();
  }

  m_numPaintJobs = mod.m_numPaintJobs;
}

Qt::ItemFlags ShapeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return 0;
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
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
      if (col == 0) {
        return TYPES[m_primitives[row].type - 1];
      }
    case Qt::EditRole:
      if (col == 0) {
        return m_primitives[row].type;
      }
      else if (col == 1) {
        return m_primitives[row].depthIndex;
      }
      else if (col == 2) {
        return QString("%1").arg(m_primitives[row].unknown1, 8, 16, QChar('0')).toUpper();
      }
      else if (col == 3) {
        return QString("%1").arg(m_primitives[row].unknown2, 8, 16, QChar('0')).toUpper();
      }
    default:
      return QVariant();
  }
}

bool ShapeModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
  int row = index.row(), col = index.column();

  if (!index.isValid() || role != Qt::EditRole ||
      row >= rowCount() || col >= columnCount()) {
    return false;
  }

  bool success;

  if (col == 0) {
    quint8 result = qBound(TYPE_MIN, value.toInt(&success), TYPE_MAX);

    if (!success || (result == m_primitives[row].type)) {
      return false;
    }

    m_primitives[row].verticesModel->resize(result);
    m_primitives[row].type = result;
  }
  else if (col == 1) {
    quint8 result = qBound(DEPTH_MIN, value.toInt(&success), DEPTH_MAX);

    if (!success || (result == m_primitives[row].depthIndex)) {
      return false;
    }

    m_primitives[row].depthIndex = result;
  }
  else {
    quint32 result = value.toString().toUInt(&success, 16);

    if (!success) {
      return false;
    }

    if (col == 2) {
      if (result == m_primitives[row].unknown1) {
        return false;
      }
      m_primitives[row].unknown1 = result;
    }
    else if (col == 3) {
      if (result == m_primitives[row].unknown2) {
        return false;
      }
      m_primitives[row].unknown2 = result;
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

bool ShapeModel::insertRows(int position, int rows, const QModelIndex& index)
{
  beginInsertRows(index, position, position + rows - 1);

  for (int row = 0; row < rows; row++) {
    Primitive primitive;
    primitive.type = 1;
    primitive.depthIndex = 0;
    primitive.verticesModel = new VerticesModel(primitive.type, this);
    primitive.materialsModel = new MaterialsModel(m_numPaintJobs, this);
    primitive.unknown1 = 0xFFFFFFFF;
    primitive.unknown2 = 0xFFFFFFFF;

    m_primitives.insert(position, primitive);
  }

  endInsertRows();
  return true;
}

bool ShapeModel::removeRows(int position, int rows, const QModelIndex& index)
{
  beginRemoveRows(index, position, position + rows - 1);

  for (int row = 0; row < rows; row++) {
    delete m_primitives[position].verticesModel;
    delete m_primitives[position].materialsModel;
    m_primitives.removeAt(position);
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

void ShapeModel::moveRows(QItemSelectionModel* selectionModel, int direction)
{
  // Using persistent indices since row removal/insertion will invalidate current selection.
  QList<QPersistentModelIndex> newPersistentRows;
  QList<QPersistentModelIndex> curPersistentRows;
  foreach (QModelIndex row, selectionModel->selectedRows()) {
    curPersistentRows.append(QPersistentModelIndex(row));
  }

  // Sort by direction to prevent overwriting.
  if (direction < 0) {
    qSort(curPersistentRows);
  }
  else {
    qSort(curPersistentRows.begin(), curPersistentRows.end(), qGreater<QPersistentModelIndex>());
  }

  QPersistentModelIndex persistentCurrent = selectionModel->currentIndex();
  QModelIndex parent, current = persistentCurrent;

  foreach (QPersistentModelIndex row, curPersistentRows) {
    int curRow = row.row();
    int newRow = qBound(0, curRow + direction, rowCount() - 1);

    if (curRow != newRow) {
      if (persistentCurrent.row() == curRow) {
        current = index(newRow < ROWS_MAX ? newRow : rowCount() - 1, 0);
      }

      Primitive primitive = m_primitives[curRow];

      beginRemoveRows(parent, curRow, curRow);
      m_primitives.removeAt(curRow);
      endRemoveRows();

      beginInsertRows(parent, newRow, newRow);
      m_primitives.insert(newRow, primitive);
      endInsertRows();
    }

    newPersistentRows.append(QPersistentModelIndex(index((newRow < ROWS_MAX ? newRow : rowCount() - 1), 0)));
  }

  selectionModel->reset();
  selectionModel->setCurrentIndex(current, QItemSelectionModel::Current);

  foreach (QPersistentModelIndex row, newPersistentRows) {
    selectionModel->select(row, (QItemSelectionModel::Select | QItemSelectionModel::Rows));
  }
}

void ShapeModel::duplicateRow(int position)
{
  beginInsertRows(QModelIndex(), position, position);

  Primitive primitive;
  primitive.type = m_primitives[position].type;
  primitive.depthIndex = m_primitives[position].depthIndex;
  primitive.verticesModel = new VerticesModel(*(m_primitives[position].verticesModel->verticesList()), this);
  primitive.materialsModel = new MaterialsModel(*(m_primitives[position].materialsModel->materialsList()), this);
  primitive.unknown1 = m_primitives[position].unknown1;
  primitive.unknown2 = m_primitives[position].unknown2;

  m_primitives.insert(position, primitive);

  endInsertRows();
}

void ShapeModel::setShape(PrimitivesList& primitives)
{
  if (primitives.isEmpty()) {
    return;
  }

  if (!m_primitives.isEmpty()) {
    beginRemoveRows(QModelIndex(), 0, qMax(0, rowCount() - 1));

    foreach (Primitive primitive, m_primitives) {
      delete primitive.verticesModel;
      delete primitive.materialsModel;
    }
    m_primitives.clear();

    endRemoveRows();
  }

  m_primitives = primitives;

  beginInsertRows(QModelIndex(), 0, m_primitives.size() - 1);

  foreach (Primitive primitive, m_primitives) {
    primitive.verticesModel->setParent(this);
    primitive.materialsModel->setParent(this);
  }

  endInsertRows();

  m_numPaintJobs = m_primitives[0].materialsModel->rowCount();
}

Vertex* ShapeModel::boundBox()
{
  if (m_primitives.isEmpty()) {
    for (int i = 0; i < 8; i++) {
      m_bound[i].x = 0; m_bound[i].y = 0; m_bound[i].z = 0;
    }
  }
  else {
    Vertex first = m_primitives.at(0).verticesModel->verticesList()->at(0);
    qint16 minX = first.x, minY = first.y, minZ = first.z, maxX = minX, maxY = minY, maxZ = minZ;

    foreach (Primitive primitive, m_primitives) {
      foreach (Vertex vertex, *(primitive.verticesModel->verticesList())) {
        if (vertex.x < minX) minX = vertex.x;
        else if (vertex.x > maxX) maxX = vertex.x;
        if (vertex.y < minY) minY = vertex.y;
        else if (vertex.y > maxY) maxY = vertex.y;
        if (vertex.z < minZ) minZ = vertex.z;
        else if (vertex.z > maxZ) maxZ = vertex.z;
      }
    }

    m_bound[0].x = minX; m_bound[0].y = minY; m_bound[0].z = maxZ;
    m_bound[1].x = maxX; m_bound[1].y = minY; m_bound[1].z = maxZ;
    m_bound[2].x = minX; m_bound[2].y = minY; m_bound[2].z = minZ;
    m_bound[3].x = maxX; m_bound[3].y = minY; m_bound[3].z = minZ;
    m_bound[4].x = minX; m_bound[4].y = maxY; m_bound[4].z = maxZ;
    m_bound[5].x = maxX; m_bound[5].y = maxY; m_bound[5].z = maxZ;
    m_bound[6].x = minX; m_bound[6].y = maxY; m_bound[6].z = minZ;
    m_bound[7].x = maxX; m_bound[7].y = maxY; m_bound[7].z = minZ;
  }

  return m_bound;
}

bool ShapeModel::setNumPaintJobs(int& num)
{
  num = qBound(PAINTJOBS_MIN, num, PAINTJOBS_MAX);

  if (num == m_numPaintJobs) {
    return false;
  }

  foreach (Primitive primitive, m_primitives) {
    primitive.materialsModel->resize(num);
  }

  m_numPaintJobs = num;

  return true;
}

void ShapeModel::replaceMaterials(quint8 paintJob, quint8 curMaterial, quint8 newMaterial)
{
  foreach (Primitive primitive, m_primitives) {
    MaterialsList* materials = primitive.materialsModel->materialsList();

    if (materials->at(paintJob) == curMaterial) {
      materials->replace(paintJob, newMaterial);
    }
  }
}

void ShapeModel::isModified()
{
  emit dataChanged(QModelIndex(), QModelIndex());
}
