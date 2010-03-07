// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2010 Daniel Stien <daniel@stien.org>
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
#include <cmath>

#include "materialsmodel.h"
#include "shapemodel.h"
#include "shaperesource.h"
#include "vector3.h"
#include "verticesmodel.h"

const int ShapeModel::ROWS_MAX;

const int ShapeModel::TYPE_MIN;
const int ShapeModel::TYPE_MAX;

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

  switch (index.column()) {
    case 1:
    case 2:
      return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    case 3:
    case 4:
    case 5:
    case 6:
      return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;

    default:
      return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }
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
      if (col == 0) {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      }
      else {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
      }
    case Qt::FontRole:
      if (col > 2) {
        return "Monospace, Courier";
      }
      break;
    case Qt::DisplayRole:
      if (col == 0) {
        return TYPES[m_primitives[row].type - 1];
      }
    case Qt::EditRole:
      if (col == 0) {
        return m_primitives[row].type;
      }
      else if (col == 3) {
        return QString("%1").arg(PRIM_CULL_POS_GET(m_primitives[row].cull1), 5, 8, QChar('0')).toUpper();
      }
      else if (col == 4) {
        return QString("%1").arg(PRIM_CULL_NEG_GET(m_primitives[row].cull1), 5, 8, QChar('0')).toUpper();
      }
      else if (col == 5) {
        return QString("%1").arg(PRIM_CULL_POS_GET(m_primitives[row].cull2), 5, 8, QChar('0')).toUpper();
      }
      else if (col == 6) {
        return QString("%1").arg(PRIM_CULL_NEG_GET(m_primitives[row].cull2), 5, 8, QChar('0')).toUpper();
      }
      break;
    case Qt::CheckStateRole:
      if (col == 1) {
        return m_primitives[row].twoSided ? Qt::Checked : Qt::Unchecked;
      }
      else if (col == 2) {
        return m_primitives[row].zBias ? Qt::Checked : Qt::Unchecked;
      }
      else if (col == 3) {
        return m_primitives[row].cull1 & PRIM_CULL_POS_FLAG ? Qt::Checked : Qt::Unchecked;
      }
      else if (col == 4) {
        return m_primitives[row].cull1 & PRIM_CULL_NEG_FLAG ? Qt::Checked : Qt::Unchecked;
      }
      else if (col == 5) {
        return m_primitives[row].cull2 & PRIM_CULL_POS_FLAG ? Qt::Checked : Qt::Unchecked;
      }
      else if (col == 6) {
        return m_primitives[row].cull2 & PRIM_CULL_NEG_FLAG ? Qt::Checked : Qt::Unchecked;
      }
  }

  return QVariant();
}

