#ifndef LIBRARY_H
#define LIBRARY_H

#include <QtCore/QList>
#include <QtCore/QFileInfo>
#include <QColor>

namespace Gds {

class Structure;
class Layers;
class LibraryPrivate;

class Library : public QObject
{
  Q_OBJECT

public:
  Library(const QString& dbPath);
  ~Library();

  QString name() const;
  QString nameWithExtension() const;
  int dbu();
  QString unit();

  void open();
  void close();

  bool isOpen() const;
  bool isClose() const;

  Structure* structureNamed(const QString  name);
  QList<Structure*> structures();
  QStringList structureNames();
  QColor colorForLayerNumber(int layerNumber) const;

  static QFileInfoList files();
  static QList<Library*> availables();
  static void example();
  static void release(QList<Library*> & libs);

private:
  LibraryPrivate  *p;

};

} // namespace Gds

#endif // LIBRARY_H

// vim : sw=2 ts=2 expandtab
