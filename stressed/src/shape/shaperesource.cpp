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

#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextStream>

#include "app/settings.h"
#include "materialsmodel.h"
#include "shapemodel.h"
#include "shaperesource.h"
#include "typedelegate.h"

QString       ShapeResource::currentFilePath;
QString       ShapeResource::currentFileFilter;

const int     ShapeResource::MAX_VERTICES;

const char    ShapeResource::FILE_SETTINGS_PATH[]  = "paths/shape";
const char    ShapeResource::FILE_FILTERS[]        = "Wavefront OBJ (*.obj);;All files (*)";
const char    ShapeResource::MTL_SRC[]             = ":/shape/materials.mtl";
const char    ShapeResource::MTL_DST[]             = "stunts.mtl";

const QRegExp ShapeResource::OBJ_REGEXP_WHITESPACE = QRegExp("\\s+");
const QRegExp ShapeResource::OBJ_REGEXP_VERTEX     = QRegExp("^v(\\s+([+-]?\\d*\\.\\d+)(?![-+0-9\\.])){3}\\s*$");
const QRegExp ShapeResource::OBJ_REGEXP_FACE       = QRegExp("^(fo?|[lp])((\\s+-?\\d+)(/-?\\d*){,2}){1,10}\\s*$");

ShapeResource::ShapeResource(QString id, QDataStream* in, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags)
{
  ui.setupUi(this);

  ui.primitivesView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui.verticesView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui.materialsView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

  ui.primitivesView->setItemDelegateForColumn(0, new TypeDelegate(ui.primitivesView));

  shapeModel = new ShapeModel(this);

  ui.primitivesView->setModel(shapeModel);
  ui.shapeView->setModel(shapeModel);
  ui.shapeView->setSelectionModel(ui.primitivesView->selectionModel());

  parse(in);
  ui.shapeView->reset();

  connect(shapeModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
      this, SLOT(isModified()));
  connect(ui.primitivesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
      this, SLOT(setModels(QModelIndex)));
}

void ShapeResource::parse(QDataStream* in)
{
  PrimitivesList primitives;

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

      if (!VerticesModel::verticesNeeded(type, verticesNeeded)) {
        throw tr("Unknown type (%1) for primitive %2.").arg(type).arg(i);
      }

      for (int j = 0; j < verticesNeeded; j++) {
        *in >> vertexIndex;
        verticesList.append(vertices[vertexIndex]);
      }
      checkError(in, tr("vertex indices in primitive %1").arg(i));

      Primitive primitive;
      primitive.type = type;
      primitive.depthIndex = depthIndex;
      primitive.verticesModel = new VerticesModel(verticesList, shapeModel);
      primitive.materialsModel = new MaterialsModel(materialsList, shapeModel);

      primitive.unknown1 = unknowns1[i];
      primitive.unknown2 = unknowns2[i];

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

  shapeModel->setShape(primitives);

  ui.numPaintJobsSpinBox->setValue(numPaintJobs);
  ui.paintJobSpinBox->setMaximum(numPaintJobs);
}

void ShapeResource::write(QDataStream* out) const
{
  // Generate list of unique vertices, include bound box for all shapes but
  // explosion debris.
  VerticesList vertices = buildVerticesList(!id().contains(QRegExp("exp[0-3]{1,1}$")));

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
  *out << (qint8)0 << (qint8)0 << (qint8)0;

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

  if (!index.isValid() || row >= shapeModel->rowCount()) {
    return;
  }

  Primitive currentPrimitive = shapeModel->primitivesList()->at(row);

  ui.verticesView->setModel(currentPrimitive.verticesModel);
  ui.materialsView->setModel(currentPrimitive.materialsModel);

  ui.shapeView->setCurrentIndex(index);
}

void ShapeResource::setNumPaintJobs()
{
  int num = ui.numPaintJobsSpinBox->value();
  if (shapeModel->setNumPaintJobs(num)) {
    ui.paintJobSpinBox->setMaximum(num);
    isModified();
  }
}

