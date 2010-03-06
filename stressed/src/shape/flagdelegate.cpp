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

#include <QAbstractItemModel>
#include <QApplication>
#include <QMouseEvent>

#include "flagdelegate.h"

FlagDelegate::FlagDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

void FlagDelegate::drawCheck(QPainter* painter, const QStyleOptionViewItem& option, const QRect& /*rect*/, Qt::CheckState state) const
{
  const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
  QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter, check(option, option.rect, Qt::Checked).size(),
      QRect(option.rect.x() + textMargin, option.rect.y(), option.rect.width() - (textMargin * 2), option.rect.height()));

  QItemDelegate::drawCheck(painter, option, checkRect, state);
}

void FlagDelegate::drawFocus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& /*rect*/) const
{
  QItemDelegate::drawFocus(painter, option, option.rect);
}

bool FlagDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  Qt::ItemFlags flags = model->flags(index);
  if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled)) {
    return false;
  }

  QVariant value = index.data(Qt::CheckStateRole);
  if (!value.isValid()) {
    return false;
  }

  if (event->type() == QEvent::MouseButtonRelease) {
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter, check(option, option.rect, Qt::Checked).size(),
        QRect(option.rect.x() + textMargin, option.rect.y(), option.rect.width() - (textMargin * 2), option.rect.height()));

    if (!checkRect.contains(static_cast<QMouseEvent*>(event)->pos())) {
      return false;
    }
  }
  else if (event->type() == QEvent::KeyPress) {
    if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
      return false;
    }
  }
  else {
    return false;
  }

  Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
  return model->setData(index, state, Qt::CheckStateRole);
}
