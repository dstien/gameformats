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

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextStream>

#include "app/settings.h"
#include "flagdelegate.h"
#include "materialdelegate.h"
#include "materialsmodel.h"
#include "shapemodel.h"
#include "shaperesource.h"
#include "typedelegate.h"
#include "verticesmodel.h"

QString       ShapeResource::m_currentFilePath;
QString       ShapeResource::m_currentFileFilter;

const int     ShapeResource::MAX_VERTICES;

const char    ShapeResource::FILE_SETTINGS_PATH[]  = "paths/shape";
const char    ShapeResource::FILE_FILTERS[]        = "Wavefront OBJ (*.obj);;All files (*)";
const char    ShapeResource::MTL_SRC[]             = ":/shape/materials.mtl";
const char    ShapeResource::MTL_DST[]             = "stunts.mtl";

const QRegExp ShapeResource::OBJ_REGEXP_WHITESPACE = QRegExp("\\s+");
const QRegExp ShapeResource::OBJ_REGEXP_VERTEX     = QRegExp("^v(\\s+([+-]?\\d*\\.?\\d*)){3,4}\\s*$");
const QRegExp ShapeResource::OBJ_REGEXP_FACE       = QRegExp("^(fo?|[lp])((\\s+-?\\d+)(/-?\\d*){,2}){1,10}\\s*$");

ShapeResource::ShapeResource(QString id, QWidget* parent, Qt::WindowFlags flags)
: Resource(id, parent, flags)
{
  m_shapeModel = new ShapeModel(this);
  setup();
}

ShapeResource::ShapeResource(const ShapeResource& res)
: Resource(res.id(), dynamic_cast<QWidget*>(res.parent()), res.windowFlags())
{
  m_shapeModel = new ShapeModel(*res.m_shapeModel, this);
  setup();
}

ShapeResource::ShapeResource(QString id, QDataStream* in, QWidget* parent, Qt::WindowFlags flags)
: Resource(id, parent, flags)
{
  m_shapeModel = new ShapeModel(this);
  parse(in);
  setup();
}

void ShapeResource::setup()
{
  m_currentPrimitive = 0;

  m_ui.setupUi(this);

  m_ui.primitivesView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_ui.primitivesView->verticalHeader()->setDefaultSectionSize(20);
  m_ui.primitivesView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

  m_ui.verticesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_ui.materialsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  m_ui.primitivesView->setItemDelegateForColumn(0, new TypeDelegate(m_ui.primitivesView));

  FlagDelegate* flagDelegate = new FlagDelegate(m_ui.primitivesView);
  m_ui.primitivesView->setItemDelegateForColumn(1, flagDelegate);
  m_ui.primitivesView->setItemDelegateForColumn(2, flagDelegate);

  m_ui.primitivesView->setModel(m_shapeModel);
  m_ui.shapeView->setModel(m_shapeModel);
  m_ui.shapeView->setSelectionModel(m_ui.primitivesView->selectionModel());

  m_ui.shapeView->reset();

  m_ui.materialsView->setItemDelegateForColumn(0, new MaterialDelegate(m_ui.materialsView));

  m_ui.numPaintJobsSpinBox->setValue(m_shapeModel->numPaintJobs());
  m_ui.paintJobSpinBox->setMaximum(m_shapeModel->numPaintJobs());

  connect(m_shapeModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
      this, SLOT(isModified()));
  connect(m_shapeModel, SIGNAL(paintJobMoved(int, int)),
      this, SIGNAL(paintJobMoved(int, int)));
  connect(m_ui.primitivesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
      this, SLOT(setModels(QModelIndex)));
}

void ShapeResource::showEvent(QShowEvent* event)
{
  VerticesModel::toggleWeld(m_ui.weldCheckBox->isChecked());
  Resource::showEvent(event);
}

