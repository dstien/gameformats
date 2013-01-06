// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2012 Daniel Stien <daniel@stien.org>
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

#include <QLineEdit>

#include "rawresource.h"

const int RawResource::LENGTH_PATH;
const int RawResource::LENGTH_TUNING;

RawResource::RawResource(QString id, QString type, unsigned int length, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags),
  m_type(type),
  m_length(length)
{
  setup();

  for (unsigned int i = 0; i < m_length; ++i) {
     m_lineEdits[i]->setText("00");
  }
}

RawResource::RawResource(const RawResource& res)
: Resource(res.id(), qobject_cast<QWidget*>(res.parent()), res.windowFlags()),
  m_type(res.m_type),
  m_length(res.m_length)
{
  setup();

  for (unsigned int i = 0; i < m_length; ++i) {
    m_lineEdits[i]->setText(res.m_lineEdits[i]->text());
  }
}

RawResource::RawResource(QString id, QString type, unsigned int length, QDataStream* in, QWidget* parent, Qt::WFlags flags)
: Resource(id, parent, flags),
  m_type(type),
  m_length(length)
{
  setup();

  parse(in);
}

void RawResource::parse(QDataStream* in)
{
  quint8 val;

  for (unsigned int i = 0; i < m_length; ++i) {
    *in >> val;
    m_lineEdits[i]->setText(QString("%1").arg(val, 2, 16, QLatin1Char('0')).toUpper());
  }

  checkError(in, tr("unknown raw data"));
}

void RawResource::write(QDataStream* out) const
{
  quint8 val;

  for (unsigned int i = 0; i < m_length; ++i) {
    val = m_lineEdits[i]->text().toUShort(0, 16);
    *out << val;
  }

  checkError(out, tr("unknown raw data"), true);
}

void RawResource::setup()
{
  m_ui.setupUi(this);

  const QFont* font = &m_ui.labelCol0->font();

  m_lineEdits = new QLineEdit*[m_length];
  for (unsigned int i = 0; i < m_length; ++i) {
    // Row label.
    if (!(i % 16)) {
      QLabel* lbl = new QLabel(QString("%1").arg(i / 16, 3, 16, QLatin1Char('0')).toUpper().append('0'), this);
      lbl->setFont(*font);
      m_ui.gridLayout->addWidget(lbl, 1 + (i / 16), 0, 1, 1);
    }

    QLineEdit* le = new QLineEdit(this);
    le->setAlignment(Qt::AlignCenter);
    le->setFont(*font);
    le->setInputMask("HH;");
    le->setMaximumSize(QSize(25, 0xFFFFFF));

    m_ui.gridLayout->addWidget(le, 1 + (i / 16), 1 + (i % 16), 1, 1);
    m_lineEdits[i] = le;

    connect(le, SIGNAL(textChanged(QString)), this, SLOT(isModified()));
  }
}
