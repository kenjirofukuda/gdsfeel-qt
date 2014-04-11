#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QLibrary>
#include <QtCore/QSettings>

#include "qzipreader_p.h"
#include "qzipwriter_p.h"

#include "library.h"
#include "structure.h"
#include "layer.h"
#include "layers.h"
#include "config.h"


namespace Gds {

const QString LIBRARY_META_FILENAME = "LIB.ini";
const QString LAYERS_FILENAME = "layers.xml";

static void getSubTree(QDir& base, QFileInfoList &infos)
{
  QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot;
  QFileInfoList entries = base.entryInfoList(filters);
  foreach (QFileInfo info , entries) {
    infos.push_back(info);
    if (info.isDir()) {
      QDir newBase(info.filePath());
      getSubTree(newBase, infos);
    }
  }
}

static void getZipMembers(QFileInfo& info,
                          QStringList& names,
                          QString findMember)
{
  QZipReader reader(info.absoluteFilePath(), QIODevice::ReadOnly);
  if (reader.status() != QZipReader::NoError) {
    qDebug() << "open error: " << info.absolutePath();
  }

  QList<QZipReader::FileInfo> list = reader.fileInfoList();
  foreach (QZipReader::FileInfo info, list) {
    names.push_back(info.filePath);
    if (! findMember.isEmpty()) {
      if (info.filePath == findMember) {
        break;
      }
    }
  }
  reader.close();
}


static bool isValidTarget(QFileInfo info)
{
  QStringList members;
  getZipMembers(info, members, LIBRARY_META_FILENAME);
  return members.contains(LIBRARY_META_FILENAME);
}

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------

class LibraryPrivate
{
public:
  LibraryPrivate(const QString& dbPath, Library* library);
  ~LibraryPrivate();

  QString name() const;
  QString nameWithExtension() const;

  bool isOpen() const;
  bool isClose() const;
  QString pathToExtract() const;

  void  tempBackup();
  void  removeExtract();
  void  loadLayers();
  void  loadLibraryMeta();
  void  lookupStructures(Library *library);
  QStringList structureNames();
  Structure *structureNamed(const QString structureName);
  QColor colorForLayerNumber(int layerNumber);

  static QString pathToExtractArea() ;

  QString _dbName;
  QString _unit;
  int _dbu;
  QFileInfo _dbFile;
  Layers _layers;
  Library *_library;

