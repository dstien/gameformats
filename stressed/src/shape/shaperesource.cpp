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

QString       ShapeResource::m_currentFilePath;
QString       ShapeResource::m_currentFileFilter;

const int     ShapeResource::MAX_VERTICES;

const char    ShapeResource::FILE_SETTINGS_PATH[]  = "paths/shape";
const char    ShapeResource::FILE_FILTERS[]        = "Wavefront OBJ (*.obj);;All files (*)";
const char    ShapeResource::MTL_SRC[]             = ":/shape/materials.mtl";
const char    ShapeResource::MTL_DST[]             = "stunts.mtl";

const QRegExp ShapeResource::OBJ_REGEXP_WHITESPACE = QRegExp("\\s+");
const QRegExp ShapeResource::OBJ_REGEXP_VERTEX     = QRegExp("^v(\\s+([+-]?\\d*\\.\\d+)(?![-+0-9\\.])){3}\\s*$");
const QRegExp ShapeResource::OBJ_REGEXP_FACE       = QRegExp("^(fo?|[lp])((\\s+-?\\d+)(/-?\\d*){,2}){1,10}\\s*$");

ShapeResource::ShapeResource(const ShapeResource& res)
: Resource(res.id(), dynamic_cast<QWidget*>(res.parent()), res.windowFlags())
{
  m_shapeModel = new ShapeModel(*res.m_shapeModel, this);
  setup();
}

ShapeResource::ShapeResource(QString id, QDataStream* in, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags)
{
  m_shapeModel = new ShapeModel(this);
  parse(in);
  setup();
}

