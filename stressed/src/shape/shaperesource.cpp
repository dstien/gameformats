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

#include <QHeaderView>

#include "materialsmodel.h"
#include "shapemodel.h"
#include "shaperesource.h"
#include "verticesmodel.h"

ShapeResource::ShapeResource(const QString& fileName, QString id, QDataStream* in, QWidget* parent, Qt::WFlags flags) :
  Resource(fileName, id, parent, flags)
{
  ui.setupUi(this);

  ui.primitivesView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui.verticesView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui.materialsView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

  parse(in);
}

void ShapeResource::parse(QDataStream* in)
{
  PrimitivesList primitives = PrimitivesList();

  quint8 numVertices, numPrimitives, numPaintJobs, reserved;
  Vertex* vertices = 0;
  quint32* unknowns1 = 0;
  quint32* unknowns2 = 0;

  // Header.
  *in >> numVertices >> numPrimitives >> numPaintJobs >> reserved;
  checkError(in, tr("header"));

  if (reserved) {
    throw tr("Reserved header field is %1, expected 0.").arg(reserved);
  }

  try {
    try {
      vertices = new Vertex[numVertices];
    }
    catch (std::bad_alloc& exc) {
      throw tr("Couldn't allocate memory for shape vertex data.");
    }

    in->readRawData(reinterpret_cast<char*>(vertices), numVertices * sizeof(Vertex));
    checkError(in, tr("vertices"));

    // Flip Z-axis.
    for (int i = 0; i < numVertices; i++) {
      vertices[i].z = -vertices[i].z;
    }

    try {
      unknowns1 = new quint32[numPrimitives];
      unknowns2 = new quint32[numPrimitives];
    }
    catch (std::bad_alloc& exc) {
      throw tr("Couldn't allocate memory for unknown shape primitive data.");
    }

    in->readRawData(reinterpret_cast<char*>(unknowns1), numPrimitives * sizeof(quint32));
    checkError(in, tr("unknown primitive data 1"));

    in->readRawData(reinterpret_cast<char*>(unknowns2), numPrimitives * sizeof(quint32));
    checkError(in, tr("unknown primitive data 2"));

    quint8 type, depthIndex, material, vertexIndex;
    for (int i = 0; i < numPrimitives; i++) {
      int verticesNeeded;

      MaterialsList materialsList;
      VerticesList verticesList;

      *in >> type >> depthIndex;

      for (int j = 0; j < numPaintJobs; j++) {
        *in >> material;
        materialsList.append(material);
      }
      checkError(in, tr("material indices in primitive %1").arg(i));

      if (type < 1 || type > 12) {
        throw tr("Unknown type (%1) for primitive %2.").arg(type).arg(i);
      }
      else if (type == 11) {
        verticesNeeded = 2;
      }
      else if (type == 12) {
        verticesNeeded = 6;
      }
      else {
        verticesNeeded = type;
      }

      for (int j = 0; j < verticesNeeded; j++) {
        *in >> vertexIndex;
        verticesList.append(vertices[vertexIndex]);
      }
      checkError(in, tr("vertex indices in primitive %1").arg(i));

      Primitive primitive;
      primitive.type = type;
      primitive.depthIndex = depthIndex;
      primitive.verticesModel = new VerticesModel(verticesList);
      primitive.materialsModel = new MaterialsModel(materialsList);

      primitive.unknown1 = unknowns1[i];
      primitive.unknown2 = unknowns2[i];

      connect(primitive.verticesModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(isModified()));
      connect(primitive.materialsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(isModified()));

      primitives.append(primitive);
    }
  }
  catch (QString msg) {
    delete[] vertices;
    vertices = 0;
    delete[] unknowns1;
    unknowns1 = 0;
    delete[] unknowns2;
    unknowns2 = 0;

    throw msg;
  }

  delete[] vertices;
  vertices = 0;
  delete[] unknowns1;
  unknowns1 = 0;
  delete[] unknowns2;
  unknowns2 = 0;

  ShapeModel* shapeModel = new ShapeModel(primitives, this);

  connect(shapeModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
      this, SLOT(isModified()));

  ui.primitivesView->setModel(shapeModel);
  ui.shapeView->setModel(shapeModel);
  ui.shapeView->setSelectionModel(ui.primitivesView->selectionModel());

  ui.numPaintJobsSpinBox->setValue(numPaintJobs);
  ui.paintJobSpinBox->setMaximum(numPaintJobs);

  connect(ui.primitivesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
      this, SLOT(setModels(QModelIndex)));
}

