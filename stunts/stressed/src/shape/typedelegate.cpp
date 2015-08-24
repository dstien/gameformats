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

#include <QComboBox>

#include "shapemodel.h"
#include "typedelegate.h"

TypeDelegate::TypeDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QWidget* TypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
  QComboBox* editor = new QComboBox(parent);
  editor->addItems(ShapeModel::TYPES);

  return editor;
}

void TypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QComboBox* typeComboBox = static_cast<QComboBox*>(editor);
  typeComboBox->setCurrentIndex(index.model()->data(index, Qt::EditRole).toInt() - 1);
}

void TypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QComboBox* typeComboBox = static_cast<QComboBox*>(editor);
  model->setData(index, typeComboBox->currentIndex() + 1);
}

void TypeDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
  editor->setGeometry(option.rect);
}
