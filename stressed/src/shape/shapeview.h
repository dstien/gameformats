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

#ifndef SHAPEVIEW_H
#define SHAPEVIEW_H

#include <QAbstractItemView>

#include "app/settings.h"
#include "matrix.h"

class ShapeModel;
class QGLWidget;

class ShapeView : public QAbstractItemView
{
  Q_OBJECT

public:
  ShapeView(QWidget* parent = 0);

  void              setModel(QAbstractItemModel* model);
  QRect             visualRect(const QModelIndex& /*index*/) const                      { return viewport()->rect(); }
  void              scrollTo(const QModelIndex& /*index*/, ScrollHint /*hint*/ = EnsureVisible) { }
  QModelIndex       indexAt(const QPoint& /*point*/) const                              { return QModelIndex(); }

public slots:
  void              reset();

protected slots:
  void              setCurrentPaintJob(int paintJob);
  void              toggleWireframe(bool enable);
  void              toggleCulling(bool enable);

protected:
  QModelIndex       moveCursor(CursorAction /*cursorAction*/, Qt::KeyboardModifiers /*modifiers*/) { return QModelIndex(); }

  void              updateGeometries();
  int               horizontalOffset() const                                            { return 0; }
  int               verticalOffset() const                                              { return 0; }

  bool              isIndexHidden(const QModelIndex& /*index*/) const                   { return 0; }

  void              setSelection(const QRect& /*rect*/, QItemSelectionModel::SelectionFlags /*command*/) { }
  QRegion           visualRegionForSelection(const QItemSelection& /*selection*/) const { return QRegion(viewport()->rect()); }

  void              showEvent(QShowEvent* event);
  void              paintEvent(QPaintEvent* event);
  void              mouseMoveEvent(QMouseEvent* event);
  void              mousePressEvent(QMouseEvent* event);

private:
  Palette             palette();
  Materials           materials();

  QGLWidget*          glWidget;
  ShapeModel*         shapeModel;
  QPoint              lastMousePosition;
  Matrix              rotation;
  Matrix              translation;
  int                 currentPaintJob;

  static const quint8 PATTERNS[5][0x80];
};

#endif
