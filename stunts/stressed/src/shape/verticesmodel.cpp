// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2013 Daniel Stien <daniel@stien.org>
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

#include "shapemodel.h"
#include "verticesmodel.h"

const int VerticesModel::VAL_MIN;
const int VerticesModel::VAL_MAX;

const float VerticesModel::Y_RATIO = 0.8f;

bool VerticesModel::m_weld = false;

VerticesModel::VerticesModel(const VerticesList& vertices, ShapeModel* parent)
: QAbstractTableModel(parent),
  m_vertices(vertices)
{
  foreach (const Vertex& vertex, m_vertices) {
    m_verticesF.append(toInternal(vertex));
  }

  setup();
}

VerticesModel::VerticesModel(int type, ShapeModel* parent)
: QAbstractTableModel(parent)
{
  setup();
  resize(type);
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
        return m_vertices[row].x;
      }
      else if (col == 1) {
        return m_vertices[row].y;
      }
      else if (col == 2) {
        return m_vertices[row].z;
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

  Vertex oldVertex = m_vertices[row];

  switch (col) {
    case 0:
      if (result == m_vertices[row].x) {
        return false;
      }
      m_vertices[row].x = result;
      break;

    case 1:
      if (result == m_vertices[row].y) {
        return false;
      }
      m_vertices[row].y = result;
      break;

    case 2:
      if (result == m_vertices[row].z) {
        return false;
      }
      m_vertices[row].z = result;
      break;

    default:
      return false;
  }

  m_verticesF[row] = toInternal(m_vertices[row]);

  ShapeModel* shapeModel = qobject_cast<ShapeModel*>(QObject::parent());
  shapeModel->computeCull();

  if (m_weld) {
    shapeModel->replaceVertices(oldVertex, m_vertices[row]);
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

    m_vertices.insert(position, vertex);
    m_verticesF.insert(position, toInternal(vertex));
  }

  endInsertRows();
  return true;
}

bool VerticesModel::removeRows(int position, int rows, const QModelIndex& index)
{
  beginRemoveRows(index, position, position + rows - 1);
  
  for (int row = 0; row < rows; row++) {
    m_vertices.removeAt(position);
    m_verticesF.removeAt(position);
  }

  endRemoveRows();

  return true;
}

void VerticesModel::flip()
{
  for (int i = 1; i < m_vertices.size(); i++) {
    m_vertices.move(i, 0);
    m_verticesF.move(i, 0);
  }

  qobject_cast<ShapeModel*>(QObject::parent())->computeCull();
}

void VerticesModel::invertX(bool flip)
{
  for (int i = 0; i < m_vertices.size(); i++) {
    m_vertices[i].x = -m_vertices[i].x;
    m_verticesF[i].x = -m_verticesF[i].x;
  }

  if (flip) {
    this->flip();
  }
  else {
    qobject_cast<ShapeModel*>(QObject::parent())->computeCull();
  }
}

void VerticesModel::replace(const Vertex& curVert, const Vertex& newVert, Primitive& primitive)
{
  bool changed = false;

  int i;
  while ((i = m_vertices.indexOf(curVert)) >= 0) {
    m_vertices.replace(i, newVert);
    m_verticesF.replace(i, toInternal(newVert));
    emit dataChanged(index(i, 0), index(i, 2));
    changed = true;
  }

  if (changed) {
    qobject_cast<ShapeModel*>(QObject::parent())->computeCull(primitive);
  }
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
  if (type < PRIM_TYPE_PARTICLE || type > PRIM_TYPE_WHEEL) {
    num = 0;
    return false;
  }
  else if (type == PRIM_TYPE_SPHERE) {
    num = 2;
  }
  else if (type == PRIM_TYPE_WHEEL) {
    num = 6;
  }
  else {
    num = type;
  }

  return true;
}

VertexF VerticesModel::toInternal(const Vertex& vertex)
{
  VertexF vertexf;
  vertexf.x = vertex.x;
  vertexf.y = (float)vertex.y * Y_RATIO;
  vertexf.z = -vertex.z;
  return vertexf;
}

void VerticesModel::setup()
{
  connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
      qobject_cast<ShapeModel*>(QObject::parent()), SLOT(isModified()));
}
