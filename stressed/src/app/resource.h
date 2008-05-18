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

#ifndef RESOURCE_H
#define RESOURCE_H

#include <QHash>
#include <QWidget>

class QDataStream;
class QListWidget;
class Resource;

typedef QHash<QString, Resource*> ResMap;

class Resource : public QWidget
{
  Q_OBJECT

public:
  Resource(QString id, QWidget* parent = 0, Qt::WFlags flags = 0);
  virtual ~Resource() {};

  static ResMap     parse(const QString& fileName, QListWidget* idsList);
  static void       write(const QString& fileName, const QListWidget* idsList, const ResMap& resources);

  static QString    fileName()        { return m_fileName; }
  QString           id() const        { return m_id; }
  virtual QString   type() const = 0;

signals:
  void              dataChanged();

protected slots:
  virtual void      isModified();

protected:
  static void       checkError(QDataStream* stream, const QString& what, bool write = false);
  virtual void      parse(QDataStream* in) = 0;
  virtual void      write(QDataStream* out) const = 0;

private:
  QString           m_id;

  static QString    m_fileName;
};

#endif
