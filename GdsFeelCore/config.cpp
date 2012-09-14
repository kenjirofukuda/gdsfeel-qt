#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include "config.h"

namespace Gds {

Config::Config()
{
}

bool Config::isSetuped()
{
  QString path = pathToSmalltalkProject();
  if (path.isEmpty())
    return false;
  return QDir(path).exists();
}

QString Config::pathToSmalltalkProject()
{
  if (! QFile(pathToProperties()).exists()) {
    return "";
  }
  QSettings setting(pathToProperties(),QSettings::IniFormat);
  return setting.value("project.path" ,"").toString();
//  QStringList keys = setting.childKeys();
//  QStringListIterator javaStyleIterator(keys);
//  while (javaStyleIterator.hasNext())
//    qDebug() << javaStyleIterator.next().toLocal8Bit().constData() << endl;
}


void Config::printWarning()
{
  qDebug() << cantRunningMessage();
}

QString Config::cantRunningMessage()
{
  return pathToProperties();
}

QDir Config::directory()
{
  return QDir(QDir::home().absoluteFilePath(".GdsFeel"));
}

QString Config::pathToProperties()
{
  return directory().absoluteFilePath("main.properties");
}


} // namespace Gds

// vim : sw=2 ts=2 expandtab
