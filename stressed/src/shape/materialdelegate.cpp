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

#include <QBitmap>
#include <QComboBox>

#include "app/settings.h"
#include "materialdelegate.h"

Icons MaterialDelegate::m_icons;
bool  MaterialDelegate::m_initialized = false;
const unsigned int MaterialDelegate::NUM_MATERIALS;

MaterialDelegate::MaterialDelegate(QObject *parent)
: QItemDelegate(parent)
{
  if (!m_initialized) {
    setup();
  }
}

QWidget* MaterialDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
  return createComboBox(parent);
}

void MaterialDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QComboBox* materialComboBox = static_cast<QComboBox*>(editor);
  materialComboBox->setCurrentIndex(index.model()->data(index, Qt::EditRole).toInt());
}

void MaterialDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QComboBox* materialComboBox = static_cast<QComboBox*>(editor);
  model->setData(index, materialComboBox->currentIndex());
}

void MaterialDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
  editor->setGeometry(option.rect);
}

const QPixmap& MaterialDelegate::getIcon(unsigned int index)
{
  if (!m_initialized) {
    setup();
  }

  if (index < NUM_MATERIALS) {
    return m_icons[index];
  }
  else {
    return m_icons[0];
  }
}

QComboBox* MaterialDelegate::createComboBox(QWidget* parent)
{
  QComboBox* materialComboBox = new QComboBox(parent);

  for (unsigned int i = 0; i < NUM_MATERIALS; i++) {
    materialComboBox->insertItem(i, tr("%1").arg(i));
    materialComboBox->setItemData(i, m_icons[i], Qt::DecorationRole);
  }

  return materialComboBox;
}

void MaterialDelegate::setup()
{
  for (unsigned int i = 0; i < NUM_MATERIALS; i++) {
    QPixmap icon(16, 16);

    if (Settings::MATERIALS[i].pattern == 1) {
      icon.fill(QColor(Qt::black));
    }
    else {
      icon.fill(QColor(Settings::PALETTE[Settings::MATERIALS[i].color]));
    }

    switch (Settings::MATERIALS[i].pattern) {
      case 1:
        icon.setMask(QBitmap(":/shape/icon_pattern_transparent.png"));
        break;
      case 2:
        icon.setMask(QBitmap(":/shape/icon_pattern_grate.png"));
        break;
      case 3:
        icon.setMask(QBitmap(":/shape/icon_pattern_grille.png"));
        break;
      case 4:
        icon.setMask(QBitmap(":/shape/icon_pattern_grille_inverse.png"));
        break;
      case 5:
        icon.setMask(QBitmap(":/shape/icon_pattern_glass.png"));
        break;
      case 6:
        icon.setMask(QBitmap(":/shape/icon_pattern_glass_inverse.png"));
        break;
    }

    m_icons.append(icon);
  }

  m_initialized = true;
}
