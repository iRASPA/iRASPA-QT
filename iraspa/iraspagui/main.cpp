/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QTranslator>
#include <QLibraryInfo>

#ifdef Q_OS_WIN
 #include <windows.h>
 #include <winuser.h>
#endif
int main(int argc, char *argv[])
{
  #if defined (USE_OPENGL)
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSamples(1);
    format.setDepthBufferSize(0);
    format.setStencilBufferSize(0);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);

    #if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
      QSurfaceFormat::setDefaultFormat(format);
    #endif
  #endif

  #if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
    #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
      QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    #endif
  #endif

  // https://vicrucann.github.io/tutorials/osg-qt-high-dpi/
#ifdef Q_OS_WIN
   ::SetProcessDPIAware(); // call before the main event loop
#endif

  QApplication a(argc, argv);

  QTranslator qtTranslator, appTranslator;

  //QLocale::setDefault(QLocale(QLocale::Chinese, QLocale::Script::SimplifiedChineseScript, QLocale::Country::China));
  //QLocale::setDefault(QLocale(QLocale::Arabic, QLocale::Script::ArabicScript, QLocale::Country::Syria));
  //QLocale::setDefault(QLocale(QLocale::Japanese, QLocale::Script::JapaneseScript, QLocale::Country::Japan));
  //QLocale::setDefault(QLocale(QLocale::Hindi, QLocale::Script::DevanagariScript, QLocale::Country::India));
  //QLocale::setDefault(QLocale(QLocale::Spanish, QLocale::Script::LatinScript, QLocale::Country::Spain));
  //QLocale::setDefault(QLocale(QLocale::Dutch, QLocale::Script::LatinScript, QLocale::Country::Netherlands));
  //QLocale::setDefault(QLocale(QLocale::Persian, QLocale::Script::ArabicScript, QLocale::Country::Iran));
  //QLocale::setDefault(QLocale(QLocale::German, QLocale::Script::LatinScript, QLocale::Country::Germany));

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  if( qtTranslator.load(QLocale(), QLatin1String("qt"), QLatin1String("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
#else
  if( qtTranslator.load(QLocale(), QLatin1String("qt"), QLatin1String("_"), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
#endif
  {
    a.installTranslator(&qtTranslator);
  }
  if( appTranslator.load(QLocale(), QLatin1String("iraspa"), QLatin1String("_"), QLatin1String(":/i18n")))
  {
    a.installTranslator(&appTranslator);
  }

  MainWindow w;
  w.show();

  return a.exec();
}