void ShapeResource::parse(QDataStream* in)
{
  PrimitivesList primitives;

  quint8 numVertices, numPrimitives, numPaintJobs, reserved;
  Vertex* vertices = 0;
  quint32* cull1 = 0;
  quint32* cull2 = 0;

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
      cull1 = new quint32[numPrimitives];
      cull2 = new quint32[numPrimitives];
    }
    catch (std::bad_alloc& exc) {
      throw tr("Couldn't allocate memory for shape primitive culling data.");
    }

    in->readRawData(reinterpret_cast<char*>(cull1), numPrimitives * sizeof(quint32));
    checkError(in, tr("primitive culling data 1"));

    in->readRawData(reinterpret_cast<char*>(cull2), numPrimitives * sizeof(quint32));
    checkError(in, tr("primitive culling data 2"));

    quint8 type, flags, material, vertexIndex;
    for (int i = 0; i < numPrimitives; i++) {
      int verticesNeeded;

      MaterialsList materialsList;
      VerticesList verticesList;

      *in >> type >> flags;
      if (flags & ~(PRIM_FLAG_TWOSIDED | PRIM_FLAG_ZBIAS)) {
        throw tr("Unknown flags (0x%1) in primitive %2.").arg(flags, 2, 16, QChar('0')).arg(i);
      }

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
      primitive.twoSided = flags & PRIM_FLAG_TWOSIDED;
      primitive.zBias = flags & PRIM_FLAG_ZBIAS;
      primitive.verticesModel = new VerticesModel(verticesList, m_shapeModel);
      primitive.materialsModel = new MaterialsModel(materialsList, m_shapeModel);

      primitive.cull1 = cull1[i];
      primitive.cull2 = cull2[i];

      primitives.append(primitive);
    }
  }
  catch (QString msg) {
    delete[] vertices;
    vertices = 0;
    delete[] cull1;
    cull1 = 0;
    delete[] cull2;
    cull2 = 0;

    throw msg;
  }

  delete[] vertices;
  vertices = 0;
  delete[] cull1;
  cull1 = 0;
  delete[] cull2;
  cull2 = 0;

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
  foreach (const Vertex& vertex, vertices) {
    *out << vertex.x << vertex.y << vertex.z;
  }
  checkError(out, tr("vertices"), true);

  // Write culling data.
  foreach (const Primitive& primitive, *(m_shapeModel->primitivesList())) {
    *out << primitive.cull1;
  }
  checkError(out, tr("primitive culling data 1"), true);

  foreach (const Primitive& primitive, *(m_shapeModel->primitivesList())) {
    *out << primitive.cull2;
  }
  checkError(out, tr("primitive culling data 2"), true);

  // Write primitives.
  int i = 0;
  quint8 flags;
  foreach (const Primitive& primitive, *(m_shapeModel->primitivesList())) {
    flags = (primitive.twoSided ? PRIM_FLAG_TWOSIDED : 0) | (primitive.zBias ? PRIM_FLAG_ZBIAS : 0);

    // Primitive header.
    *out << primitive.type << flags;
    checkError(out, tr("header for primitive %1").arg(i));

    // Material indices.
    foreach (quint8 material, *(primitive.materialsModel->materialsList())) {
      *out << material;
    }
    checkError(out, tr("material indices in primitive %1").arg(i));

    // Vertex indices.
    foreach (const Vertex& vertex, *(primitive.verticesModel->verticesList())) {
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
  m_currentPrimitive = 0;

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

  m_currentPrimitive = (Primitive*)&m_shapeModel->primitivesList()->at(row);

  m_ui.verticesView->setModel(m_currentPrimitive->verticesModel);
  m_ui.materialsView->setModel(m_currentPrimitive->materialsModel);

  m_ui.shapeView->setCurrentIndex(index);
  m_ui.shapeView->setVertexSelectionModel(m_ui.verticesView->selectionModel());

  connect(m_ui.verticesView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
      m_ui.shapeView, SLOT(selectionChanged(QItemSelection, QItemSelection)));
}

void ShapeResource::setNumPaintJobs()
{
  int num = m_ui.numPaintJobsSpinBox->value();
  if (m_shapeModel->setNumPaintJobs(num)) {
    m_ui.paintJobSpinBox->setMaximum(num);
    isModified();
  }
}

void ShapeResource::toggleWeld(bool enable)
{
  VerticesModel::toggleWeld(enable);
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

void ShapeResource::moveToPrimitives()
{
  bool success;
  int curPosition = m_ui.primitivesView->currentIndex().row();
  int newPosition = QInputDialog::getInt(
      this,
      tr("Move primitive"),
      tr("New position (1 - %1):").arg(m_ui.primitivesView->model()->rowCount()),
      curPosition + 1, 1, m_ui.primitivesView->model()->rowCount(), 1, &success) - 1;

  if (success && (newPosition != curPosition)) {
    movePrimitives(newPosition - curPosition);
  }
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

    m_ui.primitivesView->setCurrentIndex(m_shapeModel->index(row + 1, 0));
    isModified();
  }
}

void ShapeResource::mirrorXPrimitive()
{
  if (m_ui.primitivesView->selectionModel()->hasSelection()) {
    int row = m_ui.primitivesView->currentIndex().row();
    m_shapeModel->mirrorXRow(row);

    m_ui.primitivesView->setCurrentIndex(m_shapeModel->index(row + 1, 0));
    isModified();
  }
}

void ShapeResource::computeCullPrimitives()
{
  m_shapeModel->computeCullRows(m_ui.primitivesView->selectionModel()->selectedRows());
  isModified();
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
    m_ui.moveToPrimitivesAction->setEnabled(true);
    m_ui.moveDownPrimitivesAction->setEnabled(true);
    m_ui.moveLastPrimitivesAction->setEnabled(true);

    m_ui.duplicatePrimitiveAction->setEnabled(true);
    m_ui.mirrorXPrimitiveAction->setEnabled(true);
    m_ui.computeCullPrimitivesAction->setEnabled(true);
    m_ui.removePrimitivesAction->setEnabled(true);
  }
  else {
    m_ui.moveFirstPrimitivesAction->setEnabled(false);
    m_ui.moveUpPrimitivesAction->setEnabled(false);
    m_ui.moveToPrimitivesAction->setEnabled(false);
    m_ui.moveDownPrimitivesAction->setEnabled(false);
    m_ui.moveLastPrimitivesAction->setEnabled(false);

    m_ui.duplicatePrimitiveAction->setEnabled(false);
    m_ui.mirrorXPrimitiveAction->setEnabled(false);
    m_ui.computeCullPrimitivesAction->setEnabled(false);
    m_ui.removePrimitivesAction->setEnabled(false);
  }

  if (m_shapeModel->rowCount() >= 255) {
    m_ui.insertPrimitiveAction->setEnabled(false);
    m_ui.duplicatePrimitiveAction->setEnabled(false);
    m_ui.mirrorXPrimitiveAction->setEnabled(false);
  }
  else {
    m_ui.insertPrimitiveAction->setEnabled(true);
  }

  m_ui.primitivesMenu->exec(QCursor::pos());
}

