#ifndef ELEMENT_H
#define ELEMENT_H

#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtXml>
#include <QDomDocument>
#include <QMatrix>

namespace Gds {

class Structure;
class Library;


class Element : public QObject
{
  Q_OBJECT

public:
  Element();
  virtual ~Element();

  Structure *structure();
  Library *library();

  QList<QPointF> vertices() const { return _vertices ;}
  int keyNumber() const {return _keyNumber; }

  void setVertices(const QList<QPointF> &vertices);  
  virtual void setAttributes(QDomElement e);
  QList<QPointF> outlinePoints();
  QRectF dataBounds();

  static Element* fromXmlElement(QDomElement e);  
  static void resetToSmallBounds(QRectF &bounds);
  static void calcDataBounds(QList<QPointF> points, QRectF &bounds);
  static void calcOutlinePoints(QRectF bounds, QList<QPointF> &points);

protected:
  virtual void clearGeometryCache();
  virtual void lookupOutlinePoints(QList<QPointF> &points);
  virtual void lookupDataBounds(QRectF &bounds);

private:
  QList<QPointF> _vertices;
  int _keyNumber;
  QRectF *_dataBounds;
  QList<QPointF> *_outlinePoints;
};


class PrimitiveElement : public Element
{
  Q_OBJECT
public:
  PrimitiveElement();

  int datatype() const { return _datatype;}
  int layerNumber() const { return _layerNumber;}
  virtual void setAttributes(QDomElement e);

private:
  int _datatype;
  int _layerNumber;
};


class Boundary : public PrimitiveElement
{
  Q_OBJECT
};


class Path : public PrimitiveElement
{
  Q_OBJECT
public:
  Path();

  int pathtype() const { return _pathtype; }
  double width() const { return _width; }
  double halhWidth() const { return width() / 2.0; }

  virtual void setAttributes(QDomElement e);

protected:
  virtual void lookupOutlinePoints(QList<QPointF> &points);

private:
  int _pathtype;
  double _width;
};


class ReferenceElement : public Element
{
  Q_OBJECT
protected:
    ReferenceElement();
    virtual ~ReferenceElement();

public:
  double mag() const {return _mag;}
  double angle() const {return _angle;}
  QPointF origin() const;
  bool  reflected() const {return _reflected;}
  virtual void setAttributes(QDomElement e);
  QMatrix transform();

protected:
  virtual void clearGeometryCache();

private:
   void getTransform(QMatrix &mat);


private:
  double _mag;
  double _angle;
  bool _reflected;
  QMatrix *_mat;
};


class Sref : public ReferenceElement
{
  Q_OBJECT
public:
  QString referenceName() const {return _referenceName;}
  virtual void setAttributes(QDomElement e);

  void lookupOutlinePoints(QMatrix mat, QList<QPointF> &points);

protected:
  virtual void lookupOutlinePoints(QList<QPointF> &points);

private:
  QString _referenceName;
};


class Aref : public Sref
{
  Q_OBJECT
public:
    Aref();
    virtual ~Aref();

  int rowCount() const {return _rowCount;}
  int columnCount() const {return _columnCount;}
  double rowStep() const {return _rowStep;}
  double columnStep() const {return _columnStep;}
  QList<QMatrix> transforms();

  virtual void setAttributes(QDomElement e);

protected:
  virtual void clearGeometryCache();
  void lookupTransforms(QList<QMatrix> &transforms);

private:
  int _rowCount;
  int _columnCount;
  double _rowStep;
  double _columnStep;
  QList<QMatrix> *_transforms;
};


} // namespace Gds

#endif // ELEMENT_H
