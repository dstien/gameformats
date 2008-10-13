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
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>

#include "app/settings.h"
#include "bitmapresource.h"

QString BitmapResource::m_currentFilePath;
QString BitmapResource::m_currentFileFilter;

const char BitmapResource::FILE_SETTINGS_PATH[] = "paths/bitmap";
const char BitmapResource::FILE_FILTERS[] =
    "Image files (*.png *.bmp *.gif *.jpg *.jpeg);;"
    "Portable Network Graphics (*.png);;"
    "Windows Bitmap (*.bmp);;"
    "Joint Photographic Experts Group (*.jpg *.jpeg);;"
    "All files (*)";

BitmapResource::BitmapResource(QString id, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags)
{
  m_ui.setupUi(this);

  m_image = 0;
  m_ui.scrollArea->setWidget(new QLabel());

  // disable export
  m_ui.editWidth->setText("0");
  m_ui.editHeight->setText("0");

  m_ui.editUnk1->setText("0000");
  m_ui.editUnk2->setText("0000");
  m_ui.editUnk3->setText("0000");
  m_ui.editUnk4->setText("0000");
  m_ui.editUnk5->setText("01");
  m_ui.editUnk6->setText("02");
  m_ui.editUnk7->setText("04");
  m_ui.editUnk8->setText("08");
}

BitmapResource::BitmapResource(const BitmapResource& res)
: Resource(res.id(), dynamic_cast<QWidget*>(res.parent()), res.windowFlags())
{
  m_ui.setupUi(this);

  m_ui.editWidth->setText(res.m_ui.editWidth->text());
  m_ui.editHeight->setText(res.m_ui.editHeight->text());

  m_ui.editUnk1->setText(res.m_ui.editUnk1->text());
  m_ui.editUnk2->setText(res.m_ui.editUnk2->text());
  m_ui.editUnk3->setText(res.m_ui.editUnk3->text());
  m_ui.editUnk4->setText(res.m_ui.editUnk4->text());
  m_ui.editUnk5->setText(res.m_ui.editUnk5->text());
  m_ui.editUnk6->setText(res.m_ui.editUnk6->text());
  m_ui.editUnk7->setText(res.m_ui.editUnk7->text());
  m_ui.editUnk8->setText(res.m_ui.editUnk8->text());

  m_image = new QImage(*res.m_image);
  m_ui.scrollArea->setWidget(new QLabel());

  m_ui.radioScale1->setChecked(res.m_ui.radioScale1->isChecked());
  m_ui.radioScale2->setChecked(res.m_ui.radioScale2->isChecked());
  m_ui.radioScale4->setChecked(res.m_ui.radioScale4->isChecked());
  m_ui.checkAlpha->setChecked(res.m_ui.checkAlpha->isChecked());
  toggleAlpha(m_ui.checkAlpha->isChecked());
}

BitmapResource::BitmapResource(QString id, QDataStream* in, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags)
{
  m_ui.setupUi(this);

  m_image = 0;
  m_ui.scrollArea->setWidget(new QLabel());

  parse(in);
}

BitmapResource::~BitmapResource()
{
  delete m_image;
}

