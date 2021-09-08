#include <symmetrykit.h>
#include "tst_symmetrykittransformationmatrix.h"

TestTransformationMatrix::TestTransformationMatrix()
{
}

TestTransformationMatrix::~TestTransformationMatrix()
{

}

void TestTransformationMatrix::initTestCase()
{

}

void TestTransformationMatrix::cleanupTestCase()
{

}


void TestTransformationMatrix::test_case_triclinic()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/triclinic/POSCAR-001",
    "SpglibTestData/triclinic/POSCAR-002"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}

void TestTransformationMatrix::test_case_monoclinic()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/monoclinic/POSCAR-003",
    "SpglibTestData/monoclinic/POSCAR-004",
    "SpglibTestData/monoclinic/POSCAR-004-2",
    "SpglibTestData/monoclinic/POSCAR-005",
    "SpglibTestData/monoclinic/POSCAR-005-2",
    "SpglibTestData/monoclinic/POSCAR-006",
    "SpglibTestData/monoclinic/POSCAR-006-2",
    "SpglibTestData/monoclinic/POSCAR-007",
    "SpglibTestData/monoclinic/POSCAR-007-2",
    "SpglibTestData/monoclinic/POSCAR-008",
    "SpglibTestData/monoclinic/POSCAR-008-2",
    "SpglibTestData/monoclinic/POSCAR-009",
    "SpglibTestData/monoclinic/POSCAR-009-2",
    "SpglibTestData/monoclinic/POSCAR-010",
    "SpglibTestData/monoclinic/POSCAR-010-2",
    "SpglibTestData/monoclinic/POSCAR-011",
    "SpglibTestData/monoclinic/POSCAR-011-2",
    "SpglibTestData/monoclinic/POSCAR-012",
    "SpglibTestData/monoclinic/POSCAR-012-2",
    "SpglibTestData/monoclinic/POSCAR-012-3",
    "SpglibTestData/monoclinic/POSCAR-013",
    "SpglibTestData/monoclinic/POSCAR-013-2",
    "SpglibTestData/monoclinic/POSCAR-013-3",
    "SpglibTestData/monoclinic/POSCAR-014",
    "SpglibTestData/monoclinic/POSCAR-014-2",
    "SpglibTestData/monoclinic/POSCAR-015",
    "SpglibTestData/monoclinic/POSCAR-015-2",
    "SpglibTestData/monoclinic/POSCAR-015-3"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}

void TestTransformationMatrix::test_case_orthorhombic()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/orthorhombic/POSCAR-016",
    "SpglibTestData/orthorhombic/POSCAR-016-2",
    "SpglibTestData/orthorhombic/POSCAR-017-2",
    "SpglibTestData/orthorhombic/POSCAR-018",
    "SpglibTestData/orthorhombic/POSCAR-018-2",
    "SpglibTestData/orthorhombic/POSCAR-019",
    "SpglibTestData/orthorhombic/POSCAR-019-2",
    "SpglibTestData/orthorhombic/POSCAR-020",
    "SpglibTestData/orthorhombic/POSCAR-021",
    "SpglibTestData/orthorhombic/POSCAR-021-2",
    "SpglibTestData/orthorhombic/POSCAR-022",
    "SpglibTestData/orthorhombic/POSCAR-023",
    "SpglibTestData/orthorhombic/POSCAR-023-2",
    "SpglibTestData/orthorhombic/POSCAR-024",
    "SpglibTestData/orthorhombic/POSCAR-024-2",
    "SpglibTestData/orthorhombic/POSCAR-025",
    "SpglibTestData/orthorhombic/POSCAR-025-2",
    "SpglibTestData/orthorhombic/POSCAR-026",
    "SpglibTestData/orthorhombic/POSCAR-026-2",
    "SpglibTestData/orthorhombic/POSCAR-027",
    "SpglibTestData/orthorhombic/POSCAR-027-2",
    "SpglibTestData/orthorhombic/POSCAR-028",
    "SpglibTestData/orthorhombic/POSCAR-028-2",
    "SpglibTestData/orthorhombic/POSCAR-029",
    "SpglibTestData/orthorhombic/POSCAR-029-2",
    "SpglibTestData/orthorhombic/POSCAR-030",
    "SpglibTestData/orthorhombic/POSCAR-030-2",
    "SpglibTestData/orthorhombic/POSCAR-031",
    "SpglibTestData/orthorhombic/POSCAR-031-2",
    "SpglibTestData/orthorhombic/POSCAR-032",
    "SpglibTestData/orthorhombic/POSCAR-032-2",
    "SpglibTestData/orthorhombic/POSCAR-033",
    "SpglibTestData/orthorhombic/POSCAR-033-2",
    "SpglibTestData/orthorhombic/POSCAR-033-3",
    "SpglibTestData/orthorhombic/POSCAR-034",
    "SpglibTestData/orthorhombic/POSCAR-034-2",
    "SpglibTestData/orthorhombic/POSCAR-035",
    "SpglibTestData/orthorhombic/POSCAR-035-2",
    "SpglibTestData/orthorhombic/POSCAR-036",
    "SpglibTestData/orthorhombic/POSCAR-036-2",
    "SpglibTestData/orthorhombic/POSCAR-037",
    "SpglibTestData/orthorhombic/POSCAR-037-2",
    "SpglibTestData/orthorhombic/POSCAR-038",
    "SpglibTestData/orthorhombic/POSCAR-038-2",
    "SpglibTestData/orthorhombic/POSCAR-039",
    "SpglibTestData/orthorhombic/POSCAR-039-2",
    "SpglibTestData/orthorhombic/POSCAR-040",
    "SpglibTestData/orthorhombic/POSCAR-040-2",
    "SpglibTestData/orthorhombic/POSCAR-041",
    "SpglibTestData/orthorhombic/POSCAR-041-2",
    "SpglibTestData/orthorhombic/POSCAR-042",
    "SpglibTestData/orthorhombic/POSCAR-043",
    "SpglibTestData/orthorhombic/POSCAR-043-2",
    "SpglibTestData/orthorhombic/POSCAR-044",
    "SpglibTestData/orthorhombic/POSCAR-044-2",
    "SpglibTestData/orthorhombic/POSCAR-045",
    "SpglibTestData/orthorhombic/POSCAR-045-2",
    "SpglibTestData/orthorhombic/POSCAR-046",
    "SpglibTestData/orthorhombic/POSCAR-046-2",
    "SpglibTestData/orthorhombic/POSCAR-047",
    "SpglibTestData/orthorhombic/POSCAR-047-2",
    "SpglibTestData/orthorhombic/POSCAR-048",
    "SpglibTestData/orthorhombic/POSCAR-048-2",
    "SpglibTestData/orthorhombic/POSCAR-049",
    "SpglibTestData/orthorhombic/POSCAR-049-2",
    "SpglibTestData/orthorhombic/POSCAR-050",
    "SpglibTestData/orthorhombic/POSCAR-050-2",
    "SpglibTestData/orthorhombic/POSCAR-051",
    "SpglibTestData/orthorhombic/POSCAR-051-2",
    "SpglibTestData/orthorhombic/POSCAR-051-3",
    "SpglibTestData/orthorhombic/POSCAR-052",
    "SpglibTestData/orthorhombic/POSCAR-052-2",
    "SpglibTestData/orthorhombic/POSCAR-053",
    "SpglibTestData/orthorhombic/POSCAR-053-2",
    "SpglibTestData/orthorhombic/POSCAR-054",
    "SpglibTestData/orthorhombic/POSCAR-054-2",
    "SpglibTestData/orthorhombic/POSCAR-055",
    "SpglibTestData/orthorhombic/POSCAR-055-2",
    "SpglibTestData/orthorhombic/POSCAR-056",
    "SpglibTestData/orthorhombic/POSCAR-056-2",
    "SpglibTestData/orthorhombic/POSCAR-057",
    "SpglibTestData/orthorhombic/POSCAR-057-2",
    "SpglibTestData/orthorhombic/POSCAR-058",
    "SpglibTestData/orthorhombic/POSCAR-058-2",
    "SpglibTestData/orthorhombic/POSCAR-058-3",
    "SpglibTestData/orthorhombic/POSCAR-059",
    "SpglibTestData/orthorhombic/POSCAR-059-2",
    "SpglibTestData/orthorhombic/POSCAR-060",
    "SpglibTestData/orthorhombic/POSCAR-060-2",
    "SpglibTestData/orthorhombic/POSCAR-060-3",
    "SpglibTestData/orthorhombic/POSCAR-061",
    "SpglibTestData/orthorhombic/POSCAR-061-2",
    "SpglibTestData/orthorhombic/POSCAR-062",
    "SpglibTestData/orthorhombic/POSCAR-062-2",
    "SpglibTestData/orthorhombic/POSCAR-063",
    "SpglibTestData/orthorhombic/POSCAR-063-2",
    "SpglibTestData/orthorhombic/POSCAR-063-3",
    "SpglibTestData/orthorhombic/POSCAR-064",
    "SpglibTestData/orthorhombic/POSCAR-064-2",
    "SpglibTestData/orthorhombic/POSCAR-064-3",
    "SpglibTestData/orthorhombic/POSCAR-065",
    "SpglibTestData/orthorhombic/POSCAR-065-2",
    "SpglibTestData/orthorhombic/POSCAR-065-3",
    "SpglibTestData/orthorhombic/POSCAR-066",
    "SpglibTestData/orthorhombic/POSCAR-066-2",
    "SpglibTestData/orthorhombic/POSCAR-067",
    "SpglibTestData/orthorhombic/POSCAR-067-2",
    "SpglibTestData/orthorhombic/POSCAR-067-3",
    "SpglibTestData/orthorhombic/POSCAR-068",
    "SpglibTestData/orthorhombic/POSCAR-068-2",
    "SpglibTestData/orthorhombic/POSCAR-069",
    "SpglibTestData/orthorhombic/POSCAR-069-2",
    "SpglibTestData/orthorhombic/POSCAR-070",
    "SpglibTestData/orthorhombic/POSCAR-070-2",
    "SpglibTestData/orthorhombic/POSCAR-071",
    "SpglibTestData/orthorhombic/POSCAR-071-2",
    "SpglibTestData/orthorhombic/POSCAR-072",
    "SpglibTestData/orthorhombic/POSCAR-072-2",
    "SpglibTestData/orthorhombic/POSCAR-073",
    "SpglibTestData/orthorhombic/POSCAR-073-2",
    "SpglibTestData/orthorhombic/POSCAR-074",
    "SpglibTestData/orthorhombic/POSCAR-074-2"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}

