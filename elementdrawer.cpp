#include "elementdrawer.h"

#include "GdsFeelCore/element.h"
#include "GdsFeelCore/structure.h"
#include "GdsFeelCore/library.h"

namespace Gds {

ElementDrawer::ElementDrawer(Element *elm, Station *station)
{
  _element = elm;
  _station = station;
}


static void pointsToPath(QList<QPointF> points, QPainterPath &path)
{
  int i = 0;
  foreach(QPointF p , points) {
    if (i == 0) {
      path.moveTo(p);
    }
    else {
      path.lineTo(p);
    }
    i++;
  }
}


static void setElementPath(Element *elm, QPainterPath &path)
{
  pointsToPath(elm->outlinePoints(), path);
}


QColor ElementDrawer::colorForElement(Element * ge)
{
  if (PrimitiveElement *pe = qobject_cast<PrimitiveElement *>(ge)) {
    return _station->library()->colorForLayerNumber(pe->layerNumber());
  }
  return Qt::darkGray;
}


void ElementDrawer::setupPen(QPen &pen)
{
  pen.setColor(colorForElement(_element));
  pen.setWidthF(0.0f);
}


void ElementDrawer::installGraphicsItemOn(QGraphicsScene *scene)
{
  QPainterPath path;
  QPen pen;
  setElementPath(_element, path);
  setupPen(pen);
  scene->addPath(path, pen);
}


static bool LayerLessThan(Element* e1, Element* e2)
{
  PrimitiveElement *pe1 = qobject_cast<PrimitiveElement *>(e1);
  PrimitiveElement *pe2 = qobject_cast<PrimitiveElement *>(e2);
  return pe1->layerNumber() < pe2->layerNumber();
}


void ElementDrawer::layerOrderedElements(
    Structure *structure,
    QList<Element*> &primitives,
    QList<Element*> &refereces)
{
  foreach (Element *elm, structure->elements()) {
    if (PrimitiveElement *pe = qobject_cast<PrimitiveElement *>(elm)) {
      primitives.append(pe);
    }
    else {
      refereces.append(elm);
    }
  }
  qSort(primitives.begin(), primitives.end(), LayerLessThan);
}

void ElementDrawer::installStructure(
    Structure *structure,
    QGraphicsScene *scene,
    Station *station)
{

}


ElementDrawer* ElementDrawer::fromElement(Element *elm, Station *station)
{
  QString type = QString(elm->metaObject()->className());
  if (type == QString("Gds::Aref")) {
    return new ArefDrawer(elm, station);
  }
  return new ElementDrawer(elm, station);
}

SrefDrawer::SrefDrawer(Element *elm, Station *station)
  : ElementDrawer(elm, station)
{
}

void SrefDrawer::installGraphicsItemOn(QGraphicsScene * __attribute__((unused)) scene)
{
}

ArefDrawer::ArefDrawer(Element *elm, Station *station)
  : ElementDrawer(elm, station)
{
}


void ArefDrawer::installGraphicsItemOn(QGraphicsScene *scene)
{
  QPainterPath path;
  QPen pen;

  foreach (QMatrix mat, arefElement()->transforms()) {
    QList<QPointF> points;
    arefElement()->lookupOutlinePoints(mat, points);
    pointsToPath(points, path);
    points.clear();
  }

  setupPen(pen);
  scene->addPath(path, pen);
}


} // namespace Gds
