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

#include <QGLWidget>
#include <QPaintEvent>

#include "materialsmodel.h"
#include "shapemodel.h"
#include "shapeview.h"
#include "verticesmodel.h"

const quint8 ShapeView::PATTERNS[5][0x80] = {
  { // Grate
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
    0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC
  },
  { // Grille
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F
  },
  { // Inverse grille
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0
  },
  { // Glass
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF,
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF,
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF,
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF,
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF,
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF,
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF,
    0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0xF3, 0x3C, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF, 0x3C, 0xCF
  },
  { // Inverse glass
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30,
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30,
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30,
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30,
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30,
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30,
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30,
    0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0x0C, 0xC3, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30, 0xC3, 0x30
  }
};

const quint8 ShapeView::MAT_PAT[0x100] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 4, 5, 6, 5, 6, 5, 6, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const QColor ShapeView::MAT_COL[0x100] = {
  "#000000", "#0000A8", "#00A800", "#00A8A8", "#A80000", "#A800A8", "#A85400", "#A8A8A8",
  "#545454", "#5454FC", "#54FC54", "#54FCFC", "#FC5454", "#FC54FC", "#FCFC54", "#FCFCFC",
  "#246820", "#5CFCFC", "#FCFCFC", "#484848", "#383838", "#FCFC54", "#484848", "#202020",
  "#FCFC54", "#9C6438", "#B48054", "#CCA080", "#D8FCFC", "#9CFCFC", "#5CFCFC", "#E4C4AC",
  "#C0906C", "#9C6438", "#9C9CFC", "#FC4040", "#FC7C7C", "#FC40FC", "#383838", "#202020",
  "#C0C0C0", "#00A8A8", "#54FCFC", "#545454", "#000000", "#A80000", "#A80000", "#FC5454",
  "#00007C", "#0000A8", "#0000D0", "#0004FC", "#B40000", "#E40000", "#FC2020", "#FC4040",
  "#545454", "#606060", "#707070", "#7C7C7C", "#E4D800", "#FCF420", "#FCF85C", "#FCFC9C",
  "#009C9C", "#00CCCC", "#00E4E4", "#40FCFC", "#508400", "#74B400", "#90E400", "#A0FC00",
  "#440070", "#60009C", "#8000CC", "#A800FC", "#B0B0B0", "#C0C0C0", "#CCCCCC", "#DCDCDC",
  "#6C5800", "#847400", "#B4A400", "#CCC000", "#700000", "#840000", "#B40000", "#CC0000",
  "#000040", "#280040", "#340058", "#500084", "#383838", "#484848", "#CCCCCC", "#74B400",
  "#FCFCFC", "#A8A8A8", "#9C6438", "#C0602C", "#0080D0", "#84E084", "#70C46C", "#58A858",
  "#509C4C", "#388034", "#DCDCDC", "#B0B0B0", "#905410", "#6C5800", "#580000", "#744008",
  "#700000", "#80542C", "#540054", "#A400A4", "#E000E4", "#FC5CFC", "#000000", "#484848",
  "#2C2C2C", "#FCFCFC", "#B0B0B0", "#FCF85C", "#FCAC54", "#FC0000", "#9C0000", "#FC5454",
  "#DCDCDC", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000",
  "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000", "#000000"
};

ShapeView::ShapeView(QWidget* parent)
: QAbstractItemView(parent)
{
  currentPaintJob = 0;
  transform = Matrix();

  glWidget = new QGLWidget(this);
  glWidget->makeCurrent();
  glWidget->qglClearColor(Qt::white);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  setViewport(glWidget);
}

void ShapeView::setModel(QAbstractItemModel* model)
{
  ShapeModel* shapeModel = qobject_cast<ShapeModel*>(model);

  if (shapeModel) {
    transform.move(-((shapeModel->boundBox()[4].y + shapeModel->boundBox()[0].y) / 2), Matrix::AXIS_Y);
    transform.move(shapeModel->boundBox()[2].z * 2, Matrix::AXIS_Z);
//    transform.rotate(45.0f, Matrix::AXIS_Y);
    transform.rotate(10.0f, Matrix::AXIS_X);
//    transform.rotate(10.0f, Matrix::AXIS_Z);
  }

  QAbstractItemView::setModel(model);
}

void ShapeView::setCurrentPaintJob(int paintJob)
{
  currentPaintJob = qMax(0, paintJob - 1);
  viewport()->update();
}

void ShapeView::toggleWireframe(bool enable)
{
  glPolygonMode(GL_FRONT_AND_BACK, (enable ? GL_LINE : GL_FILL));
  viewport()->update();
}

