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

#include <QDir>
#include <QStringList>

#include "settings.h"

const char Settings::APP_NAME[]    = "stressed";
const char Settings::APP_VER[]     = "0.2.1";
const char Settings::APP_DESC[]    = "Stunts/4D [Sports] Driving resource editor";
const char Settings::APP_AUTHOR[]  = "Daniel Stien";
const char Settings::APP_CONTACT[] = "daniel@stien.org";

const char Settings::ORG_NAME[] = "stuntstools";
const char Settings::ORG_URL[]  = "https://github.com/dstien/gameformats/tree/master/stunts/stressed";
const char Settings::MAN_URL[]  = "http://wiki.stunts.hu/index.php/Stressed_user_reference";

const char Settings::DEFAULTS[] = ":/conf/defaults.conf";

const char Settings::PATH_MAIN_CONF_VERSION[]  = "main/configVersion";
const char Settings::PATH_MATERIALS_COLORS[]   = "materials/colors";
const char Settings::PATH_MATERIALS_PATTERNS[] = "materials/patterns";
const char Settings::PATH_PALETTES_VGA[]       = "palettes/vga";
const char Settings::PATH_TYPES[]              = "types";
const char Settings::PATH_PATHS_RESOURCE[]     = "paths/resource";

Palette    Settings::m_loadedPalette;
Materials  Settings::m_loadedMaterials;

bool       Settings::m_initialized = false;

Settings::Settings()
: QSettings(IniFormat, UserScope, ORG_NAME, APP_NAME)
{
  if (!m_initialized) {
    mergeDefaults();

    m_loadedPalette = getPalette(PATH_PALETTES_VGA);
    m_loadedMaterials = getMaterials();

    m_initialized = true;
  }
}

void Settings::mergeDefaults()
{
  // Version of bundled default config.
  QSettings defaults(DEFAULTS, IniFormat);
  int targetVersion = defaults.value(PATH_MAIN_CONF_VERSION, -1).toInt();

  // Invalid default config.
  if (targetVersion == -1) {
    return;
  }

  // Version of user config file.
  bool hasUserVersion;
  int userVersion = value(PATH_MAIN_CONF_VERSION, -1).toInt(&hasUserVersion);

  // Current user file is not up to date.
  if (!hasUserVersion || userVersion < targetVersion) {
    // Merge resource ids.

    // List of existing id keys.
    beginGroup(PATH_TYPES);
    QStringList userIds = childKeys();

    // Proceed only if types haven't been set on first usage yet.
    if (!userIds.empty()) {
      // Load existing.
      StringMap types;
      foreach(QString id, userIds) {
        types[id] = value(id).toString();
      }

      // List of defaults.
      defaults.beginGroup(PATH_TYPES);
      QStringList defaultIds = defaults.childKeys();

      // Add type of new ids to existing map.
      foreach(QString id, defaultIds) {
        if (!types.contains(id)) {
          types[id] = defaults.value(id).toString();

        }
      }

      defaults.endGroup();

      // Write result.
      setStringMap("", types);
    }

    endGroup();
    
    // Bump user config version.
    setValue(PATH_MAIN_CONF_VERSION, targetVersion);
  }
}

QString Settings::getFilePath(const QString& path)
{
  QString filePath = value(path).toString();
  if (filePath.isEmpty()) {
    filePath = restoreFilePath(path);
  }

  return filePath;
}

void Settings::setFilePath(const QString& path, const QString& filePath)
{
  setValue(path, filePath);
}

QString Settings::restoreFilePath(const QString& path)
{
  QString filePath = QDir::homePath() + QDir::separator();
  setFilePath(path, filePath);

  return filePath;
}

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
  QStringList colors = value(PATH_MATERIALS_COLORS).toStringList();
  QStringList patterns = value(PATH_MATERIALS_PATTERNS).toStringList();

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

  setValue(PATH_MATERIALS_COLORS, colors);
  setValue(PATH_MATERIALS_PATTERNS, patterns);
}

Materials Settings::restoreMaterials()
{
  QSettings defaults(DEFAULTS, IniFormat);

  QStringList colors = defaults.value(PATH_MATERIALS_COLORS).toStringList();
  QStringList patterns = defaults.value(PATH_MATERIALS_PATTERNS).toStringList();

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
