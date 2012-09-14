#ifndef STATION_H
#define STATION_H

#include <QtCore/QString>
#include <QtCore/QList>

namespace Gds {

class Library;
class Structure;

class Station
{

public:
  Station();

  void setup();
  void tearDown();

  void setActiveLibraryNamed(QString libname);
  void setActiveStructureNamed(QString strucname);

  QList<Library*> libs() { return _libs; }
  Library *library() { return _library; }
  Structure *structure() { return _structure; }

private:
  Library *_library;
  Structure *_structure;
  QList<Library*> _libs;
};

} // namespace Gds

#endif // STATION_H
