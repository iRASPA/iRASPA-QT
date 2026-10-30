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
  void test_parse_identity_operation();
  void test_parse_body_centered_operation();
  void test_hm_symbol_disambiguation_with_symops();
  void test_seitz_matrices_from_empty_generator_encoding();
  void test_get_origin_shift_for_p1();
  void test_material_type_inference();
};