void ShapeResource::write(QDataStream* out) const
{
  VerticesList vertices;
  ShapeModel* shapeModel = qobject_cast<ShapeModel*>(ui.shapeView->model());

  if (!shapeModel) {
    throw tr("No shape data.");
  }

  // Bound box for all shapes but explosion debris.
  if (!id().contains(QRegExp("exp[0-3]{1,1}$"))) {
    Vertex* bound = shapeModel->boundBox();
    for (int i = 0; i < 8; i++) {
      vertices.append(bound[i]);
    }
  }

  // Generate list of unique vertices.
  foreach (Primitive primitive, *(shapeModel->primitivesList())) {
    foreach (Vertex vertex, *(primitive.verticesModel->verticesList())) {
      if (!vertices.contains(vertex)) {
        vertices.append(vertex);
      }
      if (vertices.size() > 256) {
        throw tr("Number of vertices (%1) exceeds limit of 256.").arg(vertices.size());
      }
    }
  }

  // Write header.
  *out << (quint8)vertices.size() << (quint8)shapeModel->rowCount() << (quint8)ui.paintJobSpinBox->maximum() << (quint8)0;
  checkError(out, tr("header"), true);

  // Write vertex data.
  foreach (Vertex vertex, vertices) {
    *out << vertex.x << vertex.y << (qint16)-vertex.z;
  }
  checkError(out, tr("vertices"), true);

  // Write unknowns.
  foreach (Primitive primitive, *(shapeModel->primitivesList())) {
    *out << primitive.unknown1;
  }
  checkError(out, tr("unknown primitive data 1"), true);

  foreach (Primitive primitive, *(shapeModel->primitivesList())) {
    *out << primitive.unknown2;
  }
  checkError(out, tr("unknown primitive data 2"), true);

  // Write primitives.
  int i = 0;
  foreach (Primitive primitive, *(shapeModel->primitivesList())) {
    // Primitive header.
    *out << primitive.type << primitive.depthIndex;
    checkError(out, tr("header for primitive %1").arg(i));

    // Material indices.
    foreach (quint8 material, *(primitive.materialsModel->materialsList())) {
      *out << material;
    }
    checkError(out, tr("material indices in primitive %1").arg(i));

    // Vertex indices.
    foreach (Vertex vertex, *(primitive.verticesModel->verticesList())) {
      *out << (quint8)vertices.indexOf(vertex);
    }
    checkError(out, tr("vertex indices in primitive %1").arg(i));

    i++;
  }

  // Unknown padding.
  *out << (qint8)0 << (qint8)0 << (qint8)0x0f;

  checkError(out, tr("padding"), true);
}

void ShapeResource::deselectAll()
{
  ui.materialsView->clearSelection();
  ui.verticesView->clearSelection();
  ui.primitivesView->clearSelection();
}

void ShapeResource::setModels(const QModelIndex& index)
{
  int row = index.row();
  ShapeModel* shapeModel = qobject_cast<ShapeModel*>(ui.shapeView->model());

  if (!shapeModel || !index.isValid() ||
      (row < 0) || row >= shapeModel->rowCount()) {
    return;
  }

  Primitive currentPrimitive = shapeModel->primitivesList()->at(row);

  ui.verticesView->setModel(currentPrimitive.verticesModel);
  ui.materialsView->setModel(currentPrimitive.materialsModel);

  ui.shapeView->setCurrentIndex(index);
}

void ShapeResource::setNumPaintJobs()
{
  int num = qBound(1, ui.numPaintJobsSpinBox->value(), 127);
  ShapeModel* shapeModel = qobject_cast<ShapeModel*>(ui.shapeView->model());

  if (!shapeModel) {
    return;
  }

  foreach (Primitive primitive, *(shapeModel->primitivesList())) {
    int rows = primitive.materialsModel->rowCount();
    if (num == rows) {
      return;
    }
    else if (num < rows) {
      int diff = rows - num;
      primitive.materialsModel->removeRows(rows - diff, diff);
    }
    else {
      primitive.materialsModel->insertRows(rows, num - rows);
    }
  }

  ui.paintJobSpinBox->setMaximum(num);
  isModified();
}

void ShapeResource::isModified()
{
  ui.shapeView->viewport()->update();
  Resource::isModified();
}
