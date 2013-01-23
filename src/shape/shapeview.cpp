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

#include <GL/glu.h>
#include <QGLWidget>
#include <QPaintEvent>
#include <QVector3D>
#include <cmath>

#include "app/settings.h"
#include "materialsmodel.h"
#include "shapeview.h"
#include "verticesmodel.h"

// Convert 8-bit primitive index to unique RGB color used for picking.
#define CODE2COLOR(i) QColor(i & 0xE0, (i & 0x1C) << 3, (i & 0x3) << 6)
#define COLOR2CODE(c) (c[0] | c[1] >> 3 | c[2] >> 6)

const quint8 ShapeView::PATTERNS[6][0x80] = {
  { // Transparent
    0x00
  },
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

const float ShapeView::VERTEX_HIGHLIGHT_OFFSET = 20.0f;
const float ShapeView::PI2 = M_PI * 2.0f;
const float ShapeView::SPHERE_RADIUS_RATIO = 2.0f / 3.0f;
const float ShapeView::WHEEL_TYRE_RATIO = 3.0f / 5.0f;

ShapeView::ShapeView(QWidget* parent)
: QAbstractItemView(parent)
{
  m_shapeModel = 0;
  m_currentPaintJob = 0;
  m_wireframe = false;
  m_showCullData = false;
  m_vertexSelection = 0;

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
  m_translation.setToIdentity();
  m_rotation.setToIdentity();

  if (m_shapeModel) {
    Vertex* bound = m_shapeModel->boundBox();
    m_translation.translate(
        0.0f,
        -((bound[4].y + bound[0].y) / 2) * VerticesModel::Y_RATIO,
        0.0f);
    m_translation.translate(
        0.0f,
        0.0f,
        -distance(VerticesModel::toInternal(bound[2]), VerticesModel::toInternal(bound[5])));
    m_rotation.rotate(10.0f, 1.0f, 0.0f, 0.0f);
  }

  QAbstractItemView::reset();
}

void ShapeView::setCurrentPaintJob(int paintJob)
{
  m_currentPaintJob = qMax(0, paintJob - 1);
  viewport()->update();
}

void ShapeView::adjustCurrentPaintJobAfterMove(int oldPosition, int newPosition)
{
  if (m_currentPaintJob == oldPosition) {
    emit selectedPaintJobChangeRequested(newPosition + 1);
  }
  else if (m_currentPaintJob < oldPosition && m_currentPaintJob >= newPosition) {
    emit selectedPaintJobChangeRequested(m_currentPaintJob + 1 + 1);
  }
  else if (m_currentPaintJob > oldPosition && m_currentPaintJob <= newPosition) {
    emit selectedPaintJobChangeRequested(m_currentPaintJob - 1 + 1);
  }
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
  glMultMatrixf(m_translation.constData());
  glMultMatrixf(m_rotation.constData());

  int i = 0;
  QItemSelectionModel* selections = selectionModel();

  foreach (const Primitive& primitive, *m_shapeModel->primitivesList()) {
    VerticesFList* verticesFList = primitive.verticesModel->verticesFList();
    MaterialsList* materialsList = primitive.materialsModel->materialsList();

    int material = materialsList->at(m_currentPaintJob);
    QColor color;

    bool selected = false, pattern = false;

    if (pick) {
      m_glWidget->qglColor(CODE2COLOR(i));
    }
    else {
      if (m_vertexSelection && primitive.verticesModel == m_vertexSelection->model()) {
        foreach (const QModelIndex& index, m_vertexSelection->selectedRows()) {
          drawHighlightedVertex(verticesFList->at(index.row()));
        }
      }

      if (selections->isSelected(m_shapeModel->index(i, 0))) {
        selected = true;

        if (m_showCullData) {
          drawCullData(primitive);
        }
      }
    }

    setMaterial(material, pattern, selected, pick);

    if (primitive.twoSided | m_wireframe) {
      glDisable(GL_CULL_FACE);
    }

    if (primitive.zBias) {
      glDepthRange(0.0f, 1.0f);
    }

    if (primitive.type == PRIM_TYPE_PARTICLE) {
      glBegin(GL_POINT);
      glVertex3f(verticesFList->at(0).x, verticesFList->at(0).y, verticesFList->at(0).z);
      glEnd();
    }
    else if (primitive.type == PRIM_TYPE_LINE) {
      glBegin(GL_LINES);
      foreach (const VertexF& vertex, *verticesFList) {
        glVertex3f(vertex.x, vertex.y, vertex.z);
      }
      glEnd();
    }
    else if (primitive.type > PRIM_TYPE_LINE && primitive.type < PRIM_TYPE_SPHERE) { // Polygon
      glBegin(GL_POLYGON);
      for (int j = verticesFList->size() - 1; j >= 0; j--) {
        glVertex3f(verticesFList->at(j).x, verticesFList->at(j).y, verticesFList->at(j).z);
      }
      glEnd();
    }
    else if (m_wireframe && (primitive.type == PRIM_TYPE_SPHERE || primitive.type == PRIM_TYPE_WHEEL)) {
      glBegin(GL_LINE_STRIP);
      foreach (const VertexF& vertex, *verticesFList) {
        glVertex3f(vertex.x, vertex.y, vertex.z);
      }
      glEnd();
    }
    else if (primitive.type == PRIM_TYPE_SPHERE) {
      drawSphere(verticesFList);
    }
    else if (primitive.type == PRIM_TYPE_WHEEL) {
      drawWheel(verticesFList, material, pattern, selected, pick);
    }

    if (primitive.zBias) {
      glDepthRange(0.025f, 1.0f);
    }

    if (pattern) {
      glDisable(GL_POLYGON_STIPPLE);
    }

    if (primitive.twoSided | m_wireframe) {
      glEnable(GL_CULL_FACE);
    }

    i++;
  }

  glPopMatrix();
}

void ShapeView::drawSphere(const VerticesFList* vertices)
{
  float radius = distance(vertices->at(0), vertices->at(1)) * SPHERE_RADIUS_RATIO;

  glPushMatrix();
  glTranslatef(vertices->at(0).x, vertices->at(0).y, vertices->at(0).z);

  // Billboard face by using inverse shape rotation matrix.
  glMultMatrixf(m_rotation.transposed().constData());

  glBegin(GL_TRIANGLE_FAN);
  for (int j = 0; j < CIRCLE_STEPS; j++) {
    glVertex3f(
        std::cos((PI2 * (j + 1)) / CIRCLE_STEPS) * radius,
        std::sin((PI2 * (j + 1)) / CIRCLE_STEPS) * radius,
        0.0f);
  }
  glEnd();

  glPopMatrix();
}

void ShapeView::drawWheel(const VerticesFList* vertices, int& material, bool& pattern, const bool& selected, const bool& pick)
{
  float radius2h = distance(vertices->at(3), vertices->at(5));
  float radius2v = distance(vertices->at(0), vertices->at(1));
  float radius1h = radius2h * WHEEL_TYRE_RATIO;
  float radius1v = radius2v * WHEEL_TYRE_RATIO;

  VertexF center = centroid(vertices->at(0), vertices->at(3));
  float halfWidth = distance(vertices->at(0), center);

  glPushMatrix();
  glTranslatef(center.x, center.y, center.z);

  // Wheel rotation
  QVector3D edge1 = vertices->at(1).toQ() - vertices->at(0).toQ();
  QVector3D edge2 = vertices->at(2).toQ() - vertices->at(0).toQ();
  QVector3D normal = QVector3D::normal(edge1, edge2);

  float rotation  = std::atan2(normal.x(), normal.z()) * (180.0f / M_PI);
  glRotatef(rotation, 0.0f, 1.0f, 0.0f);

  // X/Y coordinates
  float x1[CIRCLE_STEPS], y1[CIRCLE_STEPS], x2[CIRCLE_STEPS], y2[CIRCLE_STEPS];
  for (int i = 0; i < CIRCLE_STEPS; i++) {
    float x = std::cos((PI2 * (i + 1)) / CIRCLE_STEPS);
    float y = std::sin((PI2 * (i + 1)) / CIRCLE_STEPS);
    x1[i] = x * radius1h;
    y1[i] = y * radius1v;
    x2[i] = x * radius2h;
    y2[i] = y * radius2v;
  }

  // Tyre tread
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius2h, 0.0f, halfWidth);
  glVertex3f(radius2h, 0.0f, -halfWidth);
  for (int i = 0; i < CIRCLE_STEPS; i++) {
    glVertex3f(x2[i], y2[i], halfWidth);
    glVertex3f(x2[i], y2[i], -halfWidth);
  }
  glEnd();

  if (material < MaterialsModel::VAL_MAX) {
    setMaterial(++material, pattern, selected, pick);
  }

  // Inner tyre
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius1h, 0.0f, -halfWidth);
  glVertex3f(radius2h, 0.0f, -halfWidth);
  for (int i = 0; i < CIRCLE_STEPS; i++) {
    glVertex3f(x1[i], -y1[i], -halfWidth);
    glVertex3f(x2[i], -y2[i], -halfWidth);
  }
  glEnd();
  // Outer tyre
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius1h, 0.0f, halfWidth);
  glVertex3f(radius2h, 0.0f, halfWidth);
  for (int i = 0; i < CIRCLE_STEPS; i++) {
    glVertex3f(x1[i], y1[i], halfWidth);
    glVertex3f(x2[i], y2[i], halfWidth);
  }
  glEnd();

  if (material < MaterialsModel::VAL_MAX) {
    setMaterial(++material, pattern, selected, pick);
  }

  // Inner rim
  glBegin(GL_TRIANGLE_FAN);
  for (int i = CIRCLE_STEPS - 1; i >= 0; i--) {
    glVertex3f(x1[i], y1[i], -halfWidth);
  }
  glEnd();
  // Outer rim
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < CIRCLE_STEPS; i++) {
    glVertex3f(x1[i], y1[i], halfWidth);
  }
  glEnd();

  glPopMatrix();
}

