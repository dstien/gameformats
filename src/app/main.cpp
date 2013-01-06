// stressed - Stunts/4D [Sports] Driving resource editor
// Copyright (C) 2008-2012 Daniel Stien <daniel@stien.org>
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

#include <QApplication>

#include "mainwindow.h"
#include "settings.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  app.setOrganizationName(Settings::ORG_NAME);
  app.setApplicationName(Settings::APP_NAME);

  // Scan argument list for filename.
  QString fileName;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      fileName = argv[i];
      break;
    }
  }

  MainWindow mainWindow;
  mainWindow.show();

  // Load file from command line argument.
  if (!fileName.isEmpty()) {
    mainWindow.loadFile(fileName);
  }

  return app.exec();
}
