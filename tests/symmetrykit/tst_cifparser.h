#include <QtTest>
#include <symmetrykit.h>

class CIFParser : public QObject
{
  Q_OBJECT

public:
  CIFParser();
  ~CIFParser();

private slots:
  void test_case_elements_from_type_symbol();
};
