#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "elementdrawer.h"

#include <QtCore>
#include <QtWidgets>
#include "GdsFeelCore/library.h"
#include "GdsFeelCore/structure.h"
#include "GdsFeelCore/station.h"
#include "GdsFeelCore/element.h"

using namespace Gds;

QAbstractItemModel *createLibraryListModel(QList<Library*> libs) {
  QStandardItemModel *model = new QStandardItemModel(0, 1);
  model->setHeaderData(0, Qt::Horizontal, QObject::tr("Library"));

  foreach (Library* lib, libs) {
    QStandardItem *item = new QStandardItem(lib->name());
    model->appendRow(item);
  }
  return model;
}


void MainWindow::listStructure(QString libname)
{
  QStandardItemModel *model = new QStandardItemModel(0, 1);
  model->setHeaderData(0, Qt::Horizontal, QObject::tr("Structure"));
  _station.setActiveLibraryNamed(libname);
  if (! _station.library()) return;
  QStringList names = _station.library()->structureNames();
  foreach (QString name, names) {
    QStandardItem *item = new QStandardItem(name);
    model->appendRow(item);
  }
  ui->structureListView->setModel(model);
  ui->structureListView->setSelectionModel(new QItemSelectionModel(
      ui->structureListView->model()));
  connect(ui->structureListView->selectionModel(),
          SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          this,
          SLOT(currentStructureChaged(QModelIndex,QModelIndex)));
}


void MainWindow::currentLibraryChaged(const QModelIndex &current,
                                      const QModelIndex & /* previous*/)
{
  QStandardItem *item =
      qobject_cast<QStandardItemModel*>(ui->libraryListView->model())
      ->itemFromIndex(current);
  qDebug() << item->text();
  listStructure(item->text());
}



void MainWindow::currentStructureChaged(const QModelIndex &current,
                                        const QModelIndex & /* previous*/)
{
  if (! _station.library()) return;
  QStandardItem *item =
      qobject_cast<QStandardItemModel*>(ui->structureListView->model())
      ->itemFromIndex(current);
  qDebug() << item->text();
  _station.setActiveStructureNamed(item->text());
  if (! _station.structure()) {
    qDebug() << "Library" << item->text();
    return;
  }
  _station.structure()->load();

  if (_scene) {
    _scene->clear();
    delete _scene;
    _scene = 0;
  }
  _scene = new QGraphicsScene;
  QList<Element *> primitives;
  QList<Element *> references;

  ElementDrawer::layerOrderedElements(
      _station.structure(), primitives, references);

  QList<Element *> displayElements;
  displayElements.append(primitives);
  displayElements.append(references);

  if (displayElements.size() > 0) {
    foreach (Element *elm, displayElements) {
      if (elm == nullptr) {
        qDebug() << "Null Element" << endl;
        continue;
      }
//      if (elm->vertices().size() < 2) continue;
      ElementDrawer *ed = ElementDrawer::fromElement(elm, &_station);
      ed->installGraphicsItemOn(_scene);
      free(ed);
      ed = nullptr;
    }
  }
  _view->setScene(_scene);
  _view->setBackgroundBrush(Qt::black);
  _view->show();
  QRectF r = _view->sceneRect();
  _view->fitInView(r, Qt::KeepAspectRatio);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  _station.setup();
  _scene = new QGraphicsScene;
  QGridLayout *layout = new QGridLayout;
  ui->frame->setLayout(layout);
  ui->frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _view = new QGraphicsView;
  _view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(_view);
  _view->scale(1, -1);


  ui->libraryListView->setModel(createLibraryListModel(_station.libs()));
  ui->libraryListView->setSelectionModel(new QItemSelectionModel(
      ui->libraryListView->model()));
  connect(ui->libraryListView->selectionModel(),
          SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          this,
          SLOT(currentLibraryChaged(QModelIndex,QModelIndex)));
}


MainWindow::~MainWindow()
{
  _station.tearDown();
  delete ui;
}

// vim : sw=2 ts=2 expandtab
