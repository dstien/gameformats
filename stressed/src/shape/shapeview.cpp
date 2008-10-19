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
#include <cmath>

#include "app/settings.h"
#include "materialsmodel.h"
#include "shapeview.h"
#include "verticesmodel.h"

// Convert 8-bit primitive index to unique RGB color used for picking.
#define CODE2COLOR(i) QColor(i & 0xE0, (i & 0x1C) << 3, (i & 0x3) << 6)
#define COLOR2CODE(c) (c[0] | c[1] >> 3 | c[2] >> 6)

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

ShapeView::ShapeView(QWidget* parent)
: QAbstractItemView(parent)
{
  m_shapeModel = 0;
  m_currentPaintJob = 0;
  m_wireframe = false;
  m_showCullData = false;

  m_glWidget = new QGLWidget(this);
  m_glWidget->makeCurrent();
  m_glWidget->qglClearColor(Qt::white);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_FLAT);

  setViewport(m_glWidget);
}

void ShapeView::setModel(QAbstractItemModel* model)
{
  m_shapeModel = qobject_cast<ShapeModel*>(model);

  QAbstractItemView::setModel(model);
}

void ShapeView::reset()
{
  m_translation.reset();
  m_rotation.reset();

  if (m_shapeModel) {
    Vertex* bound = m_shapeModel->boundBox();
    m_translation.move(-((bound[4].y + bound[0].y) / 2), Matrix::AXIS_Y);
    m_translation.move(bound[2].z * 2, Matrix::AXIS_Z);
    m_rotation.rotate(10.0f, Matrix::AXIS_X);
  }

  QAbstractItemView::reset();
}

void ShapeView::setCurrentPaintJob(int paintJob)
{
  m_currentPaintJob = qMax(0, paintJob - 1);
  viewport()->update();
}

void ShapeView::toggleWireframe(bool enable)
{
  m_wireframe = enable;
  glPolygonMode(GL_FRONT_AND_BACK, (enable ? GL_LINE : GL_FILL));
  viewport()->update();
}

void ShapeView::toggleShowCullData(bool enable)
{
  m_showCullData = enable;
  viewport()->update();
}

void ShapeView::updateGeometries()
{
  int width = viewport()->width();
  int height = qMax(1, viewport()->height());

  m_glWidget->makeCurrent();
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 5000.0f);

  glMatrixMode(GL_MODELVIEW);

  QAbstractItemView::updateGeometries();
}

void ShapeView::showEvent(QShowEvent* event)
{
  m_glWidget->makeCurrent();
  QAbstractItemView::showEvent(event);
}

void ShapeView::paintEvent(QPaintEvent* event)
{
  event->accept();

  draw(false);

  m_glWidget->swapBuffers();
}

