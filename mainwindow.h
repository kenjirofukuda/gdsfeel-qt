#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtWidgets>
#include "GdsFeelCore/station.h"
#include "GdsFeelCore/element.h"

namespace Ui
{
    class MainWindow;
}

class Gds::Element;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void currentLibraryChaged(const QModelIndex &current,
                            const QModelIndex &previous);
  void currentStructureChaged(const QModelIndex &current,
                              const QModelIndex &previous);
private:
  void listStructure(QString libname);
  QColor colorForElement(Gds::Element * ge);

private:
  Gds::Station _station;
  QGraphicsScene *_scene;
  QGraphicsView *_view;
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

// vim : sw=2 ts=2 expandtab
