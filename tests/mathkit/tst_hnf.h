#include <QtTest>
#include <mathkit.h>

class HermiteNormalForm : public QObject
{
  Q_OBJECT

public:
  HermiteNormalForm();
  ~HermiteNormalForm();

private slots:
  void initTestCase();
  void cleanupTestCase();

  void test_case();
};


