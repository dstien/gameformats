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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

#include "resource.h"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = 0, Qt::WFlags flags = 0);

  void              loadFile(const QString& fileName);

protected:
  void              closeEvent(QCloseEvent* event);

private slots:
  bool              reset();
  void              open();
  void              save();
  void              saveAs();
  void              selectionChanged();
  void              isModified();

private:
  void              saveFile(const QString& fileName);
  void              updateWindowTitle();

  Ui::MainWindow    ui;

  ResMap            resources;
  Resource*         currentResource;

  QString           currentFileName;
  QString           currentFilePath;
  QString           currentFileFilter;

  bool              modified;

  static const char FILE_SETTINGS_PATH[];
  static const char FILE_FILTERS[];
};

#endif
