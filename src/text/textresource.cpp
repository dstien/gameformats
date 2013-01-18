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

#include "textresource.h"

TextResource::TextResource(QString id, QWidget* parent, Qt::WindowFlags flags)
: Resource(id, parent, flags)
{
  m_ui.setupUi(this);
}

TextResource::TextResource(const TextResource& res)
: Resource(res.id(), qobject_cast<QWidget*>(res.parent()), res.windowFlags())
{
  m_ui.setupUi(this);

  m_ui.textEdit->setPlainText(res.m_ui.textEdit->toPlainText());
}

TextResource::TextResource(QString id, QDataStream* in, QWidget* parent, Qt::WindowFlags flags)
: Resource(id, parent, flags)
{
  m_ui.setupUi(this);

  parse(in);
}

// Read NULL-terminated C-string from QDataStream.
void TextResource::parse(QDataStream* in)
{
  QString content;

  qint8 cur;
  *in >> cur;

  while (cur) {
    if (cur == ']') {
      cur = '\n';
    }

    content += (char)cur;
    *in >> cur;
  }

  checkError(in, tr("plain text data"));

  m_ui.textEdit->setPlainText(content);
}

// Write NULL-terminated C-string to QDataStream.
void TextResource::write(QDataStream* out) const
{
  QByteArray content = m_ui.textEdit->toPlainText().toLatin1();

  for (int i = 0; i < content.count(); i++) {
    if (content[i] == '\n') {
      content[i] = ']';
    }

    *out << (qint8)content[i];
  }

  *out << (qint8)0;

  checkError(out, tr("plain text data"), true);
}