void ShapeView::toggleCulling(bool enable)
{
  if (enable) {
    glEnable(GL_CULL_FACE);
  }
  else {
    glDisable(GL_CULL_FACE);
  }

  viewport()->update();
}

void ShapeView::updateGeometries()
{
  int width = viewport()->width();
  int height = qMax(1, viewport()->height());

  glWidget->makeCurrent();
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 5000.0f);

  glMatrixMode(GL_MODELVIEW);

  QAbstractItemView::updateGeometries();
}

void ShapeView::showEvent(QShowEvent* event)
{
  glWidget->makeCurrent();
  QAbstractItemView::showEvent(event);
}

void ShapeView::paintEvent(QPaintEvent* event)
{
  event->accept();
  ShapeModel* shapeModel = qobject_cast<ShapeModel*>(model());

  if (!shapeModel) {
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glPushMatrix();
  transform.multMatrix();

  int i = 0;
  QItemSelectionModel* selections = selectionModel();

  foreach (Primitive primitive, *shapeModel->primitivesList()) {
    VerticesList* verticesList = primitive.verticesModel->verticesList();
    MaterialsList* materialsList = primitive.materialsModel->materialsList();

    int material = materialsList->at(currentPaintJob);
    QColor color = MAT_COL[material];

    if (selections->isSelected(shapeModel->index(i, 0))) {
      color.setRed(qMin(0xFF, color.red() + 0x7F));
      color.setGreen(qMax(0, color.green() - 0x7F));
      color.setBlue(qMax(0, color.blue() - 0x7F));
    }

    glWidget->qglColor(color);

    if (MAT_PAT[material] && (MAT_PAT[material] <= 6)) {
      if (MAT_PAT[material] == 1) { // Skip transparent primitives.
        continue;
      }

      glEnable(GL_POLYGON_STIPPLE);
      glPolygonStipple(PATTERNS[MAT_PAT[material] - 2]);
    }

    if (primitive.depthIndex) {
      glPolygonOffset(-1.0f, -primitive.depthIndex);
      glEnable(GL_POLYGON_OFFSET_POINT);
      glEnable(GL_POLYGON_OFFSET_LINE);
      glEnable(GL_POLYGON_OFFSET_FILL);
    }

    // Particle.
    if (primitive.type == 1) {
      glBegin(GL_POINT);
      glVertex3s(verticesList->at(0).x, verticesList->at(0).y, verticesList->at(0).z);
      glEnd();
    }
    // Line.
    else if (primitive.type == 2) {
      glBegin(GL_LINES);
      foreach (Vertex vertex, *verticesList) {
        glVertex3s(vertex.x, vertex.y, vertex.z);
      }
      glEnd();
    }
    // Polygon.
    else if (primitive.type > 2 && primitive.type < 11) {
      glBegin(GL_POLYGON);
      // Draw polygons backwards due to the flipped Z-axis.
      for (int i = verticesList->size(); i > 0; i--) {
        Vertex vertex = verticesList->at(i - 1);
        glVertex3s(vertex.x, vertex.y, vertex.z);
      }
      glEnd();
    }
    // Sphere/wheel.
    else if (primitive.type == 11 | primitive.type == 12) {
      glBegin(GL_LINE_STRIP);
      foreach (Vertex vertex, *verticesList) {
        glVertex3s(vertex.x, vertex.y, vertex.z);
      }
      glEnd();
    }

    if (MAT_PAT[material]) {
      glDisable(GL_POLYGON_STIPPLE);
    }

    if (primitive.depthIndex) {
      glDisable(GL_POLYGON_OFFSET_POINT);
      glDisable(GL_POLYGON_OFFSET_LINE);
      glDisable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0.0f, 0.0f);
    }

    i++;
  }

  glPopMatrix();

  glWidget->swapBuffers();
}

void ShapeView::mousePressEvent(QMouseEvent* event)
{
  event->accept();
  lastMousePosition = event->pos();
}

void ShapeView::mouseMoveEvent(QMouseEvent* event)
{
  event->accept();
  QPoint delta = event->pos() - lastMousePosition;
  lastMousePosition = event->pos();

  if (event->buttons() & Qt::LeftButton) {
    transform.rotate(-delta.x() * 0.25f, Matrix::AXIS_Y);
    transform.rotate(-delta.y() * 0.25f, Matrix::AXIS_X);
  }
  else if (event->buttons() & Qt::RightButton) {
    transform.rotate(delta.x() * 0.25f, Matrix::AXIS_Z);
  }
  else if (event->buttons() & Qt::MidButton) {
    transform.move(delta.y() * 10.0f, Matrix::AXIS_Z);
    transform.move(delta.x() * 10.0f, Matrix::AXIS_X);
  }

  viewport()->update();
}
