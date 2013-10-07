#include "element.h"
#include "structure.h"
#include "library.h"

namespace Gds {

Element::Element()
{
  _vertices.clear();
  _dataBounds = 0;
  _outlinePoints = 0;
}


Element::~Element()
{
  _vertices.clear();
  clearGeometryCache();
}


Structure *Element::structure()
{
  return qobject_cast<Structure *>(parent());
}


Library *Element::library()
{
  if (structure() == 0) {
    return 0;
  }
  return structure()->library();
}


void Element::clearGeometryCache()
{
  free(_dataBounds);
  _dataBounds = 0;
  if (_outlinePoints) {
    _outlinePoints->clear();
  }
  free(_outlinePoints);
  _outlinePoints = 0;
}


void Element::setVertices(const QList<QPointF> &vertices)
{
  _vertices.clear();
  _vertices.append(vertices);
  clearGeometryCache();
}


void Element::setAttributes(QDomElement e)
{
  QList<QPointF> points;
  QDomElement verte = e.firstChildElement("vertices");
  QDomNode xyn = verte.firstChildElement("xy");
  while (!xyn.isNull()) {
    QDomElement xye = xyn.toElement();
//    qDebug() << qPrintable(xye.tagName()) << endl;
//    qDebug() << xye.text();
    QStringList items = xye.text().split(" ");
    QPointF pt(items[0].toFloat(), items.at(1).toFloat());
    points.push_back(pt);
    xyn = xyn.nextSibling();
  }
  setVertices(points);
  _keyNumber = e.attribute("keyNumber").toInt();
}


QList<QPointF> Element::outlinePoints()
{
  QList<QPointF> result;
  if (_outlinePoints == 0) {
    _outlinePoints = new QList<QPointF>;
    lookupOutlinePoints(*_outlinePoints);
  }
  return *_outlinePoints;
}


const qreal MAX_VAL = 32767;


QRectF Element::dataBounds()
{
  if (_dataBounds == 0) {
    _dataBounds = new QRectF;
    resetToSmallBounds(*_dataBounds);
    lookupDataBounds(*_dataBounds);
  }
  return *_dataBounds;
}


void Element::resetToSmallBounds(QRectF &bounds)
{
  bounds.setCoords(MAX_VAL, MAX_VAL, -MAX_VAL, -MAX_VAL);
}


void Element::calcDataBounds(QList<QPointF> points, QRectF &bounds)
{
  qreal xmin = MAX_VAL;
  qreal xmax = -MAX_VAL;
  qreal ymin = MAX_VAL;
  qreal ymax = -MAX_VAL;
  foreach (QPointF p, points) {
    if (p.x() < xmin) xmin = p.x();
    if (p.x() > xmax) xmax = p.x();
    if (p.y() < ymin) ymin = p.y();
    if (p.y() > ymax) ymax = p.y();
  }
  bounds.setCoords(xmin, ymin, xmax, ymax);
}


void Element::calcOutlinePoints(QRectF bounds, QList<QPointF> &points)
{
  qreal xmin, xmax, ymin, ymax;
  bounds.getCoords(&xmin, &ymin, &xmax, &ymax);
  points.append(QPointF(xmin, ymin));
  points.append(QPointF(xmin, ymax));
  points.append(QPointF(xmax, ymax));
  points.append(QPointF(xmax, ymin));
  points.append(QPointF(xmin, ymin));
}


void Element::lookupDataBounds(QRectF &bounds)
{
  calcDataBounds(outlinePoints(), bounds);
}


void Element::lookupOutlinePoints(QList<QPointF> &points)
{
  points = vertices();
}


static Element* newElementFromType(QString type)
{
  if (type == QString("boundary")) {
    return new Boundary();
  }
  if (type == QString("path")) {
    return new Path();
  }
  if (type == QString("sref")) {
    return new Sref();
  }
  if (type == QString("aref")) {
    return new Aref();
  }
  qDebug() << "Can't handled: " << type << endl;
  return 0;
}


Element*
Element::fromXmlElement(QDomElement e)
{
  Element *elm = newElementFromType(e.attribute("type"));
  if (elm == 0) return 0;
  elm->setAttributes(e);
  return elm;
}


PrimitiveElement::PrimitiveElement()
{
  _datatype = 0;
  _layerNumber = 0;
}


void PrimitiveElement::setAttributes(QDomElement e)
{
  Element::setAttributes(e);
  _datatype = e.attribute("datatype", "0").toInt();
  _layerNumber = e.attribute("layerNumber", "0").toInt();
}


Path::Path()
{
  _width = 0.0;
  _pathtype = 0;
}


static qreal
getAngle(qreal x1, qreal y1, qreal x2, qreal y2)
{
  qreal angle;

  if(x1 == x2)
    angle =  M_PI_2 * ((y2 > y1) ? 1 : -1);
  else
  {
    angle = atan(fabs(y2 - y1)/fabs(x2 - x1));
    if(y2 >= y1)
    {
      if(x2 >= x1)
        angle += 0;
      else
        angle = M_PI - angle;
    }
    else
    {
      if(x2 >= x1)
        angle = 2 * M_PI - angle;
      else
        angle += M_PI;
    }
  }

  return angle;
}

#define EPS 1e-8

static QPointF
getDeltaXY(qreal hw, QPointF p1, QPointF p2, QPointF p3)
{
  qreal alpha, beta, theta, r;
  QPointF pnt;

  alpha = getAngle(p1.x(), p1.y(), p2.x(), p2.y());
  beta = getAngle(p2.x(), p2.y(), p3.x(), p3.y());

  theta = (alpha + beta + M_PI)/2.0;

  if(fabs(cos((alpha - beta)/2.0)) < EPS)
  {
    qDebug() << "Internal algorithm error: cos((alpha - beta)/2) = 0" << endl;
    return pnt;
  }
  r = ((double) hw) / cos((alpha - beta)/2.0);

  pnt.setX((qreal) (r * cos(theta)));
  pnt.setY((qreal) (r * sin(theta)));

  return pnt;
}

static QPointF
getEndDeltaXY(qreal hw, QPointF p1, QPointF p2)
{
  qreal alpha, theta, r;
  QPointF pnt;

  alpha = getAngle(p1.x(), p1.y(), p2.x(), p2.y());

  theta = alpha;
  r= hw;
  pnt.setX(-r * sin(theta));
  pnt.setY(r * cos(theta));

  return pnt;
}


static void
pathOutlinePoints(Path* path, QList<QPointF> &outpoints)
{
  outpoints.clear();
  if (path ->width() == 0.0) {
    outpoints.append(path->vertices());
    return;
  }

  qreal hw = path->halhWidth();
  int numpoints = path->vertices().size();
  if (numpoints < 2) {
    qDebug() << "PathToBoundary(): don't know to handle wires < 2 pts yet" << endl;
    return;
  }
  QPointF deltaxy =
      getEndDeltaXY(hw, path->vertices()[0], path->vertices()[1]);
  QVector<QPointF> points(2 * numpoints + 1);
  if (path->pathtype() == 0) {
    points[0].setX(path->vertices()[0].x() + deltaxy.x());
    points[0].setY(path->vertices()[0].y() + deltaxy.y());
    points[2 * numpoints].setX(points[0].x());
    points[2 * numpoints].setY(points[0].y());
    points[2 * numpoints - 1].setX(path->vertices()[0].x() - deltaxy.x());
    points[2 * numpoints - 1].setY(path->vertices()[0].y() - deltaxy.y());
  }
  else {
    points[0].setX(path->vertices()[0].x() + deltaxy.x() - deltaxy.y());
    points[0].setY(path->vertices()[0].y() + deltaxy.y() - deltaxy.x());
    points[2 * numpoints].setX(points[0].x());
    points[2 * numpoints].setY(points[0].y());
    points[2 * numpoints - 1].setX(path->vertices()[0].x() - deltaxy.x() - deltaxy.y());
    points[2 * numpoints - 1].setY(path->vertices()[0].y() - deltaxy.y() - deltaxy.x());
  }

  for(int i = 1; i < numpoints - 1; i++)
  {
    deltaxy = getDeltaXY(hw, path->vertices()[i - 1],
                         path->vertices()[i], path->vertices()[i + 1]);
    points[i].setX(path->vertices()[i].x() + deltaxy.x());
    points[i].setY(path->vertices()[i].y() + deltaxy.y());
    points[2 * numpoints - i - 1].setX(path->vertices()[i].x() - deltaxy.x());
    points[2 * numpoints - i - 1].setY(path->vertices()[i].y() - deltaxy.y());
  }

  deltaxy = getEndDeltaXY(hw, path->vertices()[numpoints - 2],
                          path->vertices()[numpoints - 1]);
  if(path->pathtype() == 0)
  {
    points[numpoints - 1].setX(path->vertices()[numpoints - 1].x() + deltaxy.x());
    points[numpoints - 1].setY(path->vertices()[numpoints - 1].y() + deltaxy.y());
    points[numpoints].setX(path->vertices()[numpoints - 1].x() - deltaxy.x());
    points[numpoints].setY(path->vertices()[numpoints - 1].y() - deltaxy.y());
  }
  else /* Extended end */
  {
    points[numpoints - 1].setX(path->vertices()[numpoints - 1].x() + deltaxy.x() + deltaxy.y());
    points[numpoints - 1].setY(path->vertices()[numpoints - 1].y() + deltaxy.y() + deltaxy.x());
    points[numpoints].setX(path->vertices()[numpoints - 1].x() - deltaxy.x() + deltaxy.y());
    points[numpoints].setY(path->vertices()[numpoints - 1].y() - deltaxy.y() + deltaxy.x());
  }
  outpoints.append(points.toList());
}


void Path::lookupOutlinePoints(QList<QPointF> &points)
{
  pathOutlinePoints(this, points);
}


void Path::setAttributes(QDomElement e)
{
  PrimitiveElement::setAttributes(e);
  _pathtype = e.attribute("pathtype", "0").toInt();
  _width = e.attribute("width", "0.0").toFloat();
}


ReferenceElement::ReferenceElement()
{
  _mag = 1.0;
  _angle = 0.0;
  _reflected = false;
  _mat = 0;
}


ReferenceElement::~ReferenceElement()
{
  free(_mat);
  _mat = 0;
}


void ReferenceElement::clearGeometryCache()
{
  Element::clearGeometryCache();
  free(_mat);
  _mat = 0;
}


QPointF ReferenceElement::origin() const
{
  Q_ASSERT(! vertices().isEmpty());
  return vertices().first();
}


void ReferenceElement::setAttributes(QDomElement e)
{
  Element::setAttributes(e);
  _mag = e.attribute("mag", "1.0").toDouble();
  _angle = e.attribute("angle", "0.0").toDouble();
  _reflected = e.attribute("reflected", "false") == QString("true");
  if (_reflected) {
    qDebug() << "MIRROR" << endl;
  }
  clearGeometryCache();
}


QMatrix ReferenceElement::transform()
{
  if (_mat == 0) {
    _mat = new QMatrix;
    getTransform(*_mat);
  }
  return *_mat;
}


void ReferenceElement::getTransform(QMatrix &mat)
{
  qreal rad = _angle * M_PI / 180.0;
  qreal cos_rad = cos(rad);
  qreal sin_rad = sin(rad);

  qreal a =  _mag * cos_rad;
  qreal b = -_mag * sin_rad;
  qreal c = origin().x();
  qreal d =  _mag * sin_rad;
  qreal e =  _mag * cos_rad;
  qreal f = origin().y();

  /* GDSII understands only the Y mirroring */
  /* Reflecting about X means changing *Y* */
  if (_reflected) {
    b = -b;
    e = -e;
  }
  mat.setMatrix(a, d, b, e, c, f);
}


void Sref::lookupOutlinePoints(QList<QPointF> &points)
{
  lookupOutlinePoints(transform(), points);
}


void Sref::lookupOutlinePoints(QMatrix mat, QList<QPointF> &points)
{
  if (referenceName().isEmpty()) {
    qDebug() << "empty reference name" << endl;
    return;
  }
  if (library() == 0) {
    qDebug() << "library not bind" << endl;
    return;
  }
  Structure *ref = library()->structureNamed(referenceName());
  if (ref == 0) {
    qDebug() << "structure not found: " << referenceName() << endl;
    return;
  }

  QList<QPointF> outlinePoints;
  Element::calcOutlinePoints(ref->dataBounds(), outlinePoints);

  foreach (QPointF p, outlinePoints) {
    points.append(mat.map(p));
  }
}


void Sref::setAttributes(QDomElement e)
{
  ReferenceElement::setAttributes(e);
  _referenceName = e.attribute("sname", "").toUpper();
}


Aref::Aref()
{
  _transforms = 0;
}


Aref::~Aref()
{
  if (_transforms) {
    _transforms->clear();
  }
  free(_transforms);
  _transforms = 0;
}


void Aref::clearGeometryCache()
{
  Sref::clearGeometryCache();
  free(_transforms);
  _transforms = 0;
}


void Aref::setAttributes(QDomElement e)
{
  Sref::setAttributes(e);
  QDomElement ashapeDE = e.firstChildElement("ashape");
  if (ashapeDE.isNull()) {
    qDebug() << "ashape element not found" << endl;
    return;
  }
  QDomElement ae = ashapeDE.toElement();
  _rowCount = ae.attribute("rows", "1").toInt();
  _columnCount = ae.attribute("cols", "1").toInt();
  _rowStep = ae.attribute("row-spacing", "0.0").toDouble();
  _columnStep = ae.attribute("column-spacing", "0.0").toDouble();
  clearGeometryCache();
}


QList<QMatrix> Aref::transforms()
{
  if (_transforms == 0) {
    _transforms = new QList<QMatrix>;
    lookupTransforms(*_transforms);
  }
  return *_transforms;
}


void Aref::lookupTransforms(QList<QMatrix> &transforms)
{
  for (int ri = 0; ri < _rowCount; ri++) {
    for (int ci = 0; ci < _columnCount; ci++) {
      double xOffset = ci * _columnStep;
      double yOffset = ri * _rowStep;
      QMatrix copy(transform());
      copy.translate(xOffset, yOffset);
      transforms.append(copy);
    }
  }
}


} // namespace Gds