void ShapeResource::moveFirstPrimitives()
{
  if (ui.primitivesView->selectionModel()->hasSelection()) {
    shapeModel->moveRows(ui.primitivesView->selectionModel(), -256);
    isModified();
  }
}

void ShapeResource::moveUpPrimitives()
{
  if (ui.primitivesView->selectionModel()->hasSelection()) {
    shapeModel->moveRows(ui.primitivesView->selectionModel(), -1);
    isModified();
  }
}

void ShapeResource::moveDownPrimitives()
{
  if (ui.primitivesView->selectionModel()->hasSelection()) {
    shapeModel->moveRows(ui.primitivesView->selectionModel(), 1);
    isModified();
  }
}

void ShapeResource::moveLastPrimitives()
{
  if (ui.primitivesView->selectionModel()->hasSelection()) {
    shapeModel->moveRows(ui.primitivesView->selectionModel(), 256);
    isModified();
  }
}

void ShapeResource::insertPrimitive()
{
  int row;
  if (ui.primitivesView->selectionModel()->hasSelection()) {
    row = ui.primitivesView->currentIndex().row();
  }
  else {
    // Insert at end if no rows are selected.
    row = 256;
  }

  shapeModel->insertRows(row, 1);

  // Select the new row.
  ui.primitivesView->selectionModel()->reset();
  ui.primitivesView->setCurrentIndex(shapeModel->index((row < 256 ? row : shapeModel->rowCount() - 1), 0));

  isModified();
}

void ShapeResource::duplicatePrimitive()
{
  if (ui.primitivesView->selectionModel()->hasSelection()) {
    int row = ui.primitivesView->currentIndex().row();
    shapeModel->duplicateRow(row);

    ui.primitivesView->setCurrentIndex(shapeModel->index(row, 0));
    isModified();
  }
}

void ShapeResource::removePrimitives()
{
  shapeModel->removeRows(ui.primitivesView->selectionModel()->selectedRows());

  if (!shapeModel->rowCount()) {
    ui.verticesView->setModel(0);
    ui.materialsView->setModel(0);
  }

  isModified();
}

void ShapeResource::primitivesContextMenu(const QPoint& /*pos*/)
{
  if (ui.primitivesView->selectionModel()->hasSelection()) {
    ui.moveFirstPrimitivesAction->setEnabled(true);
    ui.moveUpPrimitivesAction->setEnabled(true);
    ui.moveDownPrimitivesAction->setEnabled(true);
    ui.moveLastPrimitivesAction->setEnabled(true);

    ui.duplicatePrimitiveAction->setEnabled(true);
    ui.removePrimitivesAction->setEnabled(true);
  }
  else {
    ui.moveFirstPrimitivesAction->setEnabled(false);
    ui.moveUpPrimitivesAction->setEnabled(false);
    ui.moveDownPrimitivesAction->setEnabled(false);
    ui.moveLastPrimitivesAction->setEnabled(false);

    ui.duplicatePrimitiveAction->setEnabled(false);
    ui.removePrimitivesAction->setEnabled(false);
  }

  if (shapeModel->rowCount() >= 255) {
    ui.insertPrimitiveAction->setEnabled(false);
    ui.duplicatePrimitiveAction->setEnabled(false);
  }
  else {
    ui.insertPrimitiveAction->setEnabled(true);
  }

  ui.primitivesMenu->exec(QCursor::pos());
}

void ShapeResource::replaceMaterials()
{
  if (ui.materialsView->model() && ui.materialsView->selectionModel()->hasSelection()) {
    bool success;
    int curMaterial = ui.materialsView->model()->data(ui.materialsView->currentIndex()).toUInt();
    int newMaterial = QInputDialog::getInteger(
        this,
        tr("Replace material"),
        tr("New material (0 - 255):"),
        curMaterial, 0, 255, 1, &success);

    if (success && (newMaterial != curMaterial)) {
      shapeModel->replaceMaterials(ui.materialsView->currentIndex().row(), curMaterial, newMaterial);
      isModified();
    }
  }
}

