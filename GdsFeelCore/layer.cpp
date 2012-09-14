#include "layer.h"

namespace Gds {

Layer::Layer(int number)
{
  _number = number;
  resetToDefault();
}


void Layer::resetToDefault()
{
  _color = Qt::lightGray;
  _visible = true;
  _selectable = true;
}

static QColor fromXmlElement(QDomElement colorElement)
{
  float r = colorElement.attribute("r").toFloat();
  float g = colorElement.attribute("g").toFloat();
  float b = colorElement.attribute("b").toFloat();
  float a = colorElement.attribute("a", "1.0").toFloat();
  QColor color;
  color.setRedF(r);
  color.setGreenF(g);
  color.setBlueF(b);
  color.setAlphaF(a);
  return color;
}


void Layer::setAttributes(QDomElement e)
{
  resetToDefault();
  _visible = e.attribute("visible", "true") == QString("true");
  _selectable = e.attribute("selectable", "true") == QString("true");
  QDomElement colorElement = e.firstChildElement("color");
  _color = fromXmlElement(colorElement);
}

}