void ShapeView::draw(bool pick)
{
  if (!m_shapeModel) {
    return;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glPushMatrix();
  m_translation.multMatrix();
  m_rotation.multMatrix();

  int i = 0;
  QItemSelectionModel* selections = selectionModel();

  foreach (Primitive primitive, *m_shapeModel->primitivesList()) {
    VerticesList* verticesList = primitive.verticesModel->verticesList();
    MaterialsList* materialsList = primitive.materialsModel->materialsList();

    int material = materialsList->at(m_currentPaintJob);
    QColor color;

    if (pick) {
      color = CODE2COLOR(i);
    }
    else {
      color = Settings::PALETTE[Settings::MATERIALS[material].color];

      if (selections->isSelected(m_shapeModel->index(i, 0))) {
        if (m_showCullData) {
          drawCullData(primitive);
        }

        color.setRed(qMin(0xFF, color.red() + 0x7F));
        color.setGreen(qMax(0, color.green() - 0x7F));
        color.setBlue(qMax(0, color.blue() - 0x7F));
      }
    }

    m_glWidget->qglColor(color);

    if (Settings::MATERIALS[material].pattern && (Settings::MATERIALS[material].pattern <= 6)) {
      if (Settings::MATERIALS[material].pattern == 1) { // Skip transparent primitives.
        continue;
      }

      glEnable(GL_POLYGON_STIPPLE);
      glPolygonStipple(PATTERNS[Settings::MATERIALS[material].pattern - 2]);
    }

    if (primitive.twoSided | m_wireframe) {
      glDisable(GL_CULL_FACE);
    }

    if (primitive.zBias) {
      glPolygonOffset(-1.0f, -2.0f);
      glEnable(GL_POLYGON_OFFSET_POINT);
      glEnable(GL_POLYGON_OFFSET_LINE);
      glEnable(GL_POLYGON_OFFSET_FILL);
    }

    if (primitive.type == PRIM_TYPE_PARTICLE) {
      glBegin(GL_POINT);
      glVertex3s(verticesList->at(0).x, verticesList->at(0).y, -verticesList->at(0).z);
      glEnd();
    }
    else if (primitive.type == PRIM_TYPE_LINE) {
      glBegin(GL_LINES);
      foreach (Vertex vertex, *verticesList) {
        glVertex3s(vertex.x, vertex.y, -vertex.z);
      }
      glEnd();
    }
    else if (primitive.type > PRIM_TYPE_LINE && primitive.type < PRIM_TYPE_SPHERE) { // Polygon
      glBegin(GL_POLYGON);
      for (int i = verticesList->size() - 1; i >= 0; i--) {
        glVertex3s(verticesList->at(i).x, verticesList->at(i).y, -verticesList->at(i).z);
      }
      glEnd();
    }
    else if (primitive.type == PRIM_TYPE_SPHERE || primitive.type == PRIM_TYPE_WHEEL) {
      glBegin(GL_LINE_STRIP);
      foreach (Vertex vertex, *verticesList) {
        glVertex3s(vertex.x, vertex.y, -vertex.z);
      }
      glEnd();
    }

    if (Settings::MATERIALS[material].pattern) {
      glDisable(GL_POLYGON_STIPPLE);
    }

    if (primitive.zBias) {
      glDisable(GL_POLYGON_OFFSET_POINT);
      glDisable(GL_POLYGON_OFFSET_LINE);
      glDisable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(0.0f, 0.0f);
    }

    if (primitive.twoSided | m_wireframe) {
      glEnable(GL_CULL_FACE);
    }

    i++;
  }

  glPopMatrix();
}

void ShapeView::drawCullData(const Primitive& primitive) const
{
  float radius1 = 40.0f;
  float radius2 = 60.0f;
  float radius3 = 80.0f;
  float x, z;
  static const int steps = 15;
  Vertex center = centroid(primitive);

  if (m_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Horizontal, up
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius2 + center.x, center.y, -center.z);
  glVertex3f(radius3 + center.x, center.y, -center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cullHorizontal & (1 << (steps + steps - j + 1))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkRed);
      else       m_glWidget->qglColor(Qt::red);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkGreen);
      else       m_glWidget->qglColor(Qt::green);
    }
    x = cos((2.0f * M_PI * (j + 1)) / steps);
    z = -sin((2.0f * M_PI * (j + 1)) / steps);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + -center.z);
    glVertex3f(x * radius3 + center.x, center.y, z * radius3 + -center.z);
  }
  glEnd();
  // Horizontal, down
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius2 + center.x, center.y, -center.z);
  glVertex3f(radius3 + center.x, center.y, -center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cullHorizontal & (1 << (j + 2))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkRed);
      else       m_glWidget->qglColor(Qt::red);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkGreen);
      else       m_glWidget->qglColor(Qt::green);
    }
    x = cos((2.0f * M_PI * (j + 1)) / steps);
    z = sin((2.0f * M_PI * (j + 1)) / steps);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + -center.z);
    glVertex3f(x * radius3 + center.x, center.y, z * radius3 + -center.z);
  }
  glEnd();
  // Vertical, up
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius1 + center.x, center.y, -center.z);
  glVertex3f(radius2 + center.x, center.y, -center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cullVertical & (1 << (steps + steps - j + 1))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkMagenta);
      else       m_glWidget->qglColor(Qt::magenta);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkYellow);
      else       m_glWidget->qglColor(Qt::yellow);
    }
    x = cos((2.0f * M_PI * (j + 1)) / steps);
    z = -sin((2.0f * M_PI * (j + 1)) / steps);
    glVertex3f(x * radius1 + center.x, center.y, z * radius1 + -center.z);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + -center.z);
  }
  glEnd();
  // Vertical, down
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius1 + center.x, center.y, -center.z);
  glVertex3f(radius2 + center.x, center.y, -center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cullVertical & (1 << (j + 2))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkMagenta);
      else       m_glWidget->qglColor(Qt::magenta);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkYellow);
      else       m_glWidget->qglColor(Qt::yellow);
    }
    x = cos((2.0f * M_PI * (j + 1)) / steps);
    z = sin((2.0f * M_PI * (j + 1)) / steps);
    glVertex3f(x * radius1 + center.x, center.y, z * radius1 + -center.z);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + -center.z);
  }
  glEnd();

  if (m_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
}