  QMap<QString, Structure*> _structureMap;
};


LibraryPrivate::LibraryPrivate(const QString& dbPath, Library* library)
{
  _dbFile.setFile(dbPath);
  _unit = QString("");
  _dbu = 0;
  _dbName = QString("");
  _library = library;
}


LibraryPrivate::~LibraryPrivate()
{
  _structureMap.clear();
}


QString LibraryPrivate::nameWithExtension() const
{
  return _dbFile.fileName().toUpper();
}


QString LibraryPrivate::name() const
{
  return _dbFile.baseName().toUpper();
}


QString LibraryPrivate::pathToExtractArea()
{
  return QDir(
    Config::pathToSmalltalkProject())
      .absoluteFilePath(QString(".editlibs"));
}


QString LibraryPrivate::pathToExtract() const
{
  return QDir(
    pathToExtractArea())
      .absoluteFilePath(nameWithExtension());
}


void LibraryPrivate::tempBackup()
{
  QString backupPath = QDir::temp().absoluteFilePath(nameWithExtension());
  if (QFile::exists(backupPath)) {
    QFile::remove(backupPath);
  }
  QFile::copy(_dbFile.absoluteFilePath(), backupPath);
}


void LibraryPrivate::removeExtract()
{
  QString from(pathToExtract());
  QDir fromDir(from);
  QFileInfoList founds;

  getSubTree(fromDir, founds);
  foreach (QFileInfo info , founds) {
    if (! info.isDir()) {
      QFile::remove(info.absoluteFilePath());
    }
  }
  foreach (QFileInfo info , founds) {
    if (info.isDir()) {
      QDir dir(info.absoluteFilePath());
      dir.rmdir(info.absoluteFilePath());
    }
  }
  fromDir.rmdir(from);
}

void LibraryPrivate::loadLayers()
{
  QDir dir(pathToExtract());
  Q_ASSERT(dir.exists());
  QString pathToLayers = dir.absoluteFilePath(LAYERS_FILENAME);
  QFileInfo fInfo(pathToLayers);
  if (! fInfo.exists()) return;
  _layers.load(fInfo);
}


void LibraryPrivate::loadLibraryMeta()
{
  QDir dir(pathToExtract());
  Q_ASSERT(dir.exists());
  QString pathToMeta = dir.absoluteFilePath(LIBRARY_META_FILENAME);
  // FIXME:
  // if not found then call fixMetadata();
  // Q_ASSERT(QFile::exists(pathToMeta));
  if (!QFile::exists(pathToMeta)) {
    _dbu = 1000;
    _unit = "MM";
    _dbName = name();
    return;
  }
  QSettings meta(pathToMeta, QSettings::IniFormat);
  meta.beginGroup("INITLIB");
  _dbu = meta.value("dbu", 1000).toInt();
  _unit = meta.value("unit", "MM").toString();
  _dbName = meta.value("name", "").toString();
  meta.endGroup();
  Q_ASSERT(_dbName == name());
}

void LibraryPrivate::lookupStructures(Library *library)
{
  Q_ASSERT(isOpen());

  QString from(pathToExtract());
  QDir fromDir(from);
  QFileInfoList founds;

  getSubTree(fromDir, founds);
  foreach (QFileInfo info, founds) {
    if (info.isFile()) continue;
    if (info.completeSuffix() != "structure") continue;
    Structure *s = new Structure(info);
    s->setParent(library);
    _structureMap[s->name()] = s;
  }
}


QStringList LibraryPrivate::structureNames()
{
  QStringList result;
  foreach (Structure* s, _library->structures()) {
    result.push_back(s->name());
  }
  return result;
}


QColor LibraryPrivate::colorForLayerNumber(int layerNumber)
{
  return _layers.atNumber(layerNumber)->color();
}


Structure *LibraryPrivate::structureNamed(const QString structureName)
{
  if (_structureMap.contains(structureName)) {
    return _structureMap[structureName];
  }
  return 0;
}


bool  LibraryPrivate::isOpen() const
{
  QDir dir(pathToExtract());
  return dir.exists();
}

bool  LibraryPrivate::isClose() const
{
  return ! isOpen();
}


//-----------------------------------------------------------------------------
// class methods
//-----------------------------------------------------------------------------

QFileInfoList Library::files()
{
  QFileInfoList list;
  QDir dir(Config::pathToSmalltalkProject());
  qDebug() << dir.absolutePath();
  QStringList filter("*.DB");
  QFileInfoList all = dir.entryInfoList(filter);

  foreach (QFileInfo info, all) {
    if (isValidTarget(info)) {
      list.append(info);
    }
  }
  return list;
}


void Library::example()
{
  QList<Library*> libs = availables();

  foreach (Library *lib, libs) {
    lib->open();
    lib->close();
  }
  while (!libs.isEmpty())
    delete libs.takeFirst();
}


QList<Library*> Library::availables()
{
  QList<Library*> result;
  QFileInfoList infos = files();

  foreach (QFileInfo info, infos) {
    result.append(new Library(info.absoluteFilePath()));
  }
  return result;
}


void Library::release(QList<Library*> & libs)
{
  while (! libs.isEmpty()) {
    Library *lib = libs.takeFirst();
    lib->close();
    delete lib;
  }
}

//-----------------------------------------------------------------------------
// constructor & destructor
//-----------------------------------------------------------------------------

Library::Library(const QString& dbPath)
{
  p = new LibraryPrivate(dbPath, this);
}

Library::~Library()
{
  delete p;
}


//-----------------------------------------------------------------------------
// instance methods
//-----------------------------------------------------------------------------
void Library::open()
{
  QString at(p->pathToExtract());
  if (isOpen()) {
    qDebug() << "already opend" << at;
    return;
  }
  QZipReader reader(p->_dbFile.absoluteFilePath());
  QDir dir(at);
  if (! dir.mkpath(at)) {
    qDebug() << "Fuck cant create path: " << at;
  }
  qDebug() << "Try extract to: " << at;
  bool success = reader.extractAll(at);
  reader.close();
  Q_ASSERT(success);
  p->loadLibraryMeta();
  p->loadLayers();
  Q_ASSERT(dir.exists());
  p->lookupStructures(this);
}


void Library::close()
{
  QString from(p->pathToExtract());
  if (isClose()) {
    qDebug() << "already closed" << from;
    return;
  }
  QDir fromDir(from);
  QFileInfoList found;

  getSubTree(fromDir, found);
  QZipWriter writer(p->_dbFile.absoluteFilePath());
  foreach (QFileInfo info , found) {
    QString relativePath = fromDir.relativeFilePath(info.filePath());
    if (info.isFile()) {
      qDebug() << "FILE: " << relativePath;
      QFile *ar = new QFile(info.absoluteFilePath());
      writer.addFile(relativePath, ar);
      delete ar;
    }
    else if (info.isDir()) {
      qDebug() << "DIR: " << relativePath;
      writer.addDirectory(relativePath);
    }
    qDebug() << info.absoluteFilePath();
  }
  writer.close();
  p->removeExtract();
  QDir dir(p->pathToExtract());
  Q_ASSERT(! dir.exists());
}

int Library::dbu()
{
  return p->_dbu;
}

QString Library::unit()
{
  return p->_unit;
}

bool  Library::isOpen() const
{
  return p->isOpen();
}

bool  Library::isClose() const
{
  return p->isClose();
}


QString Library::nameWithExtension() const
{
  return p->nameWithExtension();
}


QString Library::name() const
{
  return p->name();
}


QList<Structure*> Library::structures()
{
  if (! isOpen()) {
    open();
  }
  return this->findChildren<Structure *>();
}


QStringList Library::structureNames()
{
  if (! isOpen()) {
    open();
  }
  return p->structureNames();
}


QColor Library::colorForLayerNumber(int layerNumber) const
{
  return p->colorForLayerNumber(layerNumber);
}


Structure *Library::structureNamed(const QString structureName)
{
  if (! isOpen()) {
    open();
  }
  return p->structureNamed(structureName);
}

} // namespace Gds


// vim: sw=2 ts=2 expandtab
