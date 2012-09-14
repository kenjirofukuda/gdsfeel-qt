#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtCore/QList>
#include "../GdsFeelCore/library.h"

using namespace Gds;

class TestLibrary : public QObject
{
  Q_OBJECT

private slots:
  void files();
  void open_close();
};

void TestLibrary::files()
{
  QFileInfoList result = Library::files();
  foreach (QFileInfo info, result) {
    qDebug() << info.fileName();
  }
}

void TestLibrary::open_close()
{
  QList<Library*> libs = Library::availables();
  QVERIFY(libs.size() > 0);
  foreach (Library* lib, libs) {
    qDebug() << lib->name();
    QBENCHMARK {
      lib->open();
    }
    QVERIFY(lib->isOpen());
    QBENCHMARK {
      lib->close();
    }
    QVERIFY(lib->isClose());
  }
}

QTEST_APPLESS_MAIN(TestLibrary)
#include "testlibrary.moc"
