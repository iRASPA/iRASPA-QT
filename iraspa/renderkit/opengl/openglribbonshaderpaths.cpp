#include "openglribbonshaderpaths.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace OpenGLRibbonShaders
{

std::string loadShaderStageSource(const char *fileName)
{
  const QString resourcePath = QStringLiteral(":/opengl/shaders/") + QString::fromUtf8(fileName);
  const QString sourceTreePath = QFileInfo(__FILE__).absolutePath() + QStringLiteral("/shaders/") + QString::fromUtf8(fileName);

  for (const QString &path : {resourcePath, sourceTreePath})
  {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      return file.readAll().toStdString();
    }
  }

  qFatal("Could not open ribbon shader stage: %s (also tried %s)", qPrintable(resourcePath), qPrintable(sourceTreePath));
}

}
