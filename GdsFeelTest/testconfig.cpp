#include <QtTest/QtTest>
#include <QObject>
#include "../GdsFeelCore/config.h"

using namespace Gds;

class TestConfig : public QObject
{
  Q_OBJECT

private slots:
  void isSetuped();
  void pathToSmalltalkProject();
};


void TestConfig::pathToSmalltalkProject()
{
  qDebug() << Config::pathToSmalltalkProject();
  QFileInfo projectDir(Config::pathToSmalltalkProject());
  QVERIFY(projectDir.isDir());
  QVERIFY(projectDir.isWritable());
  QVERIFY(projectDir.isReadable());
}


void TestConfig::isSetuped()
{
  QVERIFY(Config::isSetuped());
}


//QTEST_MAIN(TestConfig)
#include "testconfig.moc"

