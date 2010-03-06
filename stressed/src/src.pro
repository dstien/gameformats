TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = animation bitmap shape text app

qt_version = $$[QT_VERSION]
lessThan(qt_version, 4.2) {
  error(Qt version 4.2 or higher required.)
}
