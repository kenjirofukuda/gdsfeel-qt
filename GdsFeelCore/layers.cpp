#include "layers.h"
#include "layer.h"

#include <QtCore/QDebug>
#include <QtXml>
#include <QDomDocument>

namespace Gds {

Layers::Layers()
{
}


Layer* Layers::atNumber(int number)
{
  if (! _layerMap.contains(number)) {
    _layerMap[number] = new Layer(number);
  }
  return _layerMap[number];
}


QList<int> Layers::numbers() const
{
  QList<int> result = _layerMap.keys();
  qSort(result);
  return result;
}


void Layers::load(QFileInfo xmlStorageInfo)
{
  Q_ASSERT(xmlStorageInfo.exists());
  qDebug() << xmlStorageInfo.absoluteFilePath() << endl;

  QDomDocument domDoc("layers");
  QFile xmlStorage(xmlStorageInfo.absoluteFilePath());
  if (!xmlStorage.open(QIODevice::ReadOnly)) {
    qDebug() << "Xml File can't open" << xmlStorageInfo.fileName();
    return;
  }
  if (!domDoc.setContent(&xmlStorage)) {
    qDebug() << "Xml contents error" << xmlStorageInfo.fileName();
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
      if (e.tagName() != QString("layer")) continue;
      Layer *layer = atNumber(e.attribute("gdsno").toInt());
      layer->setAttributes(e);
    }
    n = n.nextSibling();
  }
}

}