void ShapeView::drawHighlightedVertex(const VertexF& vertex)
{
  glDepthRange(0.0f, 1.0f);

  glBegin(GL_LINES);
  m_glWidget->qglColor(Qt::red);
  glVertex3f(vertex.x, vertex.y + VERTEX_HIGHLIGHT_OFFSET, vertex.z);
  glVertex3f(vertex.x, vertex.y - VERTEX_HIGHLIGHT_OFFSET, vertex.z);

  m_glWidget->qglColor(Qt::blue);
  glVertex3f(vertex.x + VERTEX_HIGHLIGHT_OFFSET, vertex.y, vertex.z);
  glVertex3f(vertex.x - VERTEX_HIGHLIGHT_OFFSET, vertex.y, vertex.z);

  m_glWidget->qglColor(Qt::green);
  glVertex3f(vertex.x, vertex.y, vertex.z + VERTEX_HIGHLIGHT_OFFSET);
  glVertex3f(vertex.x, vertex.y, vertex.z - VERTEX_HIGHLIGHT_OFFSET);
  glEnd();

  glDepthRange(0.025f, 1.0f);
}

void ShapeView::drawCullData(const Primitive& primitive)
{
  const float radius1 = 40.0f;
  const float radius2 = 60.0f;
  const float radius3 = 80.0f;
  const int steps = 15;
  float x, z;

  VertexF center = centroid(primitive);

  if (m_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // 1+
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius2 + center.x, center.y, center.z);
  glVertex3f(radius3 + center.x, center.y, center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cull1 & (1 << (steps + steps - j + 1))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkRed);
      else       m_glWidget->qglColor(Qt::red);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkGreen);
      else       m_glWidget->qglColor(Qt::green);
    }
    x =  std::cos((PI2 * (j + 1)) / steps);
    z = -std::sin((PI2 * (j + 1)) / steps);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + center.z);
    glVertex3f(x * radius3 + center.x, center.y, z * radius3 + center.z);
  }
  glEnd();
  // 1-
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius2 + center.x, center.y, center.z);
  glVertex3f(radius3 + center.x, center.y, center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cull1 & (1 << (j + 2))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkRed);
      else       m_glWidget->qglColor(Qt::red);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkGreen);
      else       m_glWidget->qglColor(Qt::green);
    }
    x = std::cos((PI2 * (j + 1)) / steps);
    z = std::sin((PI2 * (j + 1)) / steps);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + center.z);
    glVertex3f(x * radius3 + center.x, center.y, z * radius3 + center.z);
  }
  glEnd();
  // 2+
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius1 + center.x, center.y, center.z);
  glVertex3f(radius2 + center.x, center.y, center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cull2 & (1 << (steps + steps - j + 1))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkMagenta);
      else       m_glWidget->qglColor(Qt::magenta);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkYellow);
      else       m_glWidget->qglColor(Qt::yellow);
    }
    x =  std::cos((PI2 * (j + 1)) / steps);
    z = -std::sin((PI2 * (j + 1)) / steps);
    glVertex3f(x * radius1 + center.x, center.y, z * radius1 + center.z);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + center.z);
  }
  glEnd();
  // 2-
  glBegin(GL_QUAD_STRIP);
  glVertex3f(radius1 + center.x, center.y, center.z);
  glVertex3f(radius2 + center.x, center.y, center.z);
  for (int j = 0; j < steps; j++) {
    if (primitive.cull2 & (1 << (j + 2))) {
      if (j % 2) m_glWidget->qglColor(Qt::darkMagenta);
      else       m_glWidget->qglColor(Qt::magenta);
    }
    else {
      if (j % 2) m_glWidget->qglColor(Qt::darkYellow);
      else       m_glWidget->qglColor(Qt::yellow);
    }
    x = std::cos((PI2 * (j + 1)) / steps);
    z = std::sin((PI2 * (j + 1)) / steps);
    glVertex3f(x * radius1 + center.x, center.y, z * radius1 + center.z);
    glVertex3f(x * radius2 + center.x, center.y, z * radius2 + center.z);
  }
  glEnd();

  if (m_wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
}

