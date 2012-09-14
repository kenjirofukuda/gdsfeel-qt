#ifndef LAYER_H
#define LAYER_H

#include <QColor>
#include <QtXml>

namespace Gds {

class Layer
{
public:
  Layer(int number = -1);

  bool isVisible() const {return _visible;}
  bool isSelectable() const {return _selectable;}
  QColor color() const {return _color;}

  void setAttributes(QDomElement e);

private:
  void resetToDefault();

  int _number;
  bool _selectable;
  bool _visible;
  QColor _color;
};

}

#endif // LAYER_H
