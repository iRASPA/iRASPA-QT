#include <QTest>
#include "tst_symmetrykitsmallestprimitivecellnopartialoccupancies.h"
#include "tst_symmetrykitsmallestprimitivecell.h"


// Note: This is equivalent to QTEST_APPLESS_MAIN for multiple test classes.
int main(int argc, char** argv)
{
   int status = 0;
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
