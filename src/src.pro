TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = animation bitmap raw shape speed text app

qt_version = $$[QT_VERSION]
lessThan(qt_version, 5.0) {
  error(Qt version 5.0 or higher required.)
}
