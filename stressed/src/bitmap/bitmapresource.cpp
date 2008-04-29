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

#include "bitmapresource.h"

QString BitmapResource::currentFilePath;
QString BitmapResource::currentFileFilter;

const char BitmapResource::FILE_FILTERS[] =
    "Image files (*.png *.bmp *.gif *.jpg *.jpeg);;"
    "Portable Network Graphics (*.png);;"
    "Windows Bitmap (*.bmp);;"
    "Joint Photographic Experts Group (*.jpg *.jpeg);;"
    "All files (*)";

const Palette BitmapResource::PALETTE = Settings().getPalette("palettes/vga");

BitmapResource::BitmapResource(const QString& fileName, QString id, QDataStream* in, QWidget* parent, Qt::WFlags flags)
: Resource(fileName, id, parent, flags)
{
  ui.setupUi(this);

  currentFilePath = fileName;

  image = 0;
  parse(in);
}

BitmapResource::~BitmapResource()
{
  delete image;
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

  ui.editWidth->setText(QString::number(width));
  ui.editHeight->setText(QString::number(height));

  ui.editUnk1->setText(QString("%1").arg(unk1, 4, 16, QChar('0')).toUpper());
  ui.editUnk2->setText(QString("%1").arg(unk2, 4, 16, QChar('0')).toUpper());
  ui.editUnk3->setText(QString("%1").arg(unk3, 4, 16, QChar('0')).toUpper());
  ui.editUnk4->setText(QString("%1").arg(unk4, 4, 16, QChar('0')).toUpper());
  ui.editUnk5->setText(QString("%1").arg(unk5, 2, 16, QChar('0')).toUpper());
  ui.editUnk6->setText(QString("%1").arg(unk6, 2, 16, QChar('0')).toUpper());
  ui.editUnk7->setText(QString("%1").arg(unk7, 2, 16, QChar('0')).toUpper());
  ui.editUnk8->setText(QString("%1").arg(unk8, 2, 16, QChar('0')).toUpper());

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
    image = new QImage(width, height, QImage::Format_Indexed8);
    image->setColorTable(PALETTE);

    int ry = 0;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if ((unk7 & 0x10) == 0x10) {
          image->setPixel(x, y, data[(x * height) + y]);
        }
        else if ((unk7 & 0x20) == 0x20) {
          if ((y % 2) == 0) {
            image->setPixel(x, y, data[(x * height) + ry]);
          }
          else {
            image->setPixel(x, y, data[(height / 2) + (x * height) + ry]);
          }
        }
        else {
          image->setPixel(x, y, data[(y * width) + x]);
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

    delete image;
    image = 0;

    throw msg;
  }

  delete[] data;
  data = 0;

  ui.scrollArea->setWidget(new QLabel());

  toggleAlpha(ui.checkAlpha->isChecked());
}

void BitmapResource::write(QDataStream* out) const
{
  quint16 unk1, unk2, unk3, unk4;
  quint8  unk5, unk6, unk7, unk8;

  *out << (quint16)image->width() << (quint16)image->height();

  unk1 = ui.editUnk1->text().toUShort(0, 16);
  unk2 = ui.editUnk2->text().toUShort(0, 16);
  unk3 = ui.editUnk3->text().toUShort(0, 16);
  unk4 = ui.editUnk4->text().toUShort(0, 16);
  *out << unk1 << unk2 << unk3 << unk4;

  unk5 = ui.editUnk5->text().toUShort(0, 16);
  unk6 = ui.editUnk6->text().toUShort(0, 16);
  unk7 = ui.editUnk7->text().toUShort(0, 16) & 0xCF;
  unk8 = ui.editUnk8->text().toUShort(0, 16);
  *out << unk5 << unk6 << unk7 << unk8;

  checkError(out, tr("header"), true);

  int length = image->width() * image->height();
  if (out->writeRawData((char*)image->bits(), length) != length) {
    throw tr("Couldn't write image data.");
  }
}

void BitmapResource::toggleAlpha(bool alpha)
{
  QColor color(image->color(ALPHA_INDEX));
  color.setAlpha(alpha ? 0 : 255);
  image->setColor(ALPHA_INDEX, color.rgba());

  scale(); // Repaint.
}

void BitmapResource::scale()
{
  QLabel* label = qobject_cast<QLabel*>(ui.scrollArea->widget());

  if (!label) {
    return;
  }

  if (ui.radioScale1->isChecked()) {
    label->setPixmap(QPixmap::fromImage(*image));
  }
  else if (ui.radioScale2->isChecked()) {
    label->setPixmap(QPixmap::fromImage(image->scaled(image->width() * 2, image->height() * 2)));
  }
  else if (ui.radioScale4->isChecked()) {
    label->setPixmap(QPixmap::fromImage(image->scaled(image->width() * 4, image->height() * 4)));
  }

  label->adjustSize();
}

void BitmapResource::exportFile()
{
  currentFilePath = QString("%1%2-%3.png").
      arg(currentDir()).
      arg(QString(fileName()).replace('.', '_')).
      arg(id());

  QString outFileName = QFileDialog::getSaveFileName(
      this,
      tr("Export bitmap"),
      currentFilePath,
      FILE_FILTERS,
      &currentFileFilter);

  if (!outFileName.isEmpty()) {
    currentFilePath = outFileName;

    QImageWriter writer(outFileName);
    writer.setText("Comment", QString("Stunts bitmap \"%1\" (%2)").arg(id(), fileName()));

    if (!writer.write(*image)) {
      QMessageBox::critical(
          this,
          QCoreApplication::applicationName(),
          tr("Error exporting bitmap resource \"%1\" to image file \"%2\":\n%3").arg(id(), outFileName, writer.errorString()));
    }
  }
}

void BitmapResource::importFile()
{
  QString inFileName = QFileDialog::getOpenFileName(
      this,
      tr("Import bitmap"),
      currentDir(),
      FILE_FILTERS,
      &currentFileFilter);

  if (!inFileName.isEmpty()) {
    currentFilePath = inFileName;

    QImage* newImage = new QImage();
    QImage* oldImage = image;

    QImageReader reader(inFileName);

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

      image = new QImage(newImage->convertToFormat(QImage::Format_Indexed8, PALETTE));

      delete newImage;
      newImage = 0;

      ui.editWidth->setText(QString::number(image->width()));
      ui.editHeight->setText(QString::number(image->height()));
      ui.editUnk5->setText(QString("%1").arg(1, 2, 16, QChar('0')));
      ui.editUnk6->setText(QString("%1").arg(2, 2, 16, QChar('0')));
      ui.editUnk7->setText(QString("%1").arg(4, 2, 16, QChar('0')));
      ui.editUnk8->setText(QString("%1").arg(8, 2, 16, QChar('0')));

      scale(); // Repaint
      isModified();
    }
    catch (QString msg) {
      delete newImage;
      newImage = 0;

      image = oldImage;

      QMessageBox::critical(
          this,
          QCoreApplication::applicationName(),
          tr("Error importing bitmap resource \"%1\" from image file \"%2\":\n%3").arg(id(), inFileName, msg));
    }
  }
}

// Get directory from currentFilePath.
QString BitmapResource::currentDir()
{
  return QString(currentFilePath).remove(currentFilePath.lastIndexOf("/") + 1, currentFilePath.length());
}