bool ShapeModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
  int row = index.row(), col = index.column();

  if (!index.isValid() || (role != Qt::EditRole && role != Qt::CheckStateRole) ||
      row >= rowCount() || col >= columnCount()) {
    return false;
  }

  bool success;

  if (role == Qt::EditRole) {
    if (col == 0) {
      quint8 result = qBound(TYPE_MIN, value.toInt(&success), TYPE_MAX);

      if (!success || (result == m_primitives[row].type)) {
        return false;
      }

      m_primitives[row].verticesModel->resize(result);
      m_primitives[row].type = result;
    }
    else if (col == 1 || col == 2) {
      return false;
    }
    else {
      quint32 result = value.toString().toUInt(&success, 8);

      if (!success) {
        return false;
      }

      if (col == 3) {
        if (result == PRIM_CULL_POS_GET(m_primitives[row].cull1)) {
          return false;
        }
        PRIM_CULL_POS_SET(m_primitives[row].cull1, result);
      }
      else if (col == 4) {
        if (result == PRIM_CULL_NEG_GET(m_primitives[row].cull1)) {
          return false;
        }
        PRIM_CULL_NEG_SET(m_primitives[row].cull1, result);
      }
      else if (col == 5) {
        if (result == PRIM_CULL_POS_GET(m_primitives[row].cull2)) {
          return false;
        }
        PRIM_CULL_POS_SET(m_primitives[row].cull2, result);
      }
      else if (col == 6) {
        if (result == PRIM_CULL_NEG_GET(m_primitives[row].cull2)) {
          return false;
        }
        PRIM_CULL_NEG_SET(m_primitives[row].cull2, result);
      }
      else {
        return false;
      }
    }
  }
  else if (Qt::CheckStateRole) {
    if (col == 1) {
      m_primitives[row].twoSided = value.toBool();
    }
    else if (col == 2) {
      m_primitives[row].zBias = value.toBool();
    }
    else if (col == 3) {
      if (value.toBool()) {
        m_primitives[row].cull1 |= PRIM_CULL_POS_FLAG;
      }
      else {
        m_primitives[row].cull1 &= ~PRIM_CULL_POS_FLAG;
      }
    }
    else if (col == 4) {
      if (value.toBool()) {
        m_primitives[row].cull1 |= PRIM_CULL_NEG_FLAG;
      }
      else {
        m_primitives[row].cull1 &= ~PRIM_CULL_NEG_FLAG;
      }
    }
    else if (col == 5) {
      if (value.toBool()) {
        m_primitives[row].cull2 |= PRIM_CULL_POS_FLAG;
      }
      else {
        m_primitives[row].cull2 &= ~PRIM_CULL_POS_FLAG;
      }
    }
    else if (col == 6) {
      if (value.toBool()) {
        m_primitives[row].cull2 |= PRIM_CULL_NEG_FLAG;
      }
      else {
        m_primitives[row].cull2 &= ~PRIM_CULL_NEG_FLAG;
      }
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
        return "2-sided";
      case 2:
        return "Z-bias";
      case 3:
        return "C1+";
      case 4:
        return "C1-";
      case 5:
        return "C2+";
      case 6:
      default:
        return "C2-";
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
    primitive.type = PRIM_TYPE_PARTICLE;
    primitive.twoSided = false;
    primitive.zBias = false;
    primitive.verticesModel = new VerticesModel(primitive.type, this);
    primitive.materialsModel = new MaterialsModel(m_numPaintJobs, this);
    primitive.cull1 = 0xFFFFFFFF;
    primitive.cull2 = 0xFFFFFFFF;

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

  Primitive primitive = m_primitives[position];
  primitive.verticesModel = new VerticesModel(*(m_primitives[position].verticesModel->verticesList()), this);
  primitive.materialsModel = new MaterialsModel(*(m_primitives[position].materialsModel->materialsList()), this);

  m_primitives.insert(position, primitive);

  endInsertRows();
}

void ShapeModel::mirrorXRow(int position)
{
  duplicateRow(position);
  m_primitives[position + 1].verticesModel->invertX(
      m_primitives[position + 1].type > PRIM_TYPE_LINE &&
      m_primitives[position + 1].type < PRIM_TYPE_SPHERE);
}

void ShapeModel::computeCullRows(const QModelIndexList& rows)
{
  foreach (const QModelIndex& row, rows) {
    computeCull(m_primitives[row.row()]);
  }

  if (!rows.isEmpty()) {
    emit dataChanged(rows.first(), rows.last());
  }
}

void ShapeModel::computeCull()
{
  Primitive* primitive = qobject_cast<ShapeResource*>(QObject::parent())->currentPrimitive();

  if (primitive) {
    computeCull(*primitive);
  }
}

void ShapeModel::computeCull(Primitive& primitive)
{
  if (primitive.twoSided || (primitive.type <= PRIM_TYPE_LINE) || (primitive.type == PRIM_TYPE_SPHERE)) {
    primitive.cull1 = primitive.cull2 = 0xFFFFFFFF;
  }
  else if (primitive.type == PRIM_TYPE_WHEEL) {
    primitive.cull1 = 0xFFFFFFFF;
    primitive.cull2 = 0xFFFFFFFF; // TODO: Like stock cars.
  }
  else {
    primitive.cull1 = primitive.cull2 = 0;

    VerticesList* vertices = primitive.verticesModel->verticesList();
    Vector3 edge1 = Vector3(vertices->at(1)) - Vector3(vertices->at(0));
    Vector3 edge2 = Vector3(vertices->at(2)) - Vector3(vertices->at(0));
    Vector3 normal = edge1.crossProduct(edge2).normalize();

    float yAngle = normal.angle(Vector3(0.0f, 1.0f, 0.0f)) * (180.0f / M_PI);

    if (!std::isnan(yAngle)) {
      // C1 flags
      if (yAngle >= 0.0f && yAngle < 135.0f) {   // C1+
        primitive.cull1 |= PRIM_CULL_POS_FLAG;
      }
      if (yAngle > 45.0f && yAngle <= 180.0f) {  // C1-
        primitive.cull1 |= PRIM_CULL_NEG_FLAG;
      }

      quint16 c1p, c1n, c2p, c2n;

      // C1+/C2- fields
      if (yAngle == 180.0f) {
        c1p = 0;
        c2n = 0;
      }
      else if (yAngle >= 75.0f && yAngle < 180.0f) {
        c1p = PRIM_CULL_9BITS;
        c2n = PRIM_CULL_7BITS;
      }
      else if (yAngle >= 55.0f && yAngle < 75.0f) {
        c1p = PRIM_CULL_11BITS;
        c2n = PRIM_CULL_5BITS;
      }
      else if (yAngle >= 45.0f && yAngle < 55.0f) {
        c1p = PRIM_CULL_13BITS;
        c2n = PRIM_CULL_3BITS;
      }
      else /*if (yAngle >= 0.0f && yAngle < 45.0f)*/ {
        c1p = PRIM_CULL_15BITS;
        c2n = 0;
        primitive.cull2 |= PRIM_CULL_POS_FLAG; // C2+ flag
      }

      // C1-/C2+ fields
      if (yAngle == 0.0f) {
        c1n = 0;
        c2p = 0;
      }
      else if (yAngle > 0.0f && yAngle <= 105.0f) {
        c1n = PRIM_CULL_9BITS;
        c2p = PRIM_CULL_7BITS;
      }
      else if (yAngle > 105.0f && yAngle <= 125.0f) {
        c1n = PRIM_CULL_11BITS;
        c2p = PRIM_CULL_5BITS;
      }
      else if (yAngle > 125.0f && yAngle <= 135.0f) {
        c1n = PRIM_CULL_13BITS;
        c2p = PRIM_CULL_3BITS;
      }
      else /*if (yAngle > 135.0f && yAngle <= 180.0f)*/ {
        c1n = PRIM_CULL_15BITS;
        c2p = 0;
        primitive.cull2 |= PRIM_CULL_NEG_FLAG; // C2- flag
      }

      // Rotate fields
      int rotation  = (int)(((M_PI + atan2(normal.x, normal.z)) / (2.0f * M_PI)) * 15.0f + 0.5f);

      if (c1p) {
        if (c1p == PRIM_CULL_15BITS) {
          PRIM_CULL_POS_SET(primitive.cull1, c1p);
        }
        else {
          PRIM_CULL_POS_SET(primitive.cull1, PRIM_CULL_ROTATE(c1p, rotation));
        }
      }

      if (c1n) {
        if (c1n == PRIM_CULL_15BITS) {
          PRIM_CULL_NEG_SET(primitive.cull1, c1n);
        }
        else {
          PRIM_CULL_NEG_SET(primitive.cull1, PRIM_CULL_ROTATE(c1n, rotation));
        }
      }

      if (c2p) {
        PRIM_CULL_POS_SET(primitive.cull2, PRIM_CULL_ROTATE(c2p, rotation));
      }

      if (c2n) {
        PRIM_CULL_NEG_SET(primitive.cull2, PRIM_CULL_ROTATE(c2n, rotation));
      }
    }
  }
}

void ShapeModel::setShape(PrimitivesList& primitives)
{
  if (primitives.isEmpty()) {
    return;
  }

  if (!m_primitives.isEmpty()) {
    beginRemoveRows(QModelIndex(), 0, qMax(0, rowCount() - 1));

    foreach (const Primitive& primitive, m_primitives) {
      delete primitive.verticesModel;
      delete primitive.materialsModel;
    }
    m_primitives.clear();

    endRemoveRows();
  }

  m_primitives = primitives;

  beginInsertRows(QModelIndex(), 0, m_primitives.size() - 1);

  foreach (const Primitive& primitive, m_primitives) {
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

void ShapeModel::replaceVertices(const Vertex& curVert, const Vertex& newVert)
{
  foreach (const Primitive& primitive, m_primitives) {
    primitive.verticesModel->replace(curVert, newVert, (Primitive&)primitive);
  }
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
