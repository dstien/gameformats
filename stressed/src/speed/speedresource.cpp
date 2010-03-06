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

#include "speedresource.h"

const int SpeedResource::NUM_VALUES;

SpeedResource::SpeedResource(QString id, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags)
{
  setup();
}

SpeedResource::SpeedResource(const SpeedResource& res)
: Resource(res.id(), qobject_cast<QWidget*>(res.parent()), res.windowFlags())
{
  setup();

  for (int i = 0; i < NUM_VALUES; ++i) {
    m_spinBoxes[i]->setValue(res.m_spinBoxes[i]->value());
  }
}

SpeedResource::SpeedResource(QString id, QDataStream* in, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags)
{
  setup();

  parse(in);
}

void SpeedResource::parse(QDataStream* in)
{
  quint8 val;

  for (int i = 0; i < NUM_VALUES; ++i) {
    *in >> val;
    m_spinBoxes[i]->setValue(val);
  }

  checkError(in, tr("opponent speed data"));
}

void SpeedResource::write(QDataStream* out) const
{
  for (int i = 0; i < NUM_VALUES; ++i) {
    *out << (quint8)m_spinBoxes[i]->value();
  }

  checkError(out, tr("opponent speed data"), true);
}

void SpeedResource::setup()
{
  m_ui.setupUi(this);

  m_spinBoxes[ 0] = m_ui.spinBoxPavedRoad;
  m_spinBoxes[ 1] = m_ui.spinBoxDirtRoad;
  m_spinBoxes[ 2] = m_ui.spinBoxIcyRoad;
  m_spinBoxes[ 3] = m_ui.spinBoxPavedSharpCorner;
  m_spinBoxes[ 4] = m_ui.spinBoxDirtSharpCorner;
  m_spinBoxes[ 5] = m_ui.spinBoxIcySharpCorner;
  m_spinBoxes[ 6] = m_ui.spinBoxPavedCorner;
  m_spinBoxes[ 7] = m_ui.spinBoxDirtCorner;
  m_spinBoxes[ 8] = m_ui.spinBoxIcyCorner;
  m_spinBoxes[ 9] = m_ui.spinBoxBankedCorner;
  m_spinBoxes[10] = m_ui.spinBoxBridge;
  m_spinBoxes[11] = m_ui.spinBoxSlalom;
  m_spinBoxes[12] = m_ui.spinBoxCorkUD;
  m_spinBoxes[13] = m_ui.spinBoxChicane;
  m_spinBoxes[14] = m_ui.spinBoxLoop;
  m_spinBoxes[15] = m_ui.spinBoxCorkLR;
}
