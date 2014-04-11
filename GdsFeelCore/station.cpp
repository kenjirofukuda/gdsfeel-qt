#include <QtCore>

#include "station.h"
#include "library.h"

namespace Gds {

Station::Station()
{
  _library = 0;
  _structure = 0;
}

void Station::setup()
{
  _libs = Library::availables();
}

void Station::tearDown()
{
  Library::release(_libs);
}

void Station::setActiveLibraryNamed(QString libname)
{
  foreach (Library* lib, _libs) {
    if (lib->name() == libname) {
      _library = lib;
      return;
    }
  }
  _library = 0;
}


void Station::setActiveStructureNamed(QString strucname)
{
  if (! _library) return;
  _structure = _library->structureNamed(strucname);
}


} // namespace Gds