void ShapeResource::materialsContextMenu(const QPoint& /*pos*/)
{
  if (ui.materialsView->model() && ui.materialsView->selectionModel()->hasSelection()) {
    ui.replaceMaterialsAction->setEnabled(true);
  }
  else {
    ui.replaceMaterialsAction->setEnabled(false);
  }

  ui.materialsMenu->exec(QCursor::pos());
}

void ShapeResource::exportFile()
{
  if (currentFilePath.isEmpty()) {
    currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);
  }

  QFileInfo fileInfo(currentFilePath);
  fileInfo.setFile(
      fileInfo.absolutePath() +
      QDir::separator() +
      QString("%1-%2.obj").arg(QString(fileName()).replace('.', '_'), id()));
  currentFilePath = fileInfo.absoluteFilePath();

  QString outFileName = QFileDialog::getSaveFileName(
      this,
      tr("Export shape"),
      currentFilePath,
      FILE_FILTERS,
      &currentFileFilter);

  if (!outFileName.isEmpty()) {
    Settings().setFilePath(FILE_SETTINGS_PATH, currentFilePath = outFileName);
    fileInfo.setFile(currentFilePath);

    try {
      QFile objFile(currentFilePath);
      if (objFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(&objFile);
        out << "# " << Settings::APP_NAME << " - " << Settings::APP_DESC << endl;
        out << "# " << Settings::ORG_URL << endl;
        out << tr("# Shape \"%1\" exported from file \"%2\"").arg(id(), fileName()) << endl << endl;

        out << "mtllib " << MTL_DST << endl << endl;

        VerticesList vertices = buildVerticesList();
        foreach (Vertex vertex, vertices) {
          out << "v" << qSetFieldWidth(10) << right << fixed << qSetRealNumberPrecision(1)
              << (float)vertex.x << (float)vertex.y << (float)vertex.z << reset << endl;
        }

        out << endl;

        int prevMat = -1, curMat;
        foreach (Primitive primitive, *(shapeModel->primitivesList())) {
          curMat = primitive.materialsModel->materialsList()->at(ui.paintJobSpinBox->value() - 1);
          if (curMat != prevMat) {
            prevMat = curMat;
            out << QString("usemtl Stunts%1").arg(curMat, 3, 10, QChar('0')) << endl;
          }

          switch (primitive.type) {
            case 1:
              out << "p";
              break;

            case 2:
            case 11:
            case 12:
              out << "l";
              break;

            default:
              out << "f";
          }

          out << qSetFieldWidth(4) << right;
          foreach (Vertex vertex, *(primitive.verticesModel->verticesList())) {
            out << vertices.indexOf(vertex) + 1;
          }
          out << reset << endl;

          if (out.status()) {
            throw tr("Couldn't write to file.");
          }
        }

        objFile.close();

        QFileInfo mtlFileInfo(fileInfo.dir(), MTL_DST);
        QFile::copy(MTL_SRC, mtlFileInfo.absoluteFilePath());
      }
      else {
        throw tr("Couldn't open file for writing.");
      }
    }
    catch (QString msg) {
      QMessageBox::critical(
          this,
          QCoreApplication::applicationName(),
          tr("Error exporting shape resource \"%1\" to Wavefront OBJ file \"%2\": %3").arg(id(), currentFilePath, msg));
    }
  }
}

