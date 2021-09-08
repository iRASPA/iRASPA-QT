#include <QTest>
#include "tst_symmetrykitfindspacegroupnopartialoccupancies.h"
#include "tst_symmetrykitfindspacegroup.h"
#include "tst_symmetrykitfindpointgroupnopartialoccupancies.h"
#include "tst_symmetrykitfindpointgroup.h"
#include "tst_symmetrykitfindconventionalcell.h"
#include "tst_symmetrykittransformationmatrix.h"
#include "tst_symmetrykitsmallestprimitivecellnopartialoccupancies.h"
#include "tst_symmetrykitsmallestprimitivecell.h"

// Note: This is equivalent to QTEST_APPLESS_MAIN for multiple test classes.
int main(int argc, char** argv)
{
   int status = 0;

   {
      TestTransformationMatrix tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      FindConventionalCell tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      FindSpaceGroup tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      FindSpaceGroupNoPartialOccupancies tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      FindPointGroupNoPartialOccupancies tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      FindPointGroup tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      SmallestPrimitiveCellNoPartialOccupancies tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      SmallestPrimitiveCell tc;
      status |= QTest::qExec(&tc, argc, argv);
   }

   return status;
}
