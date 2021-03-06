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

#include <QBuffer>
#include <QDataStream>
#include <QFileInfo>
#include <QInputDialog>
#include <QListWidget>

#include "animation/animationresource.h"
#include "bitmap/bitmapresource.h"
#include "raw/rawresource.h"
#include "shape/shaperesource.h"
#include "speed/speedresource.h"
#include "text/textresource.h"
#include "resource.h"
#include "resourcesmodel.h"
#include "settings.h"
#include "stunpack.h"

const QStringList Resource::TYPES = (QStringList() << tr("Animation") << tr("Bitmap") << tr("Path") << tr("Shape") << tr("Speed") << tr("Text") << tr("Tuning"));
const QStringList Resource::LOAD_TYPES = (QStringList() << tr("Ignore this resource") << tr("Raw data") << Resource::TYPES);

QString Resource::m_fileName;

Resource::Resource(QString id, QWidget* parent, Qt::WindowFlags flags)
: QWidget(parent, flags),
  m_id(id)
{
}

bool Resource::parse(const QString& fileName, ResourcesModel* resourcesModel, QWidget* parent)
{
  bool modified = false;

  stpk_Buffer compSrc, compDst;
  compSrc.data = compDst.data = NULL;
  QBuffer buf;

  TocList toc;

  quint32 baseOffset;

  QFile file(fileName);

  if (!file.open(QIODevice::ReadOnly)) {
    throw tr("Couldn't open file for reading.");
  }

  QDataStream in(&file);
  in.setByteOrder(QDataStream::LittleEndian);

  try {
    quint64 fileSize = file.size(), actualSize;
    quint32 reportedSize;
    in >> reportedSize;

    // Not a valid resource file, try decompression.
    if (reportedSize != fileSize) {
      quint8 compType = reportedSize & STPK_PASSES_MASK;
      quint32 decompSize = reportedSize >> 8;

      if ((compType >= 1) && (compType <= 2) && (fileSize <= STPK_MAX_SIZE) && (fileSize < decompSize)) {
        compSrc.len = fileSize;
        compSrc.offset = compDst.offset = 0;

        try {
          compSrc.data = new uchar[compSrc.len];
        }
        catch (std::bad_alloc& exc) {
          compSrc.data = NULL;
          throw tr("Couldn't allocate memory for compressed file.");
        }

        file.seek(0);
        if (file.read((char*)compSrc.data, compSrc.len) != compSrc.len) {
          throw tr("Couldn't read compressed data to memory.");
        }

        char errStr[256];
        unsigned int res = stpk_decomp(&compSrc, &compDst, 0, 0, errStr);

        delete[] compSrc.data;
        compSrc.data = NULL;

        if (res) {
          errStr[255] = '\0';
          throw tr("Decompression failed with message \"%1\"").arg(errStr).simplified();
        }

        buf.setData((char*)compDst.data, compDst.len);
        buf.open(QIODevice::ReadOnly);
        in.setDevice(&buf);

        actualSize = compDst.len;
        delete[] compDst.data;
        compDst.data = NULL;
      }
      // Data doesn't fit compression header, give up.
      else {
        throw tr("Invalid file. Reported size (%1) doesn't match actual file size (%2) or compression header.").arg(reportedSize).arg(file.size());
      }
    }
    else {
      file.seek(0);
      actualSize = reportedSize;
    }

    // Decompression done, restart parsing.
    in >> reportedSize;

    if (reportedSize != actualSize) {
      throw tr("Invalid file. Reported size (%1) doesn't match actual file size (%2).").arg(reportedSize).arg(file.size());
    }

    quint16 numResources;
    in >> numResources;

    checkError(&in, tr("header"));

    // Table of contents.
    toc.reserve(numResources);

    char tmpId[5];
    tmpId[4] = '\0';

    for (int i = 0; i < numResources; i++) {
      in.readRawData(tmpId, 4);

      toc.append(TocEntry());
      toc[i].id = tmpId;
      toc[i].pos = i;
    }

    for (int i = 0; i < numResources; i++) {
      in >> toc[i].offset;
    }

    checkError(&in, tr("table of contents"));

    // Base location of resource data.
    baseOffset = in.device()->pos();

    // Sort by offset so that the sequential data positions can be used to guess the size.
    qSort(toc.begin(), toc.end(), tocOffsetLessThan);

    // Set sizes.
    for (int i = 0; i < toc.size(); i++) {
      // Last entry ends at EOF.
      if (i == toc.size() - 1) {
        toc[i].size = in.device()->size() - baseOffset - toc[i].offset;
      }
      // Other entries ends at start of the following resource.
      else {
        toc[i].size = toc[i + 1].offset - toc[i].offset;
      }
    }

    // Restore original order.
    qSort(toc.begin(), toc.end(), tocPositionLessThan);

    // Get type mapping for registered ids.
    StringMap types = Settings().getStringMap("types");
    Resource* resource;
    QString type;
    bool typeOverride = false;

    for (int i = 0; i < numResources; i++) {
      in.device()->seek(baseOffset + toc[i].offset);

      if (!typeOverride) {
        type = types[toc[i].id];
      }
      else {
        typeOverride = false;
      }

      resource = 0;

      try {
        if (type == "text") {
          resource = new TextResource(toc[i].id, &in);
        }
        else if (type == "shape") {
          resource = new ShapeResource(toc[i].id, &in);
        }
        else if (type == "bitmap") {
          resource = new BitmapResource(toc[i].id, &in);
        }
        else if (type == "animation") {
          resource = new AnimationResource(toc[i].id, &in);
        }
        else if (type == "speed") {
          resource = new SpeedResource(toc[i].id, &in);
        }
        else if (type == "path") {
          resource = new RawResource(toc[i].id, "path", RawResource::LENGTH_PATH, &in);
        }
        else if (type == "tuning") {
          resource = new RawResource(toc[i].id, "tuning", RawResource::LENGTH_TUNING, &in);
        }
        else if (type == "raw") {
          resource = new RawResource(toc[i].id, "unknown", toc[i].size, &in);
        }
        else {
          type = tr("unknown");
          throw tr("Unknown type.");
        }
      }
      // Let user cancel/ignore/retry if parsing failed.
      catch (QString msg) {
        in.resetStatus(); // Clear errors for retry/next.

        bool ok;
        QString item = QInputDialog::getItem(parent, tr("Error"),
            tr("Parsing %1 resource \"%2\" failed: %3\n\nCancel, ignore or retry with another type:").arg(type).arg(toc[i].id).arg(msg),
            LOAD_TYPES, 1, false, &ok);

        if (ok && !item.isEmpty()) {
          if (item == tr("Raw data")) {
            type = "raw";
          }
          else if (item == tr("Animation")) {
            type = "animation";
          }
          else if (item == tr("Bitmap")) {
            type = "bitmap";
          }
          else if (item == tr("Path")) {
            type = "path";
          }
          else if (item == tr("Shape")) {
            type = "shape";
          }
          else if (item == tr("Speed")) {
            type = "speed";
          }
          else if (item == tr("Text")) {
            type = "text";
          }
          else if (item == tr("Tuning")) {
            type = "tuning";
          }

          if (item != tr("Ignore this resource")) {
            typeOverride = true;
            i--;
          }
          else {
            modified = true;
          }
        }
        else {
          return false;
        }
      }

      if (resource) {
        resourcesModel->insertRow(resource);
      }
    }

    in.unsetDevice();
    file.close();
  }
  catch (QString msg) {
    in.unsetDevice();
    file.close();

    delete[] compSrc.data;
    delete[] compDst.data;

    throw msg;
  }

  QFileInfo fileInfo(fileName);
  m_fileName = fileInfo.fileName();

  return !modified;
}