void ShapeResource::importFile()
{
  if (currentFilePath.isEmpty()) {
    currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);
  }

  QString inFileName = QFileDialog::getOpenFileName(
      this,
      tr("Import shape"),
      currentFilePath,
      FILE_FILTERS,
      &currentFileFilter);

  if (!inFileName.isEmpty()) {
    Settings().setFilePath(FILE_SETTINGS_PATH, currentFilePath = inFileName);

    try {
      QFile objFile(currentFilePath);
      if (objFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&objFile);

        PrimitivesList primitives;
        VerticesList vertices;
        quint8 material = 0;

        int lineNum = 0;
        try {
          QString line;
          while (!(line = in.readLine()).isNull()) {
            lineNum++;

            switch (line[0].toAscii()) {
              case 'v':
                if (line.contains(OBJ_REGEXP_VERTEX)) {
                  QStringList tokens = line.split(OBJ_REGEXP_WHITESPACE);

                  Vertex vertex;
                  vertex.x = (qint16)tokens[1].toFloat();
                  vertex.y = (qint16)tokens[2].toFloat();
                  vertex.z = (qint16)tokens[3].toFloat();
                  vertices.append(vertex);
                }
                break;

              case 'f':
              case 'l':
              case 'p':
                if (line.contains(OBJ_REGEXP_FACE)) {
                  if (vertices.isEmpty()) {
                    throw tr("Found primitive before any vertices.");
                  }

                  QStringList tokens = line.split(OBJ_REGEXP_WHITESPACE, QString::SkipEmptyParts);

                  Primitive primitive;
                  int numVertices = tokens.size() - 1;

                  if (line[0].toAscii() == 'l' && numVertices == 6) {
                    primitive.type = 12; // Wheel.
                  }
                  else {
                    primitive.type = numVertices;
                  }
                  primitive.depthIndex = 0;

                  VerticesList faceVertices;
                  for (int i = 0; i < numVertices; i++) {
                    int index = tokens[i + 1].section('/', 0, 0).toInt();

                    if (index == 0 || index > vertices.size()) {
                      throw tr("Vertex index %1 out of bounds (1 - %2).").arg(index).arg(vertices.size());
                    }
                    else if (index < 0) {
                      throw tr("Negative vertex indices not supported (got %1).").arg(index);
                    }
                    faceVertices.append(vertices[index - 1]);
                  }

                  primitive.verticesModel = new VerticesModel(faceVertices, shapeModel);
                  primitive.materialsModel = new MaterialsModel(material, shapeModel);
                  primitive.unknown1 = 0xFFFFFFFF;
                  primitive.unknown2 = 0xFFFFFFFF;
                  primitives.append(primitive);
                }
                break;

              case 'u':
                if (line.startsWith("usemtl")) {
                  material = line.trimmed().right(3).toUInt();
                }
                break;
            }
          }

          if (in.status()) {
            throw tr("Couldn't read from file.");
          }
        }
        catch (QString msg) {
          // Clean-up.
          foreach (Primitive primitive, primitives) {
            delete primitive.verticesModel;
            delete primitive.materialsModel;
          }

          throw tr("Parsing error at line %1: %2").arg(lineNum).arg(msg);
        }

        objFile.close();

        if (primitives.isEmpty()) {
          throw tr("No faces found in file.");
        }

        shapeModel->setShape(primitives);
        ui.shapeView->reset();

        ui.numPaintJobsSpinBox->setValue(1);
        ui.paintJobSpinBox->setMaximum(1);

        isModified();
      }
      else {
        throw tr("Couldn't open file for reading.");
      }
    }
    catch (QString msg) {
      QMessageBox::critical(
          this,
          QCoreApplication::applicationName(),
          tr("Error importing Wavefront OBJ file \"%1\" to shape resource \"%2\": %3").arg(currentFilePath, id(), msg));
    }
  }
}

VerticesList ShapeResource::buildVerticesList(bool boundBox) const
{
  VerticesList vertices;

  if (boundBox) {
    Vertex* bound = shapeModel->boundBox();
    for (int i = 0; i < 8; i++) {
      vertices.append(bound[i]);
    }
  }

  foreach (Primitive primitive, *(shapeModel->primitivesList())) {
    foreach (Vertex vertex, *(primitive.verticesModel->verticesList())) {
      if (!vertices.contains(vertex)) {
        vertices.append(vertex);
      }
      if (vertices.size() > MAX_VERTICES) {
        throw tr("Number of vertices (%1) exceeds limit of %2.").arg(vertices.size()).arg(MAX_VERTICES);
      }
    }
  }

  return vertices;
}

void ShapeResource::isModified()
{
  ui.shapeView->viewport()->update();
  Resource::isModified();
}
