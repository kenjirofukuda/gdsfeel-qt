#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QPointF>
#include <QtCore/QStringList>
#include <QtXml>
#include <QDomDocument>

#include "structure.h"
#include "library.h"
#include "element.h"

namespace Gds {


Structure::Structure(const QFileInfo &storage)
{
  Q_ASSERT(storage.isDir());
  _storage = storage;
  _numbers = generationNumbers();
  _dirty = false;
  _loaded = false;
  _dataBounds = 0;
}


Structure::~Structure()
{
  clearGeometryCache();
}


Library *Structure::library()
{
  return qobject_cast<Library *>(parent());
}


QFileInfo Structure::currentFile() const
{
  int maxNumber = _numbers.last();
  QString num;
  num.setNum(maxNumber);
  QStringList items;
  items << name() << num << "gdsfeelbeta";
  QDir dir(_storage.absoluteFilePath());
  return QFileInfo(dir.absoluteFilePath(items.join(".")));
}


QFileInfo Structure::layersFileInfo() const
{
  QDir dir(_storage.absoluteFilePath());
  return QFileInfo(dir.absoluteFilePath("layers.xml"));
}


QList<int> Structure::generationNumbers() const
{
  QList<int>  numbers;
  QDir dir(_storage.absoluteFilePath());
  QStringList filters;
  filters << "*.*.gdsfeelbeta";
  QStringList names = dir.entryList(filters, QDir::Files);
  foreach (QString name, names) {
    QString genStr = name.split(".").at(1);
    bool ok;
    int num = genStr.toInt(&ok);
    if (ok)
      numbers.push_back(num);
  }
  qSort(numbers);
  return numbers;
}


QString Structure::name() const
{
  return _storage.completeBaseName().toUpper();
}


QList<Element*> Structure::elements()
{
  load();
  return this->findChildren<Element*>();
}


void Structure::clearGeometryCache()
{
  qFree(_dataBounds);
  _dataBounds = 0;
}


const qreal MAX_VAL = 32767;

QRectF Structure::dataBounds()
{
  // FIXME: duplicate implement Element
  if (_dataBounds == 0) {
    _dataBounds = new QRectF;
    Element::resetToSmallBounds(*_dataBounds);
    lookupDataBounds(*_dataBounds);
  }
  return *_dataBounds;
}


void Structure::lookupDataBounds(QRectF &bounds)
{
  qreal xmin = MAX_VAL;
  qreal xmax = -MAX_VAL;
  qreal ymin = MAX_VAL;
  qreal ymax = -MAX_VAL;
  foreach (Element *e, elements()) {
    QList<QPointF> points;
    Element::calcOutlinePoints(e->dataBounds(), points);
    foreach (QPointF p, points) {
      if (p.x() < xmin) xmin = p.x();
      if (p.x() > xmax) xmax = p.x();
      if (p.y() < ymin) ymin = p.y();
      if (p.y() > ymax) ymax = p.y();
    }
  }
  bounds.setCoords(xmin, ymin, xmax, ymax);
}


void Structure::load()
{
  if (! _loaded) {
    forceLoad();
    _loaded = true;
  }
}


void Structure::forceLoad()
{
  _dirty = false;
//  _elements.clear();
  // FIXME:
  QFileInfo xmlInfo = currentFile();
  if (! xmlInfo.isFile()) {
    qDebug() << "Xml File not found: " << xmlInfo.fileName();
    return;
  }

  QDomDocument domDoc(name());
  QFile xmlStorage(xmlInfo.absoluteFilePath());
  if (!xmlStorage.open(QIODevice::ReadOnly)) {
    qDebug() << "Xml File can't open" << xmlInfo.fileName();
    return;
  }
  if (!domDoc.setContent(&xmlStorage)) {
    qDebug() << "Xml contents error" << xmlInfo.fileName();
    xmlStorage.close();
    return;
  }
  xmlStorage.close();

  QDomElement docElem = domDoc.documentElement();
  QDomNode n = docElem.firstChild();
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) {
//      qDebug() << qPrintable(e.tagName()) << endl;
      if (e.tagName() != QString("element")) break;
      Element *elm = Element::fromXmlElement(e);
      if (elm != 0) {
         elm->setParent(this);
//        _elements.append(elm);
      }
    }
    n = n.nextSibling();
  }
}


void Structure::store()
{
  // FIXME:
  _dirty = false;
}


bool Structure::isDirty() const
{
  return _dirty;
}


} // namespace Gds
