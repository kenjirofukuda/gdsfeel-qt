#ifndef ELEMENTDRAWER_H
#define ELEMENTDRAWER_H

#include <QtWidgets>
#include "GdsFeelCore/structure.h"
#include "GdsFeelCore/element.h"
#include "GdsFeelCore/station.h"

namespace Gds {

class ElementDrawer
{
public:
  ElementDrawer(Element *elm, Station *station);
  virtual ~ElementDrawer() {}

  virtual void installGraphicsItemOn(QGraphicsScene *scene);

  static void layerOrderedElements(
                            Structure *structure,
                            QList<Element*> &primitives,
                            QList<Element*> &refereces);

  static void installStructure(
                            Structure *structure,
                            QGraphicsScene *scene,
                            Station *station);

  static ElementDrawer* fromElement(Element *elm, Station *station);
  
protected:
  QColor colorForElement(Element * ge);
  void setupPen(QPen &pen);

  Element *_element;
  Station *_station;
};


class SrefDrawer : public ElementDrawer
{
public:
  SrefDrawer(Element *elm, Station *station);
  Sref *srefElement() { return qobject_cast<Sref*>(_element);}
  virtual void installGraphicsItemOn(QGraphicsScene *scene); // override
};


class ArefDrawer : public ElementDrawer
{
public:
  ArefDrawer(Element *elm, Station *station);
  Aref *arefElement() { return qobject_cast<Aref*>(_element);}
  virtual void installGraphicsItemOn(QGraphicsScene *scene); // override
};


} // namespace Gds

#endif // ELEMENTDRAWER_H