Vertex ShapeView::centroid(const Primitive& primitive) const
{
  Vertex res; // TODO: Use float vertex.
  res.x = res.y = res.z = 0;

  if (primitive.type == PRIM_TYPE_PARTICLE || primitive.type == PRIM_TYPE_SPHERE) {
    return primitive.verticesModel->verticesList()->at(0);
  }
  else if (primitive.type == PRIM_TYPE_LINE) {
    Vertex v1 = primitive.verticesModel->verticesList()->at(0);
    Vertex v2 = primitive.verticesModel->verticesList()->at(1);
    res.x = (v1.x + v2.x) / 2;
    res.y = (v1.y + v2.y) / 2;
    res.z = (v1.z + v2.z) / 2;
  }
  else if (primitive.type > PRIM_TYPE_LINE && primitive.type < PRIM_TYPE_SPHERE) { // Polygon
    foreach (Vertex vertex, *primitive.verticesModel->verticesList()) {
      res.x += vertex.x;
      res.y += vertex.y;
      res.z += vertex.z;
    }
    res.x /= primitive.verticesModel->verticesList()->size();
    res.y /= primitive.verticesModel->verticesList()->size();
    res.z /= primitive.verticesModel->verticesList()->size();
  }
  else if (primitive.type == PRIM_TYPE_WHEEL) {
    Vertex v1 = primitive.verticesModel->verticesList()->at(0);
    Vertex v2 = primitive.verticesModel->verticesList()->at(3);
    res.x = (v1.x + v2.x) / 2;
    res.y = (v1.y + v2.y) / 2;
    res.z = (v1.z + v2.z) / 2;
  }

  return res;
}

int ShapeView::pick()
{
  draw(true);

  GLubyte pixel[3];
  glReadPixels(m_lastMousePosition.x(), viewport()->height() - m_lastMousePosition.y(),
      1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*)pixel);

  return COLOR2CODE(pixel);
}

void ShapeView::mousePressEvent(QMouseEvent* event)
{
  event->accept();
  m_lastMousePosition = event->pos();

  int row = pick();

  if (m_shapeModel && row < ShapeModel::ROWS_MAX) {
    selectionModel()->setCurrentIndex(m_shapeModel->index(row, 0),
        QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
  }
}

void ShapeView::mouseMoveEvent(QMouseEvent* event)
{
  event->accept();
  QPoint delta = event->pos() - m_lastMousePosition;
  m_lastMousePosition = event->pos();

  if ((event->buttons() & Qt::LeftButton) && (event->buttons() & Qt::RightButton)) {
    m_translation.move(delta.y() * 5.0f, Matrix::AXIS_Y);
  }
  else if (event->buttons() & Qt::LeftButton) {
    m_rotation.rotate(-delta.x() * 0.25f, Matrix::AXIS_Y);
    m_rotation.rotate(-delta.y() * 0.25f, Matrix::AXIS_X);
  }
  else if (event->buttons() & Qt::RightButton) {
    m_rotation.rotate(delta.x() * 0.25f, Matrix::AXIS_Z);
  }
  else if (event->buttons() & Qt::MidButton) {
    m_translation.move(delta.x() * 5.0f, Matrix::AXIS_X);
    m_translation.move(-delta.y() * 5.0f, Matrix::AXIS_Z);
  }

  viewport()->update();
}