void BitmapResource::parse(QDataStream* in)
{
  quint16 width, height, unk1, unk2, unk3, unk4;
  quint8 unk5, unk6, unk7, unk8;

  // Header.
  *in >> width >> height;
  *in >> unk1 >> unk2 >> unk3 >> unk4;
  *in >> unk5 >> unk6 >> unk7 >> unk8;
  checkError(in, tr("header"));

  m_ui.editWidth->setText(QString::number(width));
  m_ui.editHeight->setText(QString::number(height));

  m_ui.editUnk1->setText(QString("%1").arg(unk1, 4, 16, QChar('0')).toUpper());
  m_ui.editUnk2->setText(QString("%1").arg(unk2, 4, 16, QChar('0')).toUpper());
  m_ui.editUnk3->setText(QString("%1").arg(unk3, 4, 16, QChar('0')).toUpper());
  m_ui.editUnk4->setText(QString("%1").arg(unk4, 4, 16, QChar('0')).toUpper());
  m_ui.editUnk5->setText(QString("%1").arg(unk5, 2, 16, QChar('0')).toUpper());
  m_ui.editUnk6->setText(QString("%1").arg(unk6, 2, 16, QChar('0')).toUpper());
  m_ui.editUnk7->setText(QString("%1").arg(unk7, 2, 16, QChar('0')).toUpper());
  m_ui.editUnk8->setText(QString("%1").arg(unk8, 2, 16, QChar('0')).toUpper());

  if (width == 0 || height == 0) {
    return;
  }

  // Image data.
  int length = width * height;
  unsigned char* data = 0;

  try {
    try {
      data = new unsigned char[length];
    }
    catch (std::bad_alloc& exc) {
      throw tr("Couldn't allocate memory for image data.");
    }

    if (in->readRawData((char*)data, length) != length) {
      throw tr("Couldn't read image data.");
    }

    // Process data.
    m_image = new QImage(width, height, QImage::Format_Indexed8);
    m_image->setColorTable(Settings::PALETTE);

    int ry = 0;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if ((unk7 & 0x10) == 0x10) {
          m_image->setPixel(x, y, data[(x * height) + y]);
        }
        else if ((unk7 & 0x20) == 0x20) {
          if ((y % 2) == 0) {
            m_image->setPixel(x, y, data[(x * height) + ry]);
          }
          else {
            m_image->setPixel(x, y, data[(height / 2) + (x * height) + ry]);
          }
        }
        else {
          m_image->setPixel(x, y, data[(y * width) + x]);
        }
      }

      if ((y % 2) == 0) {
        ry++;
      }
    }
  }
  catch (QString msg) {
    delete[] data;
    data = 0;

    delete m_image;
    m_image = 0;

    throw msg;
  }

  delete[] data;
  data = 0;

  m_ui.buttonExport->setEnabled(true);
  toggleAlpha(m_ui.checkAlpha->isChecked());
}

void BitmapResource::write(QDataStream* out) const
{
  quint16 unk1, unk2, unk3, unk4;
  quint8  unk5, unk6, unk7, unk8;

  if (m_image) {
    *out << (quint16)m_image->width() << (quint16)m_image->height();
  }
  else {
    *out << (quint16)0 << (quint16)0;
  }

  unk1 = m_ui.editUnk1->text().toUShort(0, 16);
  unk2 = m_ui.editUnk2->text().toUShort(0, 16);
  unk3 = m_ui.editUnk3->text().toUShort(0, 16);
  unk4 = m_ui.editUnk4->text().toUShort(0, 16);
  *out << unk1 << unk2 << unk3 << unk4;

  unk5 = m_ui.editUnk5->text().toUShort(0, 16);
  unk6 = m_ui.editUnk6->text().toUShort(0, 16);
  unk7 = m_ui.editUnk7->text().toUShort(0, 16) & 0xCF;
  unk8 = m_ui.editUnk8->text().toUShort(0, 16);
  *out << unk5 << unk6 << unk7 << unk8;

  checkError(out, tr("header"), true);

  if (m_image) {
    int length = m_image->width() * m_image->height();
    if (out->writeRawData((char*)m_image->bits(), length) != length) {
      throw tr("Couldn't write image data.");
    }
  }
}

void BitmapResource::toggleAlpha(bool alpha)
{
  if (!m_image) {
    return;
  }

  QColor color(m_image->color(ALPHA_INDEX));
  color.setAlpha(alpha ? 0 : 255);
  m_image->setColor(ALPHA_INDEX, color.rgba());

  scale(); // Repaint.
}

void BitmapResource::scale()
{
  QLabel* label = qobject_cast<QLabel*>(m_ui.scrollArea->widget());

  if (!label || !m_image) {
    return;
  }

  if (m_ui.radioScale1->isChecked()) {
    label->setPixmap(QPixmap::fromImage(*m_image));
  }
  else if (m_ui.radioScale2->isChecked()) {
    label->setPixmap(QPixmap::fromImage(m_image->scaled(m_image->width() * 2, m_image->height() * 2)));
  }
  else if (m_ui.radioScale4->isChecked()) {
    label->setPixmap(QPixmap::fromImage(m_image->scaled(m_image->width() * 4, m_image->height() * 4)));
  }

  label->adjustSize();
}

