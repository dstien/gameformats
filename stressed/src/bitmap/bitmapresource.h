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

#ifndef BITMAPRESOURCE_H
#define BITMAPRESOURCE_H

#include "app/resource.h"

#include "ui_bitmapresource.h"

class BitmapResource : public Resource
{
  Q_OBJECT

public:
  BitmapResource(const BitmapResource& res);
  BitmapResource(QString id, QDataStream* in, QWidget* parent = 0, Qt::WFlags flags = 0);
  ~BitmapResource();

  QString              type() const  { return "bitmap"; }
  Resource*            clone() const { return new BitmapResource(*this); }

protected:
  void                 parse(QDataStream* in);
  void                 write(QDataStream* out) const;

private slots:
  void                 toggleAlpha(bool alpha);
  void                 scale();
  void                 exportFile();
  void                 importFile();

private:
  Ui::BitmapResource   m_ui;

  QImage*              m_image;

  static QString       m_currentFilePath;
  static QString       m_currentFileFilter;

  static const char    FILE_SETTINGS_PATH[];
  static const char    FILE_FILTERS[];

  static const int     MAX_WIDTH   = 0xFFFF;
  static const int     MAX_HEIGHT  = 0xFFFF;
  static const int     ALPHA_INDEX = 0xFF;
};

#endif
