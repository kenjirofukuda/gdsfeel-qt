#ifndef CONFIG_H
#define CONFIG_H

class QString;
class QDir;
class QFile;


namespace Gds {

class Config
{
public:
  Config();

  static bool isSetuped();
  static void printWarning();
  static QString cantRunningMessage();
  static QString pathToSmalltalkProject();

private:
  static QDir directory();
  static QString pathToProperties();
};

} // namespace Gds

// vim : sw=2 ts=2 expandtab

#endif // CONFIG_H
