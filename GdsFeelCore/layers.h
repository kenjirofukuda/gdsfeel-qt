#ifndef LAYERS_H
#define LAYERS_H

#include <QMap>
#include <QFileInfo>

namespace Gds {

class Layer;

class Layers
{
public:
  Layers();

  Layer* atNumber(int number);
  QList<int> numbers() const;

  void load(QFileInfo xmlStorageInfo);

private:
  QMap<int, Layer*> _layerMap;
};

}

#endif // LAYERS_H
