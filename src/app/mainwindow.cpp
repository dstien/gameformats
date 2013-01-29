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

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QUrl>

#include "mainwindow.h"
#include "resourcesmodel.h"
#include "settings.h"

const char MainWindow::FILE_SETTINGS_PATH[] = "paths/resource";
const char MainWindow::FILE_FILTERS_LOAD[] =
    "All known resource files (*.vsh *.pvs *.esh *.pes *.3sh *.p3s *.vce *.pvc *.kms *.pkm *.sfx *.psf *.res *.pre);;"
    "Bitmaps (*.vsh *.pvs);;"
    "Icons (*.esh *.pes);;"
    "3d shapes (*.3sh *.p3s);;"
    "Voices (*.vce *.pvc);;"
    "Music (*.kms *.pkm);;"
    "Sound effects (*.sfx *.psf);;"
    "Misc (*.res *.pre);;"
    "All files (*)";

const char MainWindow::FILE_FILTERS_SAVE[] =
    "Unpacked resource files (*.vsh *.esh *.3sh *.vce *.kms *.sfx *.res);;"
    "Bitmaps (*.vsh);;"
    "Icons (*.esh);;"
    "3d shapes (*.3sh);;"
    "Voices (*.vce);;"
    "Music (*.kms);;"
    "Sound effects (*.sfx);;"
    "Misc (*.res);;"
    "All files (*)";

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags flags)
: QMainWindow(parent, flags)
{
  m_ui.setupUi(this);

  m_resourcesModel = new ResourcesModel(this);
  m_ui.resourcesView->setModel(m_resourcesModel);

  connect(m_resourcesModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
      this, SLOT(isModified()));
  connect(m_ui.resourcesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
      this, SLOT(setCurrent(QModelIndex)));

  m_currentResource = NULL;
  m_modified = false;
  updateWindowTitle();

  m_statusLabel = new QLabel(m_ui.statusBar);
  m_ui.statusBar->addWidget(m_statusLabel, 1);
  updateStatusBar();
}

void MainWindow::loadFile(const QString& fileName)
{
  Settings().setFilePath(FILE_SETTINGS_PATH, m_currentFilePath = fileName);

  try {
    m_modified = (!Resource::parse(fileName, m_resourcesModel, this));

    m_currentFileName = fileName;
    updateWindowTitle();
    updateStatusBar();
  }
  catch (QString msg) {
    QMessageBox::critical(
        this,
        QCoreApplication::applicationName(),
        tr("Error loading \"%1\":\n%2").arg(fileName, msg));

    reset();
  }
}

void MainWindow::saveFile(const QString& fileName)
{
  try {
    Resource::write(fileName, m_resourcesModel);
  }
  catch (QString msg) {
    QMessageBox::critical(
        this,
        QCoreApplication::applicationName(),
        tr("Error saving \"%1\":\n%2").arg(fileName, msg));
  }

  m_currentFileName = fileName;
  m_modified = false;
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
  if (m_modified) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(
        this,
        QCoreApplication::applicationName(),
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

  if (m_currentResource != NULL) {
    m_ui.hboxLayout->removeWidget(m_currentResource);
    m_currentResource->setParent(0);
    m_currentResource = NULL;
  }

  m_ui.resourcesView->clearSelection();
  m_resourcesModel->clear();

  m_modified = false;
  m_currentFileName.clear();
  updateWindowTitle();
  updateStatusBar();

  return true;
}

void MainWindow::open()
{
  if (!reset()) {
    return;
  }

  if (m_currentFilePath.isEmpty()) {
    m_currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);
  }

  QString fileName = QFileDialog::getOpenFileName(
      this,
      tr("Open file"),
      m_currentFilePath,
      FILE_FILTERS_LOAD,
      &m_currentFileFilter);

  if (!fileName.isEmpty()) {
    loadFile(fileName);
  }
}

void MainWindow::save()
{
  if (m_currentFileName.isEmpty()) {
    saveAs();
  }
  else {
    QString safeName = unpackedPathlessName(m_currentFileName);

    if (safeName != QFileInfo(m_currentFileName).fileName()) {
      if (changeToSafeFileName(safeName)) {
        saveAsAsIs(true);
      }
    }
    else {
      saveFile(m_currentFileName);
    }
  }
}

void MainWindow::saveAs()
{
  QString safeName = unpackedPathlessName(m_currentFileName);

  bool nameWillChange = safeName != QFileInfo(m_currentFileName).fileName();
  if (!nameWillChange || changeToSafeFileName(safeName)) {
    saveAsAsIs(nameWillChange);
  }
}

