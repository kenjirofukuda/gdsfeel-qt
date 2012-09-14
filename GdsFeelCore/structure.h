#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <QtCore/QFileInfo>
#include <QtCore/QRectF>

namespace Gds {

class Library;
class Element;

class Structure : public QObject
{
  Q_OBJECT

public:
  Structure(const QFileInfo &storage);
  ~Structure();

  Library *library();

  QString name() const;
  bool isDirty() const;
  void load();
  QList<Element*> elements();
  QRectF dataBounds();

protected:
  void forceLoad();

private:
  QList<int> generationNumbers() const;
  void store();
  QFileInfo currentFile() const;
  QFileInfo layersFileInfo() const;
  void clearGeometryCache();
  void lookupDataBounds(QRectF &bounds);

private:
  QFileInfo _storage;
  QList<int>  _numbers;
  bool _dirty;
  bool _loaded;
  QRectF *_dataBounds;

};

} // namespace Gds

#endif // STRUCTURE_H