void ShapeView::setMaterial(const int& material, bool& pattern, const bool& selected, const bool& pick)
{
  if (!pick) {
    QColor color = Settings::PALETTE[Settings::MATERIALS[material].color];

    if (selected) {
      color.setRed(qMin(0xFF, color.red() + 0x7F));
      color.setGreen(qMax(0, color.green() - 0x7F));
      color.setBlue(qMax(0, color.blue() - 0x7F));
    }

    m_glWidget->qglColor(color);
  }

  if (!m_wireframe) {
    if (Settings::MATERIALS[material].pattern && (Settings::MATERIALS[material].pattern <= 6)) {
      if (!pattern) {
        glEnable(GL_POLYGON_STIPPLE);
      }
      pattern = true;
      glPolygonStipple(PATTERNS[Settings::MATERIALS[material].pattern - 1]);
    }
    else if (pattern) {
      glDisable(GL_POLYGON_STIPPLE);
      pattern = false;
    }
  }
}

int ShapeView::pick()
{
  draw(true);

  GLubyte pixel[3];
  glReadPixels(m_lastMousePosition.x(), viewport()->height() - m_lastMousePosition.y(),
      1, 1, GL_RGB, GL_UNSIGNED_BYTE, (void*)pixel);

  return COLOR2CODE(pixel);
}