void BitmapResource::exportFile()
{
  if (m_currentFilePath.isEmpty()) {
    m_currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);
  }

  QFileInfo fileInfo(m_currentFilePath);
  fileInfo.setFile(
      fileInfo.absolutePath() +
      QDir::separator() +
      QString("%1-%2").arg(QString(fileName()).replace('.', '_'), id()) +
      ".png");
  m_currentFilePath = fileInfo.absoluteFilePath();

  QString outFileName = QFileDialog::getSaveFileName(
      this,
      tr("Export bitmap"),
      m_currentFilePath,
      FILE_FILTERS,
      &m_currentFileFilter);

  if (!outFileName.isEmpty()) {
    Settings().setFilePath(FILE_SETTINGS_PATH, m_currentFilePath = outFileName);

    QImageWriter writer(m_currentFilePath);
    writer.setText("Comment", QString("Stunts bitmap \"%1\" (%2)").arg(id(), fileName()));

    if (!writer.write(*m_image)) {
      QMessageBox::critical(
          this,
          QCoreApplication::applicationName(),
          tr("Error exporting bitmap resource \"%1\" to image file \"%2\":\n%3").arg(id(), m_currentFilePath, writer.errorString()));
    }
  }
}

void BitmapResource::importFile()
{
  if (m_currentFilePath.isEmpty()) {
    m_currentFilePath = Settings().getFilePath(FILE_SETTINGS_PATH);
  }

  QString inFileName = QFileDialog::getOpenFileName(
      this,
      tr("Import bitmap"),
      m_currentFilePath,
      FILE_FILTERS,
      &m_currentFileFilter);

  if (!inFileName.isEmpty()) {
    Settings().setFilePath(FILE_SETTINGS_PATH, m_currentFilePath = inFileName);

    QImage* newImage = new QImage();
    QImage* oldImage = m_image;

    QImageReader reader(m_currentFilePath);

    try {
      QSize size = reader.size();
      if ((size.width() > MAX_WIDTH) || (size.height() > MAX_HEIGHT)) {
        throw tr("Source file exceeds max dimensions.");
      }

      if (!reader.read(newImage)) {
        throw reader.errorString();
      }

      // Qt will not upsample color space on images with less than 256 colors.
      // We'll have to increase the color depth to 32 bits before downsampling
      // in order to avoid palette corruption.
      if (newImage->numColors() && newImage->numColors() < 256) {
        QImage* tmpImage = newImage;
        newImage =  new QImage(newImage->convertToFormat(QImage::Format_ARGB32));
        delete tmpImage;
      }


      delete oldImage;
      oldImage = 0;

      m_image = new QImage(newImage->convertToFormat(QImage::Format_Indexed8, Settings::PALETTE));

      delete newImage;
      newImage = 0;

      m_ui.editWidth->setText(QString::number(m_image->width()));
      m_ui.editHeight->setText(QString::number(m_image->height()));
      m_ui.editUnk5->setText(QString("%1").arg(1, 2, 16, QChar('0')));
      m_ui.editUnk6->setText(QString("%1").arg(2, 2, 16, QChar('0')));
      m_ui.editUnk7->setText(QString("%1").arg(4, 2, 16, QChar('0')));
      m_ui.editUnk8->setText(QString("%1").arg(8, 2, 16, QChar('0')));

      m_ui.buttonExport->setEnabled(true);

      scale(); // Repaint
      isModified();
    }
    catch (QString msg) {
      delete newImage;
      newImage = 0;

      m_image = oldImage;

      QMessageBox::critical(
          this,
          QCoreApplication::applicationName(),
          tr("Error importing bitmap resource \"%1\" from image file \"%2\":\n%3").arg(id(), m_currentFilePath, msg));
    }
  }
}
