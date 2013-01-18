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

#ifndef SPEEDRESOURCE_H
#define SPEEDRESOURCE_H

#include "app/resource.h"

#include "ui_speedresource.h"

class SpeedResource : public Resource
{
  Q_OBJECT

public:
  SpeedResource(QString id, QWidget* parent = 0, Qt::WindowFlags flags = 0);
  SpeedResource(const SpeedResource& res);
  SpeedResource(QString id, QDataStream* in, QWidget* parent = 0, Qt::WindowFlags flags = 0);

  QString            type() const  { return "speed"; }
  Resource*          clone() const { return new SpeedResource(*this); }

protected:
  void               parse(QDataStream* in);
  void               write(QDataStream* out) const;

private:
  void               setup();

  Ui::SpeedResource  m_ui;

  static const int   NUM_VALUES = 16;
  QSpinBox*          m_spinBoxes[NUM_VALUES];
};

#endif