void TestTransformationMatrix::test_case_tetragonal()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/tetragonal/POSCAR-075",
    "SpglibTestData/tetragonal/POSCAR-075-2",
    "SpglibTestData/tetragonal/POSCAR-076",
    "SpglibTestData/tetragonal/POSCAR-076-2",
    "SpglibTestData/tetragonal/POSCAR-077",
    "SpglibTestData/tetragonal/POSCAR-077-2",
    "SpglibTestData/tetragonal/POSCAR-077-3",
    "SpglibTestData/tetragonal/POSCAR-078",
    "SpglibTestData/tetragonal/POSCAR-078-2",
    "SpglibTestData/tetragonal/POSCAR-079",
    "SpglibTestData/tetragonal/POSCAR-079-2",
    "SpglibTestData/tetragonal/POSCAR-080",
    "SpglibTestData/tetragonal/POSCAR-080-2",
    "SpglibTestData/tetragonal/POSCAR-081",
    "SpglibTestData/tetragonal/POSCAR-081-2",
    "SpglibTestData/tetragonal/POSCAR-082",
    "SpglibTestData/tetragonal/POSCAR-082-2",
    "SpglibTestData/tetragonal/POSCAR-083",
    "SpglibTestData/tetragonal/POSCAR-083-2",
    "SpglibTestData/tetragonal/POSCAR-083-3",
    "SpglibTestData/tetragonal/POSCAR-084",
    "SpglibTestData/tetragonal/POSCAR-084-2",
    "SpglibTestData/tetragonal/POSCAR-085",
    "SpglibTestData/tetragonal/POSCAR-085-2",
    "SpglibTestData/tetragonal/POSCAR-086",
    "SpglibTestData/tetragonal/POSCAR-086-2",
    "SpglibTestData/tetragonal/POSCAR-087",
    "SpglibTestData/tetragonal/POSCAR-087-2",
    "SpglibTestData/tetragonal/POSCAR-088",
    "SpglibTestData/tetragonal/POSCAR-088-2",
    "SpglibTestData/tetragonal/POSCAR-090",
    "SpglibTestData/tetragonal/POSCAR-090-2",
    "SpglibTestData/tetragonal/POSCAR-091",
    "SpglibTestData/tetragonal/POSCAR-091-2",
    "SpglibTestData/tetragonal/POSCAR-092",
    "SpglibTestData/tetragonal/POSCAR-092-2",
    "SpglibTestData/tetragonal/POSCAR-092-3",
    "SpglibTestData/tetragonal/POSCAR-094",
    "SpglibTestData/tetragonal/POSCAR-094-2",
    "SpglibTestData/tetragonal/POSCAR-094-3",
    "SpglibTestData/tetragonal/POSCAR-095",
    "SpglibTestData/tetragonal/POSCAR-095-2",
    "SpglibTestData/tetragonal/POSCAR-096",
    "SpglibTestData/tetragonal/POSCAR-096-2",
    "SpglibTestData/tetragonal/POSCAR-097",
    "SpglibTestData/tetragonal/POSCAR-097-2",
    "SpglibTestData/tetragonal/POSCAR-098",
    "SpglibTestData/tetragonal/POSCAR-098-2",
    "SpglibTestData/tetragonal/POSCAR-099",
    "SpglibTestData/tetragonal/POSCAR-099-2",
    "SpglibTestData/tetragonal/POSCAR-100",
    "SpglibTestData/tetragonal/POSCAR-100-2",
    "SpglibTestData/tetragonal/POSCAR-102",
    "SpglibTestData/tetragonal/POSCAR-102-2",
    "SpglibTestData/tetragonal/POSCAR-103",
    "SpglibTestData/tetragonal/POSCAR-103-2",
    "SpglibTestData/tetragonal/POSCAR-104",
    "SpglibTestData/tetragonal/POSCAR-104-2",
    "SpglibTestData/tetragonal/POSCAR-105",
    "SpglibTestData/tetragonal/POSCAR-105-2",
    "SpglibTestData/tetragonal/POSCAR-106",
    "SpglibTestData/tetragonal/POSCAR-107",
    "SpglibTestData/tetragonal/POSCAR-107-2",
    "SpglibTestData/tetragonal/POSCAR-107-3",
    "SpglibTestData/tetragonal/POSCAR-108",
    "SpglibTestData/tetragonal/POSCAR-108-2",
    "SpglibTestData/tetragonal/POSCAR-109",
    "SpglibTestData/tetragonal/POSCAR-109-2",
    "SpglibTestData/tetragonal/POSCAR-110",
    "SpglibTestData/tetragonal/POSCAR-110-2",
    "SpglibTestData/tetragonal/POSCAR-111",
    "SpglibTestData/tetragonal/POSCAR-111-2",
    "SpglibTestData/tetragonal/POSCAR-112",
    "SpglibTestData/tetragonal/POSCAR-112-2",
    "SpglibTestData/tetragonal/POSCAR-113",
    "SpglibTestData/tetragonal/POSCAR-113-2",
    "SpglibTestData/tetragonal/POSCAR-114",
    "SpglibTestData/tetragonal/POSCAR-114-2",
    "SpglibTestData/tetragonal/POSCAR-115",
    "SpglibTestData/tetragonal/POSCAR-115-2",
    "SpglibTestData/tetragonal/POSCAR-115-3",
    "SpglibTestData/tetragonal/POSCAR-115-4",
    "SpglibTestData/tetragonal/POSCAR-115-5",
    "SpglibTestData/tetragonal/POSCAR-116",
    "SpglibTestData/tetragonal/POSCAR-116-2",
    "SpglibTestData/tetragonal/POSCAR-117",
    "SpglibTestData/tetragonal/POSCAR-117-2",
    "SpglibTestData/tetragonal/POSCAR-118",
    "SpglibTestData/tetragonal/POSCAR-118-2",
    "SpglibTestData/tetragonal/POSCAR-119",
    "SpglibTestData/tetragonal/POSCAR-119-2",
    "SpglibTestData/tetragonal/POSCAR-120",
    "SpglibTestData/tetragonal/POSCAR-120-2",
    "SpglibTestData/tetragonal/POSCAR-121",
    "SpglibTestData/tetragonal/POSCAR-121-2",
    "SpglibTestData/tetragonal/POSCAR-122",
    "SpglibTestData/tetragonal/POSCAR-122-2",
    "SpglibTestData/tetragonal/POSCAR-122-3",
    "SpglibTestData/tetragonal/POSCAR-123",
    "SpglibTestData/tetragonal/POSCAR-123-2",
    "SpglibTestData/tetragonal/POSCAR-123-3",
    "SpglibTestData/tetragonal/POSCAR-124",
    "SpglibTestData/tetragonal/POSCAR-124-2",
    "SpglibTestData/tetragonal/POSCAR-125",
    "SpglibTestData/tetragonal/POSCAR-125-2",
    "SpglibTestData/tetragonal/POSCAR-126",
    "SpglibTestData/tetragonal/POSCAR-126-2",
    "SpglibTestData/tetragonal/POSCAR-127",
    "SpglibTestData/tetragonal/POSCAR-127-2",
    "SpglibTestData/tetragonal/POSCAR-128",
    "SpglibTestData/tetragonal/POSCAR-128-2",
    "SpglibTestData/tetragonal/POSCAR-129",
    "SpglibTestData/tetragonal/POSCAR-129-2",
    "SpglibTestData/tetragonal/POSCAR-129-3",
    "SpglibTestData/tetragonal/POSCAR-130",
    "SpglibTestData/tetragonal/POSCAR-130-2",
    "SpglibTestData/tetragonal/POSCAR-131",
    "SpglibTestData/tetragonal/POSCAR-131-2",
    "SpglibTestData/tetragonal/POSCAR-132",
    "SpglibTestData/tetragonal/POSCAR-132-2",
    "SpglibTestData/tetragonal/POSCAR-133",
    "SpglibTestData/tetragonal/POSCAR-133-2",
    "SpglibTestData/tetragonal/POSCAR-134",
    "SpglibTestData/tetragonal/POSCAR-134-2",
    "SpglibTestData/tetragonal/POSCAR-135",
    "SpglibTestData/tetragonal/POSCAR-135-2",
    "SpglibTestData/tetragonal/POSCAR-136",
    "SpglibTestData/tetragonal/POSCAR-136-2",
    "SpglibTestData/tetragonal/POSCAR-136-3",
    "SpglibTestData/tetragonal/POSCAR-136-4",
    "SpglibTestData/tetragonal/POSCAR-136-5",
    "SpglibTestData/tetragonal/POSCAR-137",
    "SpglibTestData/tetragonal/POSCAR-137-2",
    "SpglibTestData/tetragonal/POSCAR-137-3",
    "SpglibTestData/tetragonal/POSCAR-138",
    "SpglibTestData/tetragonal/POSCAR-138-2",
    "SpglibTestData/tetragonal/POSCAR-139",
    "SpglibTestData/tetragonal/POSCAR-139-2",
    "SpglibTestData/tetragonal/POSCAR-140",
    "SpglibTestData/tetragonal/POSCAR-140-2",
    "SpglibTestData/tetragonal/POSCAR-141",
    "SpglibTestData/tetragonal/POSCAR-141-2",
    "SpglibTestData/tetragonal/POSCAR-142",
    "SpglibTestData/tetragonal/POSCAR-142-2",
    "SpglibTestData/tetragonal/POSCAR-142-3"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}