void ShapeResource::flipVertices()
{
  if (m_ui.verticesView->model()) {
    qobject_cast<VerticesModel*>(m_ui.verticesView->model())->flip();
  }

  isModified();
}

void ShapeResource::invertXVertices()
{
  if (m_ui.verticesView->model()) {
    qobject_cast<VerticesModel*>(m_ui.verticesView->model())->invertX(false);
  }

  isModified();
}

void ShapeResource::verticesContextMenu(const QPoint& /*pos*/)
{
  if (m_ui.verticesView->model()) {
    m_ui.flipVerticesAction->setEnabled(true);
    m_ui.invertXVerticesAction->setEnabled(true);
  }
  else {
    m_ui.flipVerticesAction->setEnabled(false);
    m_ui.invertXVerticesAction->setEnabled(false);
  }

  m_ui.verticesMenu->exec(QCursor::pos());
}

void ShapeResource::replaceMaterials()
{
  if (m_ui.materialsView->model() && m_ui.materialsView->selectionModel()->hasSelection()) {
    int curMaterial = m_ui.materialsView->model()->data(m_ui.materialsView->currentIndex()).toUInt();

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Replace material"));

    QLabel* label = new QLabel(tr("New material:"), &dlg);
    QComboBox* comboBox = MaterialDelegate::createComboBox(&dlg);
    comboBox->setCurrentIndex(curMaterial);
    label->setBuddy(comboBox);

    QVBoxLayout* vbox = new QVBoxLayout(&dlg);
    vbox->addWidget(label);
    vbox->addStretch(1);
    vbox->addWidget(comboBox);
    vbox->addStretch(1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    QPushButton* okButton = static_cast<QPushButton*>(buttonBox->addButton(QDialogButtonBox::Ok));
    okButton->setDefault(true);
    vbox->addWidget(buttonBox);

    QObject::connect(buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));

    if (dlg.exec() == QDialog::Accepted) {
      int newMaterial = comboBox->currentIndex();

      if (newMaterial != curMaterial) {
        m_shapeModel->replaceMaterials(m_ui.materialsView->currentIndex().row(), curMaterial, newMaterial);
        isModified();
      }
    }
  }
}

void ShapeResource::movePaintJobs(int direction)
{
  if (m_ui.materialsView->model()) {
    QItemSelectionModel* materialsSelectionModel = m_ui.materialsView->selectionModel();

    if (materialsSelectionModel->hasSelection()) {
      m_shapeModel->movePaintJobs(materialsSelectionModel, direction);
      isModified();
    }
  }
}

