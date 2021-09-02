#include <QTest>
#include "tst_symmetrykitfindpointgroupnopartialoccupancies.h"
#include "tst_symmetrykitfindpointgroup.h"


// Note: This is equivalent to QTEST_APPLESS_MAIN for multiple test classes.
int main(int argc, char** argv)
{
   int status = 0;
   {
      FindPointGroupNoPartialOccupancies tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      FindPointGroup tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   return status;
}
