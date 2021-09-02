#include <QtTest>
#include "qfuzzycompare.h"
#include <symmetrykit.h>

class SmallestPrimitiveCellNoPartialOccupancies : public QObject
{
  Q_OBJECT

public:
  SmallestPrimitiveCellNoPartialOccupancies();
  ~SmallestPrimitiveCellNoPartialOccupancies();

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



//#include "tst_smallestprimitivecellnopartialoccupancies.moc"

