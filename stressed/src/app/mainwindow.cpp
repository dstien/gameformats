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

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"

const char MainWindow::APP_NAME[] = "stressed";
const char MainWindow::ORG_NAME[] = "stuntstools";

MainWindow::MainWindow(QWidget* parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
  ui.setupUi(this);

  currentResource = NULL;
  modified = false;
  updateWindowTitle();
}

void MainWindow::loadFile(const QString& fileName)
{
  try {
    resources = Resource::parse(fileName, ui.idsList);

    currentFileName = fileName;
    updateWindowTitle();
  }
  catch (QString msg) {
    QMessageBox::critical(
        this,
        APP_NAME,
        tr("Error loading \"%1\":\n%2").arg(fileName, msg));

    reset();
  }

  currentFilePath = fileName;
}

void MainWindow::saveFile(const QString& fileName)
{
  try {
    Resource::write(fileName, ui.idsList, resources);
  }
  catch (QString msg) {
    QMessageBox::critical(
        this,
        APP_NAME,
        tr("Error saving \"%1\":\n%2").arg(fileName, msg));
  }

  currentFileName = fileName;
  currentFilePath = fileName;
  modified = false;
  updateWindowTitle();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  if (reset()) {
    event->accept();
  }
  else {
    event->ignore();
  }
}

bool MainWindow::reset()
{
  if (modified) {
    modified = false;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(
        this,
        APP_NAME,
        tr("The file has been modified.\n"
          "Do you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
      save();
    }
    else if (ret == QMessageBox::Cancel) {
      return false;
    }
  }

  ui.idsList->clear();

  if (currentResource != NULL) {
    ui.hboxLayout->removeWidget(currentResource);
    currentResource->setParent(0);
    currentResource = NULL;
  }

  foreach (QString key, resources.keys()) {
    delete resources[key];
  }

  resources.clear();

  currentFileName.clear();
  updateWindowTitle();

  return true;
}

void MainWindow::open()
{
  if (!reset()) {
    return;
  }

  QString fileName = QFileDialog::getOpenFileName(
      this,
      tr("Open file"),
      currentFilePath,
      tr("All files (*)"),
      &currentFileFilter);

  if (!fileName.isEmpty()) {
    loadFile(fileName);
  }
}

void MainWindow::save()
{
  if (currentFileName.isEmpty()) {
    saveAs();
  }
  else {
    saveFile(currentFileName);
  }
}

void MainWindow::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName(
      this,
      tr("Save file"),
      currentFilePath,
      tr("All files (*)"),
      &currentFileFilter);

  if (!fileName.isEmpty()) {
    saveFile(fileName);
  }
}

void MainWindow::selectionChanged()
{
  if (currentResource != NULL) {
    disconnect(currentResource, SIGNAL(dataChanged()), this, SLOT(isModified()));

    ui.vboxLayout->removeWidget(currentResource);
    currentResource->hide();
    currentResource->setParent(0);
  }

  currentResource = resources[ui.idsList->currentItem()->text()];
  currentResource->setParent(ui.container);
  currentResource->show();
  ui.vboxLayout->addWidget(currentResource);

  connect(currentResource, SIGNAL(dataChanged()), this, SLOT(isModified()));
}

void MainWindow::isModified()
{
  if (!modified) {
    modified = true;
    updateWindowTitle();
  }
}

void MainWindow::updateWindowTitle()
{
  setWindowTitle(
      (modified ? "*" : "") +
      (currentFileName.isEmpty() ? tr("New file") : currentFileName) +
      " - " +
      APP_NAME);
}
