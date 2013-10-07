#include <QtWidgets>
#include "mainwindow.h"
#include "GdsFeelCore/config.h"
#include "GdsFeelCore/library.h"

using namespace Gds;

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  if (! Config::isSetuped()) {
    Config::printWarning();
    QMessageBox::warning(
        0,
        "Error!",
        Config::cantRunningMessage());
    return 1;
  }

  MainWindow w;
  w.show();
  return a.exec();
}

// vim : sw=2 ts=2 expandtab