void TestTransformationMatrix::test_case_trigonal()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/trigonal/POSCAR-143",
    "SpglibTestData/trigonal/POSCAR-143-2",
    "SpglibTestData/trigonal/POSCAR-144",
    "SpglibTestData/trigonal/POSCAR-144-2",
    "SpglibTestData/trigonal/POSCAR-145",
    "SpglibTestData/trigonal/POSCAR-145-2",
    "SpglibTestData/trigonal/POSCAR-146",
    "SpglibTestData/trigonal/POSCAR-146-2",
    "SpglibTestData/trigonal/POSCAR-147",
    "SpglibTestData/trigonal/POSCAR-147-2",
    "SpglibTestData/trigonal/POSCAR-148",
    "SpglibTestData/trigonal/POSCAR-148-2",
    "SpglibTestData/trigonal/POSCAR-149",
    "SpglibTestData/trigonal/POSCAR-149-2",
    "SpglibTestData/trigonal/POSCAR-150",
    "SpglibTestData/trigonal/POSCAR-150-2",
    "SpglibTestData/trigonal/POSCAR-151",
    "SpglibTestData/trigonal/POSCAR-151-2",
    "SpglibTestData/trigonal/POSCAR-152",
    "SpglibTestData/trigonal/POSCAR-152-2",
    "SpglibTestData/trigonal/POSCAR-153",
    "SpglibTestData/trigonal/POSCAR-154",
    "SpglibTestData/trigonal/POSCAR-154-2",
    "SpglibTestData/trigonal/POSCAR-154-3",
    "SpglibTestData/trigonal/POSCAR-155",
    "SpglibTestData/trigonal/POSCAR-155-2",
    "SpglibTestData/trigonal/POSCAR-156",
    "SpglibTestData/trigonal/POSCAR-156-2",
    "SpglibTestData/trigonal/POSCAR-157",
    "SpglibTestData/trigonal/POSCAR-157-2",
    "SpglibTestData/trigonal/POSCAR-158",
    "SpglibTestData/trigonal/POSCAR-158-2",
    "SpglibTestData/trigonal/POSCAR-159",
    "SpglibTestData/trigonal/POSCAR-159-2",
    "SpglibTestData/trigonal/POSCAR-160",
    "SpglibTestData/trigonal/POSCAR-160-2",
    "SpglibTestData/trigonal/POSCAR-161",
    "SpglibTestData/trigonal/POSCAR-161-2",
    "SpglibTestData/trigonal/POSCAR-162",
    "SpglibTestData/trigonal/POSCAR-162-2",
    "SpglibTestData/trigonal/POSCAR-163",
    "SpglibTestData/trigonal/POSCAR-163-2",
    "SpglibTestData/trigonal/POSCAR-164",
    "SpglibTestData/trigonal/POSCAR-164-2",
    "SpglibTestData/trigonal/POSCAR-165",
    "SpglibTestData/trigonal/POSCAR-165-2",
    "SpglibTestData/trigonal/POSCAR-166",
    "SpglibTestData/trigonal/POSCAR-166-2",
    "SpglibTestData/trigonal/POSCAR-167",
    "SpglibTestData/trigonal/POSCAR-167-2",
    "SpglibTestData/trigonal/POSCAR-167-3"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}