void ShapeResource::setup()
{
  m_ui.setupUi(this);

  m_ui.primitivesView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  m_ui.verticesView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  m_ui.materialsView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

  m_ui.primitivesView->setItemDelegateForColumn(0, new TypeDelegate(m_ui.primitivesView));

  m_ui.primitivesView->setModel(m_shapeModel);
  m_ui.shapeView->setModel(m_shapeModel);
  m_ui.shapeView->setSelectionModel(m_ui.primitivesView->selectionModel());

  m_ui.shapeView->reset();

  m_ui.numPaintJobsSpinBox->setValue(m_shapeModel->numPaintJobs());
  m_ui.paintJobSpinBox->setMaximum(m_shapeModel->numPaintJobs());

  connect(m_shapeModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
      this, SLOT(isModified()));
  connect(m_ui.primitivesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
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
      primitive.verticesModel = new VerticesModel(verticesList, m_shapeModel);
      primitive.materialsModel = new MaterialsModel(materialsList, m_shapeModel);

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

  m_shapeModel->setShape(primitives);
}

void ShapeResource::write(QDataStream* out) const
{
  // Generate list of unique vertices, include bound box for all shapes but
  // explosion debris.
  VerticesList vertices = buildVerticesList(!id().contains(QRegExp("exp[0-3]{1,1}$")));

  // Write header.
  *out << (quint8)vertices.size() << (quint8)m_shapeModel->rowCount() << (quint8)m_ui.paintJobSpinBox->maximum() << (quint8)0;
  checkError(out, tr("header"), true);

  // Write vertex data.
  foreach (Vertex vertex, vertices) {
    *out << vertex.x << vertex.y << vertex.z;
  }
  checkError(out, tr("vertices"), true);

  // Write unknowns.
  foreach (Primitive primitive, *(m_shapeModel->primitivesList())) {
    *out << primitive.unknown1;
  }
  checkError(out, tr("unknown primitive data 1"), true);

  foreach (Primitive primitive, *(m_shapeModel->primitivesList())) {
    *out << primitive.unknown2;
  }
  checkError(out, tr("unknown primitive data 2"), true);

  // Write primitives.
  int i = 0;
  foreach (Primitive primitive, *(m_shapeModel->primitivesList())) {
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
  m_ui.materialsView->clearSelection();
  m_ui.verticesView->clearSelection();
  m_ui.primitivesView->clearSelection();
}

void ShapeResource::setModels(const QModelIndex& index)
{
  int row = index.row();

  if (!index.isValid() || row >= m_shapeModel->rowCount()) {
    return;
  }

  Primitive currentPrimitive = m_shapeModel->primitivesList()->at(row);

  m_ui.verticesView->setModel(currentPrimitive.verticesModel);
  m_ui.materialsView->setModel(currentPrimitive.materialsModel);

  m_ui.shapeView->setCurrentIndex(index);
}

void ShapeResource::setNumPaintJobs()
{
  int num = m_ui.numPaintJobsSpinBox->value();
  if (m_shapeModel->setNumPaintJobs(num)) {
    m_ui.paintJobSpinBox->setMaximum(num);
    isModified();
  }
}

void ShapeResource::movePrimitives(int direction)
{
  if (m_ui.primitivesView->selectionModel()->hasSelection()) {
    m_shapeModel->moveRows(m_ui.primitivesView->selectionModel(), direction);
    isModified();
  }
}

void ShapeResource::moveFirstPrimitives()
{
  movePrimitives(-ShapeModel::ROWS_MAX);
}

void ShapeResource::moveUpPrimitives()
{
  movePrimitives(-1);
}

void ShapeResource::moveDownPrimitives()
{
  movePrimitives(1);
}

void ShapeResource::moveLastPrimitives()
{
  movePrimitives(ShapeModel::ROWS_MAX);
}

void ShapeResource::insertPrimitive()
{
  int row;
  if (m_ui.primitivesView->selectionModel()->hasSelection()) {
    row = m_ui.primitivesView->currentIndex().row();
  }
  else {
    // Insert at end if no rows are selected.
    row = ShapeModel::ROWS_MAX;
  }

  m_shapeModel->insertRows(row, 1);

  // Select the new row.
  m_ui.primitivesView->selectionModel()->reset();
  m_ui.primitivesView->setCurrentIndex(m_shapeModel->index((row < ShapeModel::ROWS_MAX ? row : m_shapeModel->rowCount() - 1), 0));

  isModified();
}

void ShapeResource::duplicatePrimitive()
{
  if (m_ui.primitivesView->selectionModel()->hasSelection()) {
    int row = m_ui.primitivesView->currentIndex().row();
    m_shapeModel->duplicateRow(row);

    m_ui.primitivesView->setCurrentIndex(m_shapeModel->index(row, 0));
    isModified();
  }
}

void ShapeResource::removePrimitives()
{
  m_shapeModel->removeRows(m_ui.primitivesView->selectionModel()->selectedRows());

  if (!m_shapeModel->rowCount()) {
    m_ui.verticesView->setModel(0);
    m_ui.materialsView->setModel(0);
  }

  isModified();
}

void ShapeResource::primitivesContextMenu(const QPoint& /*pos*/)
{
  if (m_ui.primitivesView->selectionModel()->hasSelection()) {
    m_ui.moveFirstPrimitivesAction->setEnabled(true);
    m_ui.moveUpPrimitivesAction->setEnabled(true);
    m_ui.moveDownPrimitivesAction->setEnabled(true);
    m_ui.moveLastPrimitivesAction->setEnabled(true);

    m_ui.duplicatePrimitiveAction->setEnabled(true);
    m_ui.removePrimitivesAction->setEnabled(true);
  }
  else {
    m_ui.moveFirstPrimitivesAction->setEnabled(false);
    m_ui.moveUpPrimitivesAction->setEnabled(false);
    m_ui.moveDownPrimitivesAction->setEnabled(false);
    m_ui.moveLastPrimitivesAction->setEnabled(false);

    m_ui.duplicatePrimitiveAction->setEnabled(false);
    m_ui.removePrimitivesAction->setEnabled(false);
  }

  if (m_shapeModel->rowCount() >= 255) {
    m_ui.insertPrimitiveAction->setEnabled(false);
    m_ui.duplicatePrimitiveAction->setEnabled(false);
  }
  else {
    m_ui.insertPrimitiveAction->setEnabled(true);
  }

  m_ui.primitivesMenu->exec(QCursor::pos());
}

void ShapeResource::replaceMaterials()
{
  if (m_ui.materialsView->model() && m_ui.materialsView->selectionModel()->hasSelection()) {
    bool success;
    int curMaterial = m_ui.materialsView->model()->data(m_ui.materialsView->currentIndex()).toUInt();
    int newMaterial = QInputDialog::getInteger(
        this,
        tr("Replace material"),
        tr("New material (0 - 255):"),
        curMaterial, 0, 255, 1, &success);

    if (success && (newMaterial != curMaterial)) {
      m_shapeModel->replaceMaterials(m_ui.materialsView->currentIndex().row(), curMaterial, newMaterial);
      isModified();
    }
  }
}

void ShapeResource::materialsContextMenu(const QPoint& /*pos*/)
{
  if (m_ui.materialsView->model() && m_ui.materialsView->selectionModel()->hasSelection()) {
    m_ui.replaceMaterialsAction->setEnabled(true);
  }
  else {
    m_ui.replaceMaterialsAction->setEnabled(false);
  }

  m_ui.materialsMenu->exec(QCursor::pos());
}

void ShapeResource::exportFile()
{
  if (m_currentFilePath.isEmpty()) {
    m_currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);
  }

  QFileInfo fileInfo(m_currentFilePath);
  fileInfo.setFile(
      fileInfo.absolutePath() +
      QDir::separator() +
      QString("%1-%2.obj").arg(QString(fileName()).replace('.', '_'), id()));
  m_currentFilePath = fileInfo.absoluteFilePath();

  QString outFileName = QFileDialog::getSaveFileName(
      this,
      tr("Export shape"),
      m_currentFilePath,
      FILE_FILTERS,
      &m_currentFileFilter);

  if (!outFileName.isEmpty()) {
    Settings().setFilePath(FILE_SETTINGS_PATH, m_currentFilePath = outFileName);
    fileInfo.setFile(m_currentFilePath);

    try {
      QFile objFile(m_currentFilePath);
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
        foreach (Primitive primitive, *(m_shapeModel->primitivesList())) {
          curMat = primitive.materialsModel->materialsList()->at(m_ui.paintJobSpinBox->value() - 1);
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
          tr("Error exporting shape resource \"%1\" to Wavefront OBJ file \"%2\": %3").arg(id(), m_currentFilePath, msg));
    }
  }
}

void ShapeResource::importFile()
{
  if (m_currentFilePath.isEmpty()) {
    m_currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);
  }

  QString inFileName = QFileDialog::getOpenFileName(
      this,
      tr("Import shape"),
      m_currentFilePath,
      FILE_FILTERS,
      &m_currentFileFilter);

  if (!inFileName.isEmpty()) {
    Settings().setFilePath(FILE_SETTINGS_PATH, m_currentFilePath = inFileName);

    try {
      QFile objFile(m_currentFilePath);
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

                  primitive.verticesModel = new VerticesModel(faceVertices, m_shapeModel);
                  primitive.materialsModel = new MaterialsModel(material, m_shapeModel);
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

        m_shapeModel->setShape(primitives);
        m_ui.shapeView->reset();

        m_ui.numPaintJobsSpinBox->setValue(1);
        m_ui.paintJobSpinBox->setMaximum(1);

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
          tr("Error importing Wavefront OBJ file \"%1\" to shape resource \"%2\": %3").arg(m_currentFilePath, id(), msg));
    }
  }
}

VerticesList ShapeResource::buildVerticesList(bool boundBox) const
{
  VerticesList vertices;

  if (boundBox) {
    Vertex* bound = m_shapeModel->boundBox();
    for (int i = 0; i < 8; i++) {
      vertices.append(bound[i]);
    }
  }

  foreach (Primitive primitive, *(m_shapeModel->primitivesList())) {
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
  m_ui.shapeView->viewport()->update();
  Resource::isModified();
}