VertexF ShapeView::centroid(const Primitive& primitive)
{
  VertexF res;
  res.x = res.y = res.z = 0.0f;

  if (primitive.type == PRIM_TYPE_PARTICLE || primitive.type == PRIM_TYPE_SPHERE) {
    return primitive.verticesModel->verticesFList()->at(0);
  }
  else if (primitive.type == PRIM_TYPE_LINE) {
    return centroid(primitive.verticesModel->verticesFList()->at(0),
        primitive.verticesModel->verticesFList()->at(1));
  }
  else if (primitive.type > PRIM_TYPE_LINE && primitive.type < PRIM_TYPE_SPHERE) { // Polygon
    foreach (VertexF vertex, *primitive.verticesModel->verticesFList()) {
      res.x += vertex.x;
      res.y += vertex.y;
      res.z += vertex.z;
    }
    res.x /= primitive.verticesModel->verticesFList()->size();
    res.y /= primitive.verticesModel->verticesFList()->size();
    res.z /= primitive.verticesModel->verticesFList()->size();
  }
  else if (primitive.type == PRIM_TYPE_WHEEL) {
    return centroid(primitive.verticesModel->verticesFList()->at(0),
        primitive.verticesModel->verticesFList()->at(3));
  }

  return res;
}

VertexF ShapeView::centroid(const VertexF& v1, const VertexF& v2)
{
  VertexF res;
  res.x = res.y = res.z = 0.0f;

  res.x = (v1.x + v2.x) / 2.0f;
  res.y = (v1.y + v2.y) / 2.0f;
  res.z = (v1.z + v2.z) / 2.0f;

  return res;
}

float ShapeView::distance(const VertexF& v1, const VertexF& v2)
{
  float dx = v2.x - v1.x;
  float dy = v2.y - v1.y;
  float dz = v2.z - v1.z;

  return sqrt(dx * dx + dy * dy + dz * dz);
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
    m_translation.translate(0.0f, delta.y() * 5.0f, 0.0f);
  }
  else if (event->buttons() & Qt::LeftButton) {
    m_rotation.rotate(-delta.x() * 0.25f, 0.0f, 1.0f, 0.0f);
    m_rotation.rotate(-delta.y() * 0.25f, 1.0f, 0.0f, 0.0f);
  }
  else if (event->buttons() & Qt::RightButton) {
    m_rotation.rotate(delta.x() * 0.25f, 0.0f, 0.0f, 1.0f);
  }
  else if (event->buttons() & Qt::MidButton) {
    m_translation.translate(delta.x() * 5.0f, 0.0f, -delta.y() * 5.0f);
  }

  viewport()->update();
}
