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

#include <QStringList>

#include "settings.h"

const char Settings::DEFAULTS[] = ":/conf/defaults.conf";

StringMap Settings::getStringMap(const QString& path)
{
  StringMap map;

  beginGroup(path);

  QStringList keys = childKeys();

  if (keys.empty()) {
    endGroup();
    restoreStringMap(path);
    beginGroup(path);

    keys = childKeys();
  }

  foreach(QString key, keys) {
    map[key] = value(key).toString();
  }

  endGroup();

  return map;
}

void Settings::setStringMap(const QString& path, const StringMap& map)
{
  beginGroup(path);

  foreach(QString key, map.keys()) {
    setValue(key, map[key]);
  }

  endGroup();
}

void Settings::restoreStringMap(const QString& path)
{
  StringMap map;
  QSettings defaults(DEFAULTS, IniFormat);

  defaults.beginGroup(path);

  QStringList keys = defaults.childKeys();

  foreach(QString key, keys) {
    map[key] = defaults.value(key).toString();
  }

  defaults.endGroup();

  setStringMap(path, map);
}

Palette Settings::getPalette(const QString& path)
{
  QStringList colorList = value(path).toStringList();

  if (colorList.empty()) {
    return restorePalette(path);
  }

  return parsePalette(colorList);
}

void Settings::setPalette(const QString& path, const Palette& pal)
{
  QStringList colorList;

  foreach (QRgb color, pal) {
    colorList << QColor(color).name().toUpper();
  }

  setValue(path, colorList);
}

Palette Settings::restorePalette(const QString& path)
{
  QSettings defaults(DEFAULTS, IniFormat);

  QStringList colorList = defaults.value(path).toStringList();

  Palette pal = parsePalette(colorList);

  setPalette(path, pal);

  return pal;
}

Palette Settings::parsePalette(const QStringList& colorList)
{
  Palette pal;

  foreach (QString hex, colorList) {
    pal.append(QColor(hex.trimmed()).rgb());
  }

  // Ensure at least 256 colors.
  for (int i = pal.size(); i < 256; i++) {
    pal.append(QColor().rgb());
  }

  return pal;
}

Materials Settings::getMaterials()
{
  QStringList colors = value("materials/colors").toStringList();
  QStringList patterns = value("materials/patterns").toStringList();

  if (colors.empty() | patterns.empty()) {
    return restoreMaterials();
  }

  return parseMaterials(colors, patterns);
}

void Settings::setMaterials(const Materials& materials)
{
  QStringList colors, patterns;

  foreach (Material mat, materials) {
    colors << QString("%1").arg(mat.color);
    patterns << QString("%1").arg(mat.pattern);
  }

  setValue("materials/colors", colors);
  setValue("materials/patterns", patterns);
}

Materials Settings::restoreMaterials()
{
  QSettings defaults(DEFAULTS, IniFormat);

  QStringList colors = defaults.value("materials/colors").toStringList();
  QStringList patterns = defaults.value("materials/patterns").toStringList();

  Materials materials = parseMaterials(colors, patterns);

  setMaterials(materials);

  return materials;
}

Materials Settings::parseMaterials(const QStringList& colors, const QStringList& patterns)
{
  Materials materials;
  for (int i = 0; i < colors.size(); i++) {
    Material mat;
    mat.color = qBound(0, colors[i].toInt(), 255);

    if (i < patterns.size()) {
      mat.pattern = qBound(0, patterns[i].toInt(), 6);
    }
    else {
      mat.pattern = 0;
    }
    materials.append(mat);
  }

  // Ensure at least 256 materials.
  for (int i = materials.size(); i < 256; i++) {
    Material mat;
    mat.color = 0;
    mat.pattern = 0;
    materials.append(mat);
  }

  return materials;
}
