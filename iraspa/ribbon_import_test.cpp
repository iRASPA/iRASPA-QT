#include <QCoreApplication>
#include <QUrl>
#include <iostream>
#include "scene.h"
#include "skcolorsets.h"
#include "forcefieldset.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  const QString path = QString::fromLocal8Bit(argv[1]);
  SKColorSets colorSets;
  ForceFieldSets forceFieldSets;
  try
  {
    Scene scene(QUrl::fromLocalFile(path), colorSets, forceFieldSets, true, false);
    std::cout << "import ok\n";
  }
  catch (const std::exception &e)
  {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }
  catch (...)
  {
    std::cerr << "unknown exception\n";
    return 1;
  }
  return 0;
}