void ShapeResource::moveFirstPaintJobs()
{
  movePaintJobs(-ShapeModel::ROWS_MAX);
}

void ShapeResource::moveUpPaintJobs()
{
  movePaintJobs(-1);
}

void ShapeResource::moveDownPaintJobs()
{
  movePaintJobs(1);
}

void ShapeResource::moveLastPaintJobs()
{
  movePaintJobs(ShapeModel::ROWS_MAX);
}

void ShapeResource::moveToPaintJobs()
{
  bool success;
  int curPosition = m_ui.materialsView->currentIndex().row();
  int newPosition = QInputDialog::getInt(
      this,
      tr("Move paint-jobs"),
      tr("New position (1 - %1):").arg(m_ui.materialsView->model()->rowCount()),
      curPosition + 1, 1, m_ui.materialsView->model()->rowCount(), 1, &success) - 1;

  //Implemented so that the behaviour is consistent with move first/last.
  if (success) {
    moveFirstPaintJobs();
    movePaintJobs(newPosition);
  }
}

void ShapeResource::materialsContextMenu(const QPoint& /*pos*/)
{
  if (m_ui.materialsView->model() && m_ui.materialsView->selectionModel()->hasSelection()) {
    m_ui.replaceMaterialsAction->setEnabled(true);
    m_ui.moveFirstPaintJobsAction->setEnabled(true);
    m_ui.moveUpPaintJobsAction->setEnabled(true);
    m_ui.moveDownPaintJobsAction->setEnabled(true);
    m_ui.moveLastPaintJobsAction->setEnabled(true);
    m_ui.moveToPaintJobsAction->setEnabled(true);
  }
  else {
    m_ui.replaceMaterialsAction->setEnabled(false);
    m_ui.moveFirstPaintJobsAction->setEnabled(false);
    m_ui.moveUpPaintJobsAction->setEnabled(false);
    m_ui.moveDownPaintJobsAction->setEnabled(false);
    m_ui.moveLastPaintJobsAction->setEnabled(false);
    m_ui.moveToPaintJobsAction->setEnabled(false);
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
        foreach (const Vertex& vertex, vertices) {
          out << "v" << qSetFieldWidth(10) << right << fixed << qSetRealNumberPrecision(1)
              << (float)vertex.x << (float)vertex.y << (float)vertex.z << reset << endl;
        }

        out << endl;

        int prevMat = -1, curMat;
        foreach (const Primitive& primitive, *(m_shapeModel->primitivesList())) {
          curMat = primitive.materialsModel->materialsList()->at(m_ui.paintJobSpinBox->value() - 1);
          if (curMat != prevMat) {
            prevMat = curMat;
            out << QString("usemtl Stunts%1").arg(curMat, 3, 10, QChar('0')) << endl;
          }

          switch (primitive.type) {
            case PRIM_TYPE_PARTICLE:
              out << "p";
              break;

            case PRIM_TYPE_LINE:
            case PRIM_TYPE_SPHERE:
              out << "l";
              break;

            default:
              out << "f";
          }

          out << qSetFieldWidth(4) << right;
          foreach (const Vertex& vertex, *(primitive.verticesModel->verticesList())) {
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

            switch (line[0].toLatin1()) {
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

                  if (line[0].toLatin1() == 'l' && numVertices == 6) {
                    primitive.type = PRIM_TYPE_WHEEL;
                  }
                  else {
                    primitive.type = numVertices;
                  }
                  primitive.twoSided = false;
                  primitive.zBias = false;

                  VerticesList faceVertices;
                  for (int i = 0; i < numVertices; i++) {
                    int index = tokens[i + 1].section('/', 0, 0).toInt();

                    if (index < 0) {
                      index = vertices.size() + index + 1;
                    }

                    if (index < 1 || index > vertices.size()) {
                      throw tr("Vertex index %1 out of bounds (1 - %2).").arg(index).arg(vertices.size());
                    }
                    faceVertices.append(vertices[index - 1]);
                  }

                  primitive.verticesModel = new VerticesModel(faceVertices, m_shapeModel);
                  primitive.materialsModel = new MaterialsModel(material, m_shapeModel);
                  m_shapeModel->computeCull(primitive);
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
          foreach (const Primitive& primitive, primitives) {
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

  foreach (const Primitive& primitive, *(m_shapeModel->primitivesList())) {
    foreach (const Vertex& vertex, *(primitive.verticesModel->verticesList())) {
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
