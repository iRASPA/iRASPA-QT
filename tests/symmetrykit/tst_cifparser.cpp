#include <symmetrykit.h>
#include "tst_cifparser.h"

#include <map>
#include <tuple>
#include <vector>

CIFParser::CIFParser()
{
}

CIFParser::~CIFParser()
{
}

// The asymmetric unit of IRMOF-1: a site list whose elements are given by
// '_atom_site_type_symbol' only. Reading every site as carbon was a regression
// that came from SKAsymmetricAtom defaulting to carbon rather than to the
// 'not identified yet' element 0 that the parser tests for.
void CIFParser::test_case_elements_from_type_symbol()
{
  const QString cif = QString(
    "data_IRMOF-1\n"
    "_cell_length_a 25.832\n"
    "_cell_length_b 25.832\n"
    "_cell_length_c 25.832\n"
    "_cell_angle_alpha 90.0\n"
    "_cell_angle_beta 90.0\n"
    "_cell_angle_gamma 90.0\n"
    "_symmetry_space_group_name_H-M 'P 1'\n"
    "loop_\n"
    "_atom_site_label\n"
    "_atom_site_type_symbol\n"
    "_atom_site_fract_x\n"
    "_atom_site_fract_y\n"
    "_atom_site_fract_z\n"
    "Zn1 Zn 0.29356 0.29356 0.29356\n"
    "O1 O 0.25000 0.25000 0.25000\n"
    "O2 O 0.21810 0.28190 0.34020\n"
    "C1 C 0.25000 0.25000 0.34770\n"
    "C2 C 0.25000 0.25000 0.40620\n"
    "C3 C 0.28100 0.28100 0.43440\n"
    "H3 H 0.30250 0.30250 0.41660\n");

  QTemporaryDir directory;
  QVERIFY(directory.isValid());

  const QString path = directory.filePath(QString("IRMOF-1.cif"));
  QFile file(path);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
  QTextStream out(&file);
  out << cif;
  file.close();

  QUrl url = QUrl::fromLocalFile(path);
  SKCIFParser parser = SKCIFParser(url, false, false, CharacterSet::whitespaceAndNewlineCharacterSet());
  parser.startParsing();

  std::map<int, int> occurrences{};
  for(const std::tuple<double3, int, double> &atom: parser.firstTestFrame())
  {
    occurrences[std::get<1>(atom)] += 1;
  }

  QCOMPARE(static_cast<int>(occurrences.size()), 4);
  QCOMPARE(occurrences[1], 1);
  QCOMPARE(occurrences[6], 3);
  QCOMPARE(occurrences[8], 2);
  QCOMPARE(occurrences[30], 1);
}
