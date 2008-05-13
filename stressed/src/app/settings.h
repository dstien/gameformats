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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>
#include <QHash>
#include <QSettings>
#include <QVector>

typedef struct {
  int color;
  int pattern;
} Material;

typedef QHash<QString, QString> StringMap;
typedef QVector<QRgb>           Palette;
typedef QList<Material>         Materials;

class Settings : public QSettings
{
  Q_OBJECT

public:
  StringMap         getStringMap(const QString& path);
  void              setStringMap(const QString& path, const StringMap& map);
  void              restoreStringMap(const QString& path);

  Palette           getPalette(const QString& path);
  void              setPalette(const QString& path, const Palette& pal);
  Palette           restorePalette(const QString& path);

  Materials         getMaterials();
  void              setMaterials(const Materials& materials);
  Materials         restoreMaterials();

  static const char APP_NAME[];
  static const char APP_DESC[];
  static const char ORG_NAME[];
  static const char ORG_URL[];

  static const char       DEFAULTS[];
  static const Palette    PALETTE;
  static const Materials  MATERIALS;

private:
  Palette           parsePalette(const QStringList& colorList);
  Materials         parseMaterials(const QStringList& colors, const QStringList& patterns);
};

#endif