void MainWindow::saveAsAsIs(bool nameWasChanged)
{
  m_currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);

  QString fileName = QFileDialog::getSaveFileName(
      this,
      tr("Save file"),
      m_currentFilePath,
      FILE_FILTERS_SAVE,
      &m_currentFileFilter);

  if (!fileName.isEmpty()) {
    Settings().setFilePath(FILE_SETTINGS_PATH, m_currentFilePath = fileName);
    saveFile(fileName);

    if (nameWasChanged) {
      QFileInfo fileInfo = QFileInfo(fileName);
      QString extension = fileInfo.suffix().toLower();

      if (extension == "3sh" || extension == "vsh" || extension == "esh")
      {
        QMessageBox::information(
              this,
              QCoreApplication::applicationName(),
              tr("Be sure to rename or move any %1 file from the Stunts "
                 "directory, or the file just saved will not be loaded by "
                 "the game.")
                .arg(fileInfo.completeBaseName() + ".p" + extension.left(2)));
      }
    }
  }
}

bool MainWindow::changeToSafeFileName(const QString& safeFileName)
{
  int ret = QMessageBox::question(
      this,
      QCoreApplication::applicationName(),
      tr("As stressed creates unpacked resource files, "
         "the file name will be changed to %1 by default. "
         "Continue saving?")
        .arg(safeFileName),
      QMessageBox::Ok | QMessageBox::Cancel);


  switch (ret) {
  case QMessageBox::Ok: {
    QDir path = QDir(QFileInfo(m_currentFileName).path());
    Settings().setFilePath(FILE_SETTINGS_PATH,
                           m_currentFilePath = QFileInfo(path, safeFileName).filePath());
    return true;
  }
  default:
    return false;
  }
}

QString MainWindow::unpackedPathlessName(const QString& fileName)
{
  QFileInfo fileInfo = QFileInfo(fileName);
  if (fileName != "")
  {
    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();
    return baseName + "." + unpackedExtension(extension);
  }
  else
  {
    return fileInfo.fileName();
  }
}

QString MainWindow::unpackedExtension(const QString& extension)
{
  QString lowerExtension = extension.toLower();

  if (lowerExtension == "pre") {
    return "res";
  }
  else if (lowerExtension == "pvs") {
    return "vsh";
  }
  else if (lowerExtension == "pes") {
    return "esh";
  }
  else if (lowerExtension == "p3s") {
    return "3sh";
  }
  else if (lowerExtension == "pkm") {
    return "kms";
  }
  else if (lowerExtension == "pvc") {
    return "vce";
  }
  else if (lowerExtension == "psf") {
    return "svx";
  }
  else {
    return extension;
  }
}

void MainWindow::manual()
{
  QDesktopServices::openUrl(QUrl(Settings::MAN_URL));
}

void MainWindow::about()
{
  QMessageBox::about(this, tr("About stressed"),
      tr("<div align=\"center\"><h1>%1 %2</h1><br/>"
         "%3<br/>"
         "Using Qt "QT_VERSION_STR"/%5<br/>"
         "<a href=\"%4\">%4</a><br/><br/>"
         "<em>Thanks to the Stunts community<br>"
         "for keeping the game alive!</em><br><br>"
         "<small>&copy; 2008-2013 <a href=\"mailto:%6\">%7</a><br>"
         "Licensed under the terms of the <a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GNU GPL v2</a>."
         "</small></div>").arg(Settings::APP_NAME, Settings::APP_VER, Settings::APP_DESC, Settings::ORG_URL, qVersion(), Settings::APP_CONTACT, Settings::APP_AUTHOR));
}

void MainWindow::setCurrent(const QModelIndex& index)
{
  if (m_currentResource != NULL) {
    disconnect(m_currentResource, SIGNAL(dataChanged()), this, SLOT(isModified()));

    m_currentResource->hide();
    m_ui.vboxLayout->removeWidget(m_currentResource);
    m_currentResource->setParent(0);
  }

  if (index.isValid())
  {
    m_currentResource = m_resourcesModel->at(index);
    m_currentResource->setParent(m_ui.container);
    m_ui.vboxLayout->addWidget(m_currentResource);
    m_currentResource->show();

    connect(m_currentResource, SIGNAL(dataChanged()), this, SLOT(isModified()));
  }
}

