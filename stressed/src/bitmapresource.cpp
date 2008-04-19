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

#include "bitmapresource.h"
#include "settings.h"

BitmapResource::BitmapResource(QString id, QDataStream* in, QWidget* parent, Qt::WFlags flags) :
  Resource(id, parent, flags)
{
  ui.setupUi(this);

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
  checkError(in, tr("resource \"%1\" bitmap header").arg(id));

  ui.editWidth->insert(QString::number(width));
  ui.editHeight->insert(QString::number(height));

  ui.editUnk1->insert(QString("%1").arg(unk1, 4, 16, QChar('0')).toUpper());
  ui.editUnk2->insert(QString("%1").arg(unk2, 4, 16, QChar('0')).toUpper());
  ui.editUnk3->insert(QString("%1").arg(unk3, 4, 16, QChar('0')).toUpper());
  ui.editUnk4->insert(QString("%1").arg(unk4, 4, 16, QChar('0')).toUpper());
  ui.editUnk5->insert(QString("%1").arg(unk5, 2, 16, QChar('0')).toUpper());
  ui.editUnk6->insert(QString("%1").arg(unk6, 2, 16, QChar('0')).toUpper());
  ui.editUnk7->insert(QString("%1").arg(unk7, 2, 16, QChar('0')).toUpper());
  ui.editUnk8->insert(QString("%1").arg(unk8, 2, 16, QChar('0')).toUpper());

  // Image data.
  int length = width * height;
  unsigned char* data;

  try {
    data = new unsigned char[length];
  }
  catch (std::bad_alloc& exc) {
    throw tr("Couldn't allocate memory for bitmap image data.");
  }

  if (in->readRawData((char*)data, length) != length) {
    throw tr("Reading resource \"%1\" image data failed.");
  }

  // Process data.
  image = new QImage(width, height, QImage::Format_Indexed8);
  image->setColorTable(Settings().getPalette("palettes/vga"));

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

  delete[] data;
  data = NULL;

  QLabel *label = new QLabel;
  label->setPixmap(QPixmap::fromImage(*image));
  ui.scrollArea->setWidget(label);
}

void BitmapResource::write(QDataStream* out) const
{
  quint16 width, height, unk1, unk2, unk3, unk4;
  quint8 unk5, unk6, unk7, unk8;

  width = ui.editWidth->text().toUShort();
  height = ui.editHeight->text().toUShort();
  *out << width << height;

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

  checkError(out, tr("resource \"%1\" bitmap header").arg(id), true);

  if (out->writeRawData((char*)image->bits(), width * height) != (width * height)) {
    throw tr("Writing resource \"%1\" image data failed.");
  }
}