bool Resource::tocOffsetLessThan(const TocEntry &e1, const TocEntry &e2)
{
  return e1.offset < e2.offset;
}

bool Resource::tocPositionLessThan(const TocEntry &e1, const TocEntry &e2)
{
  return e1.pos < e2.pos;
}

void Resource::write(const QString& fileName, const ResourcesModel* resourcesModel)
{
  QFile file(fileName);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    throw tr("Couldn't open file for writing.");
  }

  QDataStream out(&file);
  out.setByteOrder(QDataStream::LittleEndian);

  quint16 numResources = resourcesModel->rowCount();

  out << (quint32)0 << numResources;

  checkError(&out, tr("header"), true);

  for (int i = 0; i < numResources; i++) {
    QByteArray id = (QString("%1").arg(resourcesModel->at(i)->id().left(4), 4, '\0')).toLatin1();
    out << (qint8)id[0] << (qint8)id[1] << (qint8)id[2] << (qint8)id[3];
  }

  quint32 tocOffset = out.device()->size();

  // Placeholder offsets.
  for (int i = 0; i < numResources; i++) {
    out << (quint32)0;
  }

  checkError(&out, tr("table of contents"), true);

  quint32 curOffset = 0, baseOffset = out.device()->size();

  for (int i = 0; i < numResources; i++) {
    // Write offset in TOC.
    curOffset = out.device()->size() - baseOffset;
    out.device()->seek(tocOffset + (i * 4));
    out << curOffset;
    out.device()->seek(baseOffset + curOffset);

    Resource* resource = resourcesModel->at(i);

    // Write content.
    try {
      resource->write(&out);
    }
    catch (QString msg) {
      throw tr("Writing %1 resource \"%2\" failed: %3").arg(resource->type()).arg(resource->id()).arg(msg);
    }
  }

  // Final file size header field.
  out.device()->seek(0);
  out << (quint32)out.device()->size();

  checkError(&out, tr("final file size"), true);

  out.unsetDevice();
  file.close();

  QFileInfo fileInfo(fileName);
  m_fileName = fileInfo.fileName();
}

Resource* Resource::typeDialog(QWidget* parent)
{
  bool ok;
  QString item = QInputDialog::getItem(parent, tr("Insert"),
      tr("Select type for new resource:"), TYPES, 0, false, &ok);

  Resource* resource = 0;

  if (ok && !item.isEmpty()) {
    if (item == tr("Animation")) {
      resource = new AnimationResource("anim");
    }
    else if (item == tr("Bitmap")) {
      resource = new BitmapResource("bmap");
    }
    else if (item == tr("Path")) {
      resource = new RawResource("path", "path", RawResource::LENGTH_PATH);
    }
    else if (item == tr("Shape")) {
      resource = new ShapeResource("shpe");
    }
    else if (item == tr("Speed")) {
      resource = new SpeedResource("sped");
    }
    else if (item == tr("Text")) {
      resource = new TextResource("text");
    }
    else if (item == tr("Tuning")) {
      resource = new RawResource("simd", "tuning", RawResource::LENGTH_TUNING);
    }
  }

  return resource;
}

void Resource::isModified()
{
  emit dataChanged();
}

void Resource::checkError(QDataStream* stream, const QString& what, bool write)
{
  QString action = write ? tr("writing") : tr("reading");

  switch (stream->status()) {
    case QDataStream::Ok:
      break;
    case QDataStream::ReadPastEnd:
      throw tr("Reached unexpected end of file while %1 %2.").arg(action).arg(what);
    case QDataStream::ReadCorruptData:
      throw tr("Data corruption occured while %1 %2.").arg(action).arg(what);
    default:
      throw tr("Device error occured while %1 %2 (\"%3\").").arg(action).arg(what).arg(stream->device()->errorString());
  }
}
