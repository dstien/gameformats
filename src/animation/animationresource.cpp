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

#include "animationresource.h"

AnimationResource::AnimationResource(QString id, QWidget* parent, Qt::WindowFlags flags)
: Resource(id, parent, flags)
{
  m_ui.setupUi(this);
}

AnimationResource::AnimationResource(const AnimationResource& res)
: Resource(res.id(), qobject_cast<QWidget*>(res.parent()), res.windowFlags())
{
  m_ui.setupUi(this);

  m_ui.framesEdit->setPlainText(res.m_ui.framesEdit->toPlainText());
}

AnimationResource::AnimationResource(QString id, QDataStream* in, QWidget* parent, Qt::WindowFlags flags)
: Resource(id, parent, flags)
{
  m_ui.setupUi(this);

  parse(in);
}

// Read NULL-terminated array from QDataStream.
void AnimationResource::parse(QDataStream* in)
{
  QString content;

  qint8 cur;
  *in >> cur;

  while (cur) {
    content.append(QString("%1\n").arg(cur));
    *in >> cur;
  }

  checkError(in, tr("animation sequence indices"));

  m_ui.framesEdit->setPlainText(content);
}

// Write NULL-terminated array to QDataStream.
void AnimationResource::write(QDataStream* out) const
{
  QStringList frames = m_ui.framesEdit->toPlainText().split('\n', QString::SkipEmptyParts);

  foreach (const QString& f, frames) {
    quint8 i = f.toInt();
    *out << (quint8)(i ? i : 1);
  }

  *out << (qint8)0;

  checkError(out, tr("animation sequence indices"), true);
}
