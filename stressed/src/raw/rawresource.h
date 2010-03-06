// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2010 Daniel Stien <daniel@stien.org>
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

#ifndef RAWRESOURCE_H
#define RAWRESOURCE_H

#include "app/resource.h"

#include "ui_rawresource.h"

class QLineEdit;

class RawResource : public Resource
{
  Q_OBJECT

public:
  RawResource(QString id, QString type, unsigned int length, QWidget* parent = 0, Qt::WFlags flags = 0);
  RawResource(const RawResource& res);
  RawResource(QString id, QString type, unsigned int length, QDataStream* in, QWidget* parent = 0, Qt::WFlags flags = 0);

  QString           type() const  { return m_type; }
  Resource*         clone() const { return new RawResource(*this); }

  static const int  LENGTH_PATH   = 186;
  static const int  LENGTH_TUNING = 776;

protected:
  void              parse(QDataStream* in);
  void              write(QDataStream* out) const;

private:
  void              setup();

  Ui::RawResource   m_ui;

  QString           m_type;
  unsigned int      m_length;
  quint8*           m_data;
  QLineEdit**       m_lineEdits;
};

#endif