void TestTransformationMatrix::test_case_hexagonal()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/hexagonal/POSCAR-168",
    "SpglibTestData/hexagonal/POSCAR-169",
    "SpglibTestData/hexagonal/POSCAR-169-2",
    "SpglibTestData/hexagonal/POSCAR-170",
    "SpglibTestData/hexagonal/POSCAR-170-2",
    "SpglibTestData/hexagonal/POSCAR-171",
    "SpglibTestData/hexagonal/POSCAR-171-2",
    "SpglibTestData/hexagonal/POSCAR-172",
    "SpglibTestData/hexagonal/POSCAR-173",
    "SpglibTestData/hexagonal/POSCAR-173-2",
    "SpglibTestData/hexagonal/POSCAR-174",
    "SpglibTestData/hexagonal/POSCAR-174-2",
    "SpglibTestData/hexagonal/POSCAR-175",
    "SpglibTestData/hexagonal/POSCAR-175-2",
    "SpglibTestData/hexagonal/POSCAR-176",
    "SpglibTestData/hexagonal/POSCAR-176-2",
    "SpglibTestData/hexagonal/POSCAR-177",
    "SpglibTestData/hexagonal/POSCAR-179",
    "SpglibTestData/hexagonal/POSCAR-179-2",
    "SpglibTestData/hexagonal/POSCAR-180",
    "SpglibTestData/hexagonal/POSCAR-180-2",
    "SpglibTestData/hexagonal/POSCAR-181",
    "SpglibTestData/hexagonal/POSCAR-181-2",
    "SpglibTestData/hexagonal/POSCAR-182",
    "SpglibTestData/hexagonal/POSCAR-182-2",
    "SpglibTestData/hexagonal/POSCAR-183",
    "SpglibTestData/hexagonal/POSCAR-183-2",
    "SpglibTestData/hexagonal/POSCAR-184",
    "SpglibTestData/hexagonal/POSCAR-184-2",
    "SpglibTestData/hexagonal/POSCAR-185",
    "SpglibTestData/hexagonal/POSCAR-185-2",
    "SpglibTestData/hexagonal/POSCAR-186",
    "SpglibTestData/hexagonal/POSCAR-186-2",
    "SpglibTestData/hexagonal/POSCAR-187",
    "SpglibTestData/hexagonal/POSCAR-187-2",
    "SpglibTestData/hexagonal/POSCAR-188",
    "SpglibTestData/hexagonal/POSCAR-188-2",
    "SpglibTestData/hexagonal/POSCAR-189",
    "SpglibTestData/hexagonal/POSCAR-189-2",
    "SpglibTestData/hexagonal/POSCAR-190",
    "SpglibTestData/hexagonal/POSCAR-190-2",
    "SpglibTestData/hexagonal/POSCAR-191",
    "SpglibTestData/hexagonal/POSCAR-191-2",
    "SpglibTestData/hexagonal/POSCAR-192",
    "SpglibTestData/hexagonal/POSCAR-192-2",
    "SpglibTestData/hexagonal/POSCAR-193",
    "SpglibTestData/hexagonal/POSCAR-193-2",
    "SpglibTestData/hexagonal/POSCAR-194",
    "SpglibTestData/hexagonal/POSCAR-194-2"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}


void TestTransformationMatrix::test_case_cubic()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/cubic/POSCAR-195",
    "SpglibTestData/cubic/POSCAR-195-2",
    "SpglibTestData/cubic/POSCAR-196",
    "SpglibTestData/cubic/POSCAR-196-2",
    "SpglibTestData/cubic/POSCAR-197",
    "SpglibTestData/cubic/POSCAR-197-2",
    "SpglibTestData/cubic/POSCAR-198",
    "SpglibTestData/cubic/POSCAR-198-2",
    "SpglibTestData/cubic/POSCAR-199",
    "SpglibTestData/cubic/POSCAR-199-2",
    "SpglibTestData/cubic/POSCAR-200",
    "SpglibTestData/cubic/POSCAR-200-2",
    "SpglibTestData/cubic/POSCAR-205",
    "SpglibTestData/cubic/POSCAR-205-3",
    "SpglibTestData/cubic/POSCAR-206",
    "SpglibTestData/cubic/POSCAR-206-2",
    "SpglibTestData/cubic/POSCAR-207",
    "SpglibTestData/cubic/POSCAR-208",
    "SpglibTestData/cubic/POSCAR-208-2",
    "SpglibTestData/cubic/POSCAR-209",
    "SpglibTestData/cubic/POSCAR-210",
    "SpglibTestData/cubic/POSCAR-210-2",
    "SpglibTestData/cubic/POSCAR-211",
    "SpglibTestData/cubic/POSCAR-212",
    "SpglibTestData/cubic/POSCAR-212-2",
    "SpglibTestData/cubic/POSCAR-213",
    "SpglibTestData/cubic/POSCAR-213-2",
    "SpglibTestData/cubic/POSCAR-214",
    "SpglibTestData/cubic/POSCAR-214-2",
    "SpglibTestData/cubic/POSCAR-215",
    "SpglibTestData/cubic/POSCAR-215-2",
    "SpglibTestData/cubic/POSCAR-216",
    "SpglibTestData/cubic/POSCAR-216-2",
    "SpglibTestData/cubic/POSCAR-217",
    "SpglibTestData/cubic/POSCAR-217-2",
    "SpglibTestData/cubic/POSCAR-218",
    "SpglibTestData/cubic/POSCAR-218-2",
    "SpglibTestData/cubic/POSCAR-219",
    "SpglibTestData/cubic/POSCAR-219-2",
    "SpglibTestData/cubic/POSCAR-220",
    "SpglibTestData/cubic/POSCAR-220-2",
    "SpglibTestData/cubic/POSCAR-221",
    "SpglibTestData/cubic/POSCAR-221-2",
    "SpglibTestData/cubic/POSCAR-222",
    "SpglibTestData/cubic/POSCAR-222-2",
    "SpglibTestData/cubic/POSCAR-223",
    "SpglibTestData/cubic/POSCAR-223-2",
    "SpglibTestData/cubic/POSCAR-224",
    "SpglibTestData/cubic/POSCAR-224-2",
    "SpglibTestData/cubic/POSCAR-225",
    "SpglibTestData/cubic/POSCAR-225-2",
    "SpglibTestData/cubic/POSCAR-226",
    "SpglibTestData/cubic/POSCAR-226-2",
    "SpglibTestData/cubic/POSCAR-227",
    "SpglibTestData/cubic/POSCAR-227-2",
    "SpglibTestData/cubic/POSCAR-228",
    "SpglibTestData/cubic/POSCAR-228-2",
    "SpglibTestData/cubic/POSCAR-229",
    "SpglibTestData/cubic/POSCAR-229-2",
    "SpglibTestData/cubic/POSCAR-230",
    "SpglibTestData/cubic/POSCAR-230-2",
    "SpglibTestData/cubic/POSCAR-230-3",
    "SpglibTestData/cubic/POSCAR-230-4"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}

