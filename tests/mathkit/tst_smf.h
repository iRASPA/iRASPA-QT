#include <QtTest>
#include <mathkit.h>

class SmithNormalForm : public QObject
{
  Q_OBJECT

public:
  SmithNormalForm();
  ~SmithNormalForm();

private slots:
  void initTestCase();
  void cleanupTestCase();

  void test_case();
};


