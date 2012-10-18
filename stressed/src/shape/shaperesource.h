// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2012 Daniel Stien <daniel@stien.org>
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

#ifndef SHAPERESOURCE_H
#define SHAPERESOURCE_H

#include "app/resource.h"
#include "verticesmodel.h"

#include "ui_shaperesource.h"

class ShapeModel;

class ShapeResource : public Resource
{
  Q_OBJECT

public:
  ShapeResource(QString id, QWidget* parent = 0, Qt::WFlags flags = 0);
  ShapeResource(const ShapeResource& res);
  ShapeResource(QString id, QDataStream* in, QWidget* parent = 0, Qt::WFlags flags = 0);

  QString           type() const       { return "shape"; }
  Resource*         clone() const      { return new ShapeResource(*this); }
  Primitive*        currentPrimitive() { return m_currentPrimitive; }

protected:
  void              parse(QDataStream* in);
  void              write(QDataStream* out) const;

private slots:
  void              deselectAll();
  void              setModels(const QModelIndex& index);
  void              setNumPaintJobs();
  void              toggleWeld(bool enable);

  void              movePrimitives(int direction);
  void              moveFirstPrimitives();
  void              moveUpPrimitives();
  void              moveToPrimitives();
  void              moveDownPrimitives();
  void              moveLastPrimitives();

  void              insertPrimitive();
  void              duplicatePrimitive();
  void              mirrorXPrimitive();
  void              computeCullPrimitives();
  void              removePrimitives();
  void              primitivesContextMenu(const QPoint& pos);

  void              flipVertices();
  void              invertXVertices();
  void              verticesContextMenu(const QPoint& pos);

  void              replaceMaterials();
  void              materialsContextMenu(const QPoint& pos);

  void              exportFile();
  void              importFile();

  void              isModified();

private:
  void              setup();
  void              showEvent(QShowEvent* event);
  VerticesList      buildVerticesList(bool boundBox = false) const;

  Ui::ShapeResource m_ui;

  ShapeModel*       m_shapeModel;
  Primitive*        m_currentPrimitive;

  static QString    m_currentFilePath;
  static QString    m_currentFileFilter;

  static const int  MAX_VERTICES = 256;

  static const char FILE_SETTINGS_PATH[];
  static const char FILE_FILTERS[];
  static const char MTL_SRC[];
  static const char MTL_DST[];

  static const QRegExp OBJ_REGEXP_WHITESPACE;
  static const QRegExp OBJ_REGEXP_VERTEX;
  static const QRegExp OBJ_REGEXP_FACE;
};

#endif
