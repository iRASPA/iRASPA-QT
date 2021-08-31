#include <QtTest>

#include <symmetrykit.h>

class SmallestPrimitiveCell : public QObject
{
  Q_OBJECT

public:
  SmallestPrimitiveCell();
  ~SmallestPrimitiveCell();

private slots:
  void initTestCase();
  void cleanupTestCase();

  void test_case_triclinic();
  void test_case_monoclinic();
  void test_case_orthorhombic();
  void test_case_tetragonal();
  void test_case_trigonal();
  void test_case_hexagonal();
  void test_case_cubic();
  void test_case_virtual();
};

//#include "tst_smallestprimitivecell.moc"