void MainWindow::moveResources(int direction)
{
  if (m_ui.resourcesView->selectionModel()->hasSelection()) {
    disconnect(m_ui.resourcesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(setCurrent(QModelIndex)));
    m_resourcesModel->moveRows(m_ui.resourcesView->selectionModel(), direction);
    connect(m_ui.resourcesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(setCurrent(QModelIndex)));
    isModified();
  }
}

void MainWindow::moveFirstResources()
{
  moveResources(-ResourcesModel::ROWS_MAX);
}

void MainWindow::moveUpResources()
{
  moveResources(-1);
}

void MainWindow::moveDownResources()
{
  moveResources(1);
}

void MainWindow::moveLastResources()
{
  moveResources(ResourcesModel::ROWS_MAX);
}

void MainWindow::sortResources()
{
  m_resourcesModel->sort();
  isModified();
}

void MainWindow::insertResource()
{
  Resource* resource = Resource::typeDialog(this);

  if (resource) {
    int row;
    if (m_ui.resourcesView->currentIndex().isValid()) {
      row = m_ui.resourcesView->currentIndex().row();
    }
    else {
      row = m_resourcesModel->rowCount();
    }

    m_resourcesModel->insertRow(resource, row);
    m_ui.resourcesView->setCurrentIndex(m_resourcesModel->index(row));
    isModified();
    updateStatusBar();
    renameResource();
  }
}

void MainWindow::duplicateResource()
{
  if (m_ui.resourcesView->currentIndex().isValid()) {
    int row = m_ui.resourcesView->currentIndex().row();
    m_resourcesModel->duplicateRow(row);

    m_ui.resourcesView->setCurrentIndex(m_resourcesModel->index(row));
    isModified();
    updateStatusBar();
    renameResource();
  }
}

void MainWindow::renameResource()
{
  m_ui.resourcesView->edit(m_ui.resourcesView->currentIndex());
}

void MainWindow::removeResources()
{
  if (m_ui.resourcesView->selectionModel()->selectedRows().size() >= m_resourcesModel->rowCount()) {
    setCurrent(QModelIndex());
  }

  m_resourcesModel->removeRows(m_ui.resourcesView->selectionModel()->selectedRows());
  isModified();
  updateStatusBar();
}

void MainWindow::resourcesContextMenu(const QPoint& /*pos*/)
{
  if (m_ui.resourcesView->selectionModel()->hasSelection()) {
    m_ui.moveFirstResourcesAction->setEnabled(true);
    m_ui.moveUpResourcesAction->setEnabled(true);
    m_ui.moveDownResourcesAction->setEnabled(true);
    m_ui.moveLastResourcesAction->setEnabled(true);

    m_ui.duplicateResourceAction->setEnabled(true);
    m_ui.renameResourceAction->setEnabled(true);
    m_ui.removeResourcesAction->setEnabled(true);
  }
  else {
    m_ui.moveFirstResourcesAction->setEnabled(false);
    m_ui.moveUpResourcesAction->setEnabled(false);
    m_ui.moveDownResourcesAction->setEnabled(false);
    m_ui.moveLastResourcesAction->setEnabled(false);

    m_ui.duplicateResourceAction->setEnabled(false);
    m_ui.renameResourceAction->setEnabled(false);
    m_ui.removeResourcesAction->setEnabled(false);
  }

  if (m_resourcesModel->rowCount() > 1) {
    m_ui.sortResourcesAction->setEnabled(true);
  }
  else {
    m_ui.sortResourcesAction->setEnabled(false);
  }

  if (m_resourcesModel->rowCount() >= 65535) {
    m_ui.insertResourceAction->setEnabled(false);
    m_ui.duplicateResourceAction->setEnabled(false);
  }
  else {
    m_ui.insertResourceAction->setEnabled(true);
    m_ui.duplicateResourceAction->setEnabled(true);
  }

  m_ui.resourcesMenu->exec(QCursor::pos());
}

void MainWindow::isModified()
{
  if (!m_modified) {
    m_modified = true;
    updateWindowTitle();
  }
}

void MainWindow::updateWindowTitle()
{
  setWindowTitle(
      (m_modified ? "*" : "") +
      (m_currentFileName.isEmpty() ? tr("New file") : m_currentFileName) +
      " - " +
      QCoreApplication::applicationName());
}

void MainWindow::updateStatusBar()
{
  QString msg;
  if (m_resourcesModel->rowCount() == 1) {
    msg.append(tr("1 resource"));
  }
  else {
    msg.append(tr("%1 resources").arg(m_resourcesModel->rowCount()));
  }

  m_statusLabel->setText(msg);
}
