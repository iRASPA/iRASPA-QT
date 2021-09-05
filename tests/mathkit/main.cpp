#include <QTest>
#include "tst_hnf.h"
#include "tst_smf.h"

// Note: This is equivalent to QTEST_APPLESS_MAIN for multiple test classes.
int main(int argc, char** argv)
{
   int status = 0;
   {
      HermiteNormalForm tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   {
      SmithNormalForm tc;
      status |= QTest::qExec(&tc, argc, argv);
   }
   return status;
}
