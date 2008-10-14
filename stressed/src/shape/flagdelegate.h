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

#ifndef FLAGDELEGATE_H
#define FLAGDELEGATE_H

#include <QItemDelegate>

class FlagDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  FlagDelegate(QObject *parent = 0);

  void              drawCheck(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, Qt::CheckState state) const;
  void              drawDisplay(QPainter* /*painter*/, const QStyleOptionViewItem& /*option*/, const QRect& /*rect*/, const QString& /*text*/) const { }
  void              drawFocus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect) const;
  bool              editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
};

#endif