void TestTransformationMatrix::test_case_virtual()
{
  srand((unsigned) time(nullptr));

  const std::vector<QString> testData =
  {
    "SpglibTestData/virtual_structure/POSCAR-1-221-33",
    "SpglibTestData/virtual_structure/POSCAR-1-222-33",
    "SpglibTestData/virtual_structure/POSCAR-1-223-33",
    "SpglibTestData/virtual_structure/POSCAR-1-224-33",
    "SpglibTestData/virtual_structure/POSCAR-1-227-73",
    "SpglibTestData/virtual_structure/POSCAR-1-227-93",
    "SpglibTestData/virtual_structure/POSCAR-1-227-99",
    "SpglibTestData/virtual_structure/POSCAR-1-230-conv-56",
    "SpglibTestData/virtual_structure/POSCAR-1-230-prim-33",
    "SpglibTestData/virtual_structure/POSCAR-1-bcc-33",
    "SpglibTestData/virtual_structure/POSCAR-10-221-18",
    "SpglibTestData/virtual_structure/POSCAR-10-223-18",
    "SpglibTestData/virtual_structure/POSCAR-10-227-50",
    "SpglibTestData/virtual_structure/POSCAR-102-224-13",
    "SpglibTestData/virtual_structure/POSCAR-104-222-13",
    "SpglibTestData/virtual_structure/POSCAR-105-223-13",
    "SpglibTestData/virtual_structure/POSCAR-109-227-13",
    "SpglibTestData/virtual_structure/POSCAR-11-227-48",
    "SpglibTestData/virtual_structure/POSCAR-110-230-conv-15",
    "SpglibTestData/virtual_structure/POSCAR-110-230-prim-13",
    "SpglibTestData/virtual_structure/POSCAR-111-221-11",
    "SpglibTestData/virtual_structure/POSCAR-111-224-11",
    "SpglibTestData/virtual_structure/POSCAR-111-227-66",
    "SpglibTestData/virtual_structure/POSCAR-112-222-11",
    "SpglibTestData/virtual_structure/POSCAR-112-223-11",
    "SpglibTestData/virtual_structure/POSCAR-113-227-68",
    "SpglibTestData/virtual_structure/POSCAR-115-221-14",
    "SpglibTestData/virtual_structure/POSCAR-115-223-14",
    "SpglibTestData/virtual_structure/POSCAR-115-227-33",
    "SpglibTestData/virtual_structure/POSCAR-116-230-conv-34",
    "SpglibTestData/virtual_structure/POSCAR-117-230-conv-33",
    "SpglibTestData/virtual_structure/POSCAR-118-222-14",
    "SpglibTestData/virtual_structure/POSCAR-118-224-14",
    "SpglibTestData/virtual_structure/POSCAR-12-221-19",
    "SpglibTestData/virtual_structure/POSCAR-12-224-19",
    "SpglibTestData/virtual_structure/POSCAR-12-227-21",
    "SpglibTestData/virtual_structure/POSCAR-12-227-83",
    "SpglibTestData/virtual_structure/POSCAR-120-230-conv-16",
    "SpglibTestData/virtual_structure/POSCAR-120-230-prim-14",
    "SpglibTestData/virtual_structure/POSCAR-122-230-conv-13",
    "SpglibTestData/virtual_structure/POSCAR-122-230-prim-11",
    "SpglibTestData/virtual_structure/POSCAR-123-221-05",
    "SpglibTestData/virtual_structure/POSCAR-126-222-05",
    "SpglibTestData/virtual_structure/POSCAR-13-222-18",
    "SpglibTestData/virtual_structure/POSCAR-13-224-18",
    "SpglibTestData/virtual_structure/POSCAR-13-227-49",
    "SpglibTestData/virtual_structure/POSCAR-13-230-conv-44",
    "SpglibTestData/virtual_structure/POSCAR-131-223-05",
    "SpglibTestData/virtual_structure/POSCAR-134-224-05",
    "SpglibTestData/virtual_structure/POSCAR-14-227-47",
    "SpglibTestData/virtual_structure/POSCAR-14-227-51",
    "SpglibTestData/virtual_structure/POSCAR-14-230-conv-45",
    "SpglibTestData/virtual_structure/POSCAR-142-230-conv-05",
    "SpglibTestData/virtual_structure/POSCAR-142-230-prim-05",
    "SpglibTestData/virtual_structure/POSCAR-146-221-27",
    "SpglibTestData/virtual_structure/POSCAR-146-222-27",
    "SpglibTestData/virtual_structure/POSCAR-146-223-27",
    "SpglibTestData/virtual_structure/POSCAR-146-224-27",
    "SpglibTestData/virtual_structure/POSCAR-146-227-92",
    "SpglibTestData/virtual_structure/POSCAR-146-230-conv-36",
    "SpglibTestData/virtual_structure/POSCAR-146-230-conv-55",
    "SpglibTestData/virtual_structure/POSCAR-146-230-prim-27",
    "SpglibTestData/virtual_structure/POSCAR-146-bcc-27",
    "SpglibTestData/virtual_structure/POSCAR-148-221-15",
    "SpglibTestData/virtual_structure/POSCAR-148-222-15",
    "SpglibTestData/virtual_structure/POSCAR-148-223-15",
    "SpglibTestData/virtual_structure/POSCAR-148-224-15",
    "SpglibTestData/virtual_structure/POSCAR-148-227-70",
    "SpglibTestData/virtual_structure/POSCAR-148-230-conv-17",
    "SpglibTestData/virtual_structure/POSCAR-148-230-conv-37",
    "SpglibTestData/virtual_structure/POSCAR-148-230-prim-15",
    "SpglibTestData/virtual_structure/POSCAR-148-bcc-15",
    "SpglibTestData/virtual_structure/POSCAR-15-222-19",
    "SpglibTestData/virtual_structure/POSCAR-15-223-19",
    "SpglibTestData/virtual_structure/POSCAR-15-230-conv-21",
    "SpglibTestData/virtual_structure/POSCAR-15-230-conv-22",
    "SpglibTestData/virtual_structure/POSCAR-15-230-prim-18",
    "SpglibTestData/virtual_structure/POSCAR-15-230-prim-19",
    "SpglibTestData/virtual_structure/POSCAR-15-bcc-18",
    "SpglibTestData/virtual_structure/POSCAR-15-bcc-19",
    "SpglibTestData/virtual_structure/POSCAR-155-221-17",
    "SpglibTestData/virtual_structure/POSCAR-155-222-17",
    "SpglibTestData/virtual_structure/POSCAR-155-223-17",
    "SpglibTestData/virtual_structure/POSCAR-155-224-17",
    "SpglibTestData/virtual_structure/POSCAR-155-227-72",
    "SpglibTestData/virtual_structure/POSCAR-155-230-conv-19",
    "SpglibTestData/virtual_structure/POSCAR-155-230-conv-38",
    "SpglibTestData/virtual_structure/POSCAR-155-230-prim-17",
    "SpglibTestData/virtual_structure/POSCAR-155-bcc-17",
    "SpglibTestData/virtual_structure/POSCAR-16-221-20",
    "SpglibTestData/virtual_structure/POSCAR-16-222-20",
    "SpglibTestData/virtual_structure/POSCAR-16-223-20",
    "SpglibTestData/virtual_structure/POSCAR-16-224-20",
    "SpglibTestData/virtual_structure/POSCAR-16-227-84",
    "SpglibTestData/virtual_structure/POSCAR-160-221-16",
    "SpglibTestData/virtual_structure/POSCAR-160-224-16",
    "SpglibTestData/virtual_structure/POSCAR-160-227-16",
    "SpglibTestData/virtual_structure/POSCAR-160-227-71",
    "SpglibTestData/virtual_structure/POSCAR-160-fcc",
    "SpglibTestData/virtual_structure/POSCAR-161-222-16",
    "SpglibTestData/virtual_structure/POSCAR-161-223-16",
    "SpglibTestData/virtual_structure/POSCAR-161-230-conv-18",
    "SpglibTestData/virtual_structure/POSCAR-161-230-prim-16",
    "SpglibTestData/virtual_structure/POSCAR-161-bcc-16",
    "SpglibTestData/virtual_structure/POSCAR-166-221-06",
    "SpglibTestData/virtual_structure/POSCAR-166-224-06",
    "SpglibTestData/virtual_structure/POSCAR-166-227-06",
    "SpglibTestData/virtual_structure/POSCAR-166-227-38",
    "SpglibTestData/virtual_structure/POSCAR-167-222-06",
    "SpglibTestData/virtual_structure/POSCAR-167-223-06",
    "SpglibTestData/virtual_structure/POSCAR-167-230-conv-06",
    "SpglibTestData/virtual_structure/POSCAR-167-230-prim-06",
    "SpglibTestData/virtual_structure/POSCAR-167-bcc-6",
    "SpglibTestData/virtual_structure/POSCAR-17-227-60",
    "SpglibTestData/virtual_structure/POSCAR-17-227-85",
    "SpglibTestData/virtual_structure/POSCAR-17-230-conv-46",
    "SpglibTestData/virtual_structure/POSCAR-18-227-86",
    "SpglibTestData/virtual_structure/POSCAR-19-227-59",
    "SpglibTestData/virtual_structure/POSCAR-19-227-89",
    "SpglibTestData/virtual_structure/POSCAR-19-230-conv-51",
    "SpglibTestData/virtual_structure/POSCAR-195-221-07",
    "SpglibTestData/virtual_structure/POSCAR-195-222-07",
    "SpglibTestData/virtual_structure/POSCAR-195-223-07",
    "SpglibTestData/virtual_structure/POSCAR-195-224-07",
    "SpglibTestData/virtual_structure/POSCAR-198-227-40",
    "SpglibTestData/virtual_structure/POSCAR-198-230-conv-20",
    "SpglibTestData/virtual_structure/POSCAR-199-230-conv-07",
    "SpglibTestData/virtual_structure/POSCAR-199-230-prim-07",
    "SpglibTestData/virtual_structure/POSCAR-2-221-28",
    "SpglibTestData/virtual_structure/POSCAR-2-222-28",
    "SpglibTestData/virtual_structure/POSCAR-2-223-28",
    "SpglibTestData/virtual_structure/POSCAR-2-224-28",
    "SpglibTestData/virtual_structure/POSCAR-2-227-41",
    "SpglibTestData/virtual_structure/POSCAR-2-227-74",
    "SpglibTestData/virtual_structure/POSCAR-2-227-94",
    "SpglibTestData/virtual_structure/POSCAR-2-230-conv-39",
    "SpglibTestData/virtual_structure/POSCAR-2-230-conv-57",
    "SpglibTestData/virtual_structure/POSCAR-2-230-prim-28",
    "SpglibTestData/virtual_structure/POSCAR-2-bcc-28",
    "SpglibTestData/virtual_structure/POSCAR-20-227-53",
    "SpglibTestData/virtual_structure/POSCAR-20-227-90",
    "SpglibTestData/virtual_structure/POSCAR-20-230-conv-53",
    "SpglibTestData/virtual_structure/POSCAR-200-221-02",
    "SpglibTestData/virtual_structure/POSCAR-200-223-02",
    "SpglibTestData/virtual_structure/POSCAR-201-222-02",
    "SpglibTestData/virtual_structure/POSCAR-201-224-02",
    "SpglibTestData/virtual_structure/POSCAR-205-230-conv-08",
    "SpglibTestData/virtual_structure/POSCAR-206-230-conv-02",
    "SpglibTestData/virtual_structure/POSCAR-206-230-prim-02",
    "SpglibTestData/virtual_structure/POSCAR-207-221-04",
    "SpglibTestData/virtual_structure/POSCAR-207-222-04",
    "SpglibTestData/virtual_structure/POSCAR-208-223-04",
    "SpglibTestData/virtual_structure/POSCAR-208-224-04",
    "SpglibTestData/virtual_structure/POSCAR-21-221-23",
    "SpglibTestData/virtual_structure/POSCAR-21-222-23",
    "SpglibTestData/virtual_structure/POSCAR-21-223-23",
    "SpglibTestData/virtual_structure/POSCAR-21-224-23",
    "SpglibTestData/virtual_structure/POSCAR-21-230-conv-49",
    "SpglibTestData/virtual_structure/POSCAR-212-227-19",
    "SpglibTestData/virtual_structure/POSCAR-213-230-conv-09",
    "SpglibTestData/virtual_structure/POSCAR-214-230-conv-04",
    "SpglibTestData/virtual_structure/POSCAR-214-230-prim-04",
    "SpglibTestData/virtual_structure/POSCAR-215-221-03",
    "SpglibTestData/virtual_structure/POSCAR-215-224-03",
    "SpglibTestData/virtual_structure/POSCAR-215-227-18",
    "SpglibTestData/virtual_structure/POSCAR-216-227-03",
    "SpglibTestData/virtual_structure/POSCAR-218-222-03",
    "SpglibTestData/virtual_structure/POSCAR-218-223-03",
    "SpglibTestData/virtual_structure/POSCAR-22-230-conv-26",
    "SpglibTestData/virtual_structure/POSCAR-22-230-prim-23",
    "SpglibTestData/virtual_structure/POSCAR-220-230-conv-03",
    "SpglibTestData/virtual_structure/POSCAR-220-230-prim-03",
    "SpglibTestData/virtual_structure/POSCAR-221-221-01",
    "SpglibTestData/virtual_structure/POSCAR-222-222-01",
    "SpglibTestData/virtual_structure/POSCAR-223-223-01",
    "SpglibTestData/virtual_structure/POSCAR-224-224-01",
    "SpglibTestData/virtual_structure/POSCAR-227-227-01",
    "SpglibTestData/virtual_structure/POSCAR-230-230-conv-01",
    "SpglibTestData/virtual_structure/POSCAR-230-230-conv-62",
    "SpglibTestData/virtual_structure/POSCAR-230-230-prim-01",
    "SpglibTestData/virtual_structure/POSCAR-24-230-conv-23",
    "SpglibTestData/virtual_structure/POSCAR-24-230-prim-20",
    "SpglibTestData/virtual_structure/POSCAR-25-221-21",
    "SpglibTestData/virtual_structure/POSCAR-25-223-21",
    "SpglibTestData/virtual_structure/POSCAR-25-227-54",
    "SpglibTestData/virtual_structure/POSCAR-26-227-64",
    "SpglibTestData/virtual_structure/POSCAR-27-230-conv-48",
    "SpglibTestData/virtual_structure/POSCAR-28-227-62",
    "SpglibTestData/virtual_structure/POSCAR-29-230-conv-52",
    "SpglibTestData/virtual_structure/POSCAR-3-221-29",
    "SpglibTestData/virtual_structure/POSCAR-3-222-29",
    "SpglibTestData/virtual_structure/POSCAR-3-223-29",
    "SpglibTestData/virtual_structure/POSCAR-3-224-29",
    "SpglibTestData/virtual_structure/POSCAR-3-227-82",
    "SpglibTestData/virtual_structure/POSCAR-3-227-95",
    "SpglibTestData/virtual_structure/POSCAR-3-230-conv-58",
    "SpglibTestData/virtual_structure/POSCAR-30-227-65",
    "SpglibTestData/virtual_structure/POSCAR-31-227-58",
    "SpglibTestData/virtual_structure/POSCAR-32-230-conv-47",
    "SpglibTestData/virtual_structure/POSCAR-33-227-63",
    "SpglibTestData/virtual_structure/POSCAR-34-222-21",
    "SpglibTestData/virtual_structure/POSCAR-34-224-21",
    "SpglibTestData/virtual_structure/POSCAR-35-221-22",
    "SpglibTestData/virtual_structure/POSCAR-35-224-22",
    "SpglibTestData/virtual_structure/POSCAR-35-227-87",
    "SpglibTestData/virtual_structure/POSCAR-37-222-22",
    "SpglibTestData/virtual_structure/POSCAR-37-223-22",
    "SpglibTestData/virtual_structure/POSCAR-38-221-26",
    "SpglibTestData/virtual_structure/POSCAR-39-224-26",
    "SpglibTestData/virtual_structure/POSCAR-4-227-77",
    "SpglibTestData/virtual_structure/POSCAR-4-227-81",
    "SpglibTestData/virtual_structure/POSCAR-4-227-96",
    "SpglibTestData/virtual_structure/POSCAR-4-230-conv-59",
    "SpglibTestData/virtual_structure/POSCAR-40-223-26",
    "SpglibTestData/virtual_structure/POSCAR-41-222-26",
    "SpglibTestData/virtual_structure/POSCAR-43-230-conv-25",
    "SpglibTestData/virtual_structure/POSCAR-43-230-conv-29",
    "SpglibTestData/virtual_structure/POSCAR-43-230-prim-22",
    "SpglibTestData/virtual_structure/POSCAR-43-230-prim-26",
    "SpglibTestData/virtual_structure/POSCAR-43-bcc-22",
    "SpglibTestData/virtual_structure/POSCAR-43-bcc-26",
    "SpglibTestData/virtual_structure/POSCAR-44-227-24",
    "SpglibTestData/virtual_structure/POSCAR-45-230-conv-24",
    "SpglibTestData/virtual_structure/POSCAR-45-230-prim-21",
    "SpglibTestData/virtual_structure/POSCAR-46-227-28",
    "SpglibTestData/virtual_structure/POSCAR-47-221-08",
    "SpglibTestData/virtual_structure/POSCAR-47-223-08",
    "SpglibTestData/virtual_structure/POSCAR-48-222-08",
    "SpglibTestData/virtual_structure/POSCAR-48-224-08",
    "SpglibTestData/virtual_structure/POSCAR-5-221-32",
    "SpglibTestData/virtual_structure/POSCAR-5-222-32",
    "SpglibTestData/virtual_structure/POSCAR-5-223-32",
    "SpglibTestData/virtual_structure/POSCAR-5-224-32",
    "SpglibTestData/virtual_structure/POSCAR-5-227-45",
    "SpglibTestData/virtual_structure/POSCAR-5-227-75",
    "SpglibTestData/virtual_structure/POSCAR-5-227-98",
    "SpglibTestData/virtual_structure/POSCAR-5-230-conv-40",
    "SpglibTestData/virtual_structure/POSCAR-5-230-conv-43",
    "SpglibTestData/virtual_structure/POSCAR-5-230-conv-61",
    "SpglibTestData/virtual_structure/POSCAR-5-230-prim-29",
    "SpglibTestData/virtual_structure/POSCAR-5-230-prim-32",
    "SpglibTestData/virtual_structure/POSCAR-5-bcc-29",
    "SpglibTestData/virtual_structure/POSCAR-5-bcc-32",
    "SpglibTestData/virtual_structure/POSCAR-51-227-29",
    "SpglibTestData/virtual_structure/POSCAR-53-227-32",
    "SpglibTestData/virtual_structure/POSCAR-54-230-conv-30",
    "SpglibTestData/virtual_structure/POSCAR-6-221-30",
    "SpglibTestData/virtual_structure/POSCAR-6-223-30",
    "SpglibTestData/virtual_structure/POSCAR-6-227-79",
    "SpglibTestData/virtual_structure/POSCAR-61-230-conv-31",
    "SpglibTestData/virtual_structure/POSCAR-62-227-31",
    "SpglibTestData/virtual_structure/POSCAR-65-221-09",
    "SpglibTestData/virtual_structure/POSCAR-66-223-09",
    "SpglibTestData/virtual_structure/POSCAR-67-224-09",
    "SpglibTestData/virtual_structure/POSCAR-68-222-09",
    "SpglibTestData/virtual_structure/POSCAR-7-222-30",
    "SpglibTestData/virtual_structure/POSCAR-7-224-30",
    "SpglibTestData/virtual_structure/POSCAR-7-227-78",
    "SpglibTestData/virtual_structure/POSCAR-7-227-80",
    "SpglibTestData/virtual_structure/POSCAR-7-230-conv-60",
    "SpglibTestData/virtual_structure/POSCAR-70-230-conv-11",
    "SpglibTestData/virtual_structure/POSCAR-70-230-prim-09",
    "SpglibTestData/virtual_structure/POSCAR-70-bcc-9",
    "SpglibTestData/virtual_structure/POSCAR-73-230-conv-10",
    "SpglibTestData/virtual_structure/POSCAR-73-230-prim-08",
    "SpglibTestData/virtual_structure/POSCAR-74-227-09",
    "SpglibTestData/virtual_structure/POSCAR-75-221-25",
    "SpglibTestData/virtual_structure/POSCAR-75-222-25",
    "SpglibTestData/virtual_structure/POSCAR-76-227-61",
    "SpglibTestData/virtual_structure/POSCAR-77-223-25",
    "SpglibTestData/virtual_structure/POSCAR-77-224-25",
    "SpglibTestData/virtual_structure/POSCAR-78-227-91",
    "SpglibTestData/virtual_structure/POSCAR-78-230-conv-54",
    "SpglibTestData/virtual_structure/POSCAR-8-221-31",
    "SpglibTestData/virtual_structure/POSCAR-8-224-31",
    "SpglibTestData/virtual_structure/POSCAR-8-227-44",
    "SpglibTestData/virtual_structure/POSCAR-8-227-97",
    "SpglibTestData/virtual_structure/POSCAR-80-230-conv-28",
    "SpglibTestData/virtual_structure/POSCAR-80-230-prim-25",
    "SpglibTestData/virtual_structure/POSCAR-81-221-24",
    "SpglibTestData/virtual_structure/POSCAR-81-222-24",
    "SpglibTestData/virtual_structure/POSCAR-81-223-24",
    "SpglibTestData/virtual_structure/POSCAR-81-224-24",
    "SpglibTestData/virtual_structure/POSCAR-81-227-88",
    "SpglibTestData/virtual_structure/POSCAR-81-230-conv-50",
    "SpglibTestData/virtual_structure/POSCAR-82-230-conv-27",
    "SpglibTestData/virtual_structure/POSCAR-82-230-prim-24",
    "SpglibTestData/virtual_structure/POSCAR-83-221-10",
    "SpglibTestData/virtual_structure/POSCAR-84-223-10",
    "SpglibTestData/virtual_structure/POSCAR-85-222-10",
    "SpglibTestData/virtual_structure/POSCAR-86-224-10",
    "SpglibTestData/virtual_structure/POSCAR-88-230-conv-12",
    "SpglibTestData/virtual_structure/POSCAR-88-230-prim-10",
    "SpglibTestData/virtual_structure/POSCAR-89-221-12",
    "SpglibTestData/virtual_structure/POSCAR-89-222-12",
    "SpglibTestData/virtual_structure/POSCAR-9-222-31",
    "SpglibTestData/virtual_structure/POSCAR-9-223-31",
    "SpglibTestData/virtual_structure/POSCAR-9-227-43",
    "SpglibTestData/virtual_structure/POSCAR-9-230-conv-41",
    "SpglibTestData/virtual_structure/POSCAR-9-230-conv-42",
    "SpglibTestData/virtual_structure/POSCAR-9-230-prim-30",
    "SpglibTestData/virtual_structure/POSCAR-9-230-prim-31",
    "SpglibTestData/virtual_structure/POSCAR-9-bcc-30",
    "SpglibTestData/virtual_structure/POSCAR-9-bcc-31",
    "SpglibTestData/virtual_structure/POSCAR-91-227-67",
    "SpglibTestData/virtual_structure/POSCAR-92-227-35",
    "SpglibTestData/virtual_structure/POSCAR-92-230-conv-35",
    "SpglibTestData/virtual_structure/POSCAR-93-223-12",
    "SpglibTestData/virtual_structure/POSCAR-93-224-12",
    "SpglibTestData/virtual_structure/POSCAR-95-227-36",
    "SpglibTestData/virtual_structure/POSCAR-95-230-conv-32",
    "SpglibTestData/virtual_structure/POSCAR-96-227-69",
    "SpglibTestData/virtual_structure/POSCAR-98-230-conv-14",
    "SpglibTestData/virtual_structure/POSCAR-98-230-prim-12",
    "SpglibTestData/virtual_structure/POSCAR-99-221-13"
  };


  for(const QString &fileName: testData)
  {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      QString fileContent = in.readAll();
      file.close();
      SKPOSCARLegacyParser parser = SKPOSCARLegacyParser(QString(fileContent), false, false, CharacterSet::whitespaceAndNewlineCharacterSet(), nullptr);
      parser.startParsing();

      std::vector<std::tuple<double3, int, double> > atoms = parser.firstTestFrame();
      double3x3 unitCell = parser.movies().front().front()->cell->unitCell();
      bool allowPartialOccupancies = true;
      double symmetryPrecision=1e-5;

      double3 randomShift = double3(0.5*(((double) rand() / RAND_MAX)-0.5), 0.5*(((double) rand() / RAND_MAX)-0.5),0.5*(((double) rand() / RAND_MAX)-0.5));
      std::vector<std::tuple<double3,int,double>> randomlyShiftedAtoms{};
      std::transform(atoms.begin(), atoms.end(),std::back_inserter(randomlyShiftedAtoms),
              [randomShift](const std::tuple<double3,int,double> &atom) { return std::make_tuple(std::get<0>(atom) + randomShift, std::get<1>(atom), std::get<2>(atom)); });

      std::optional<SKSpaceGroup::FoundSpaceGroupInfo> spaceGroup = SKSpaceGroup::findSpaceGroup(unitCell, randomlyShiftedAtoms, allowPartialOccupancies, symmetryPrecision);

      if(spaceGroup)
      {
        SKSymmetryCell cell = spaceGroup->cell;
        double3x3 transformationMatrix = spaceGroup->transformationMatrix;
        double3x3 rotationMatrix = spaceGroup->rotationMatrix;

        double3x3 originalUnitCell = rotationMatrix * cell.unitCell() * transformationMatrix;
        if(!(originalUnitCell == unitCell))
        {
          qDebug() << fileName << originalUnitCell << " vs. " << unitCell;
        }
        QCOMPARE(originalUnitCell, unitCell);
      }
      else
      {
        QFAIL((QString("\nSpaceGroup not found for file: ") + fileName).toStdString().c_str());
      }
    }
  }
}
