#include <mathkit.h>
#include <stdlib.h>
#include "tst_smf.h"

SmithNormalForm::SmithNormalForm()
{
}

SmithNormalForm::~SmithNormalForm()
{

}


void SmithNormalForm::initTestCase()
{

}

void SmithNormalForm::cleanupTestCase()
{

}

void SmithNormalForm::test_case()
{
  for(int rows=2;rows<10;rows++)
  {
    for(int columns=2;columns<10;columns++)
    {
       for(int count=0;count<500;count++)
       {
         RingMatrix m = RingMatrix::createRandomRingMatrix(rows, rows, 4);
         std::tuple<RingMatrix,RingMatrix,RingMatrix> smf = m.SmithNormalForm();
         const RingMatrix U = std::get<0>(smf);
         const RingMatrix V = std::get<1>(smf);
         const RingMatrix A = std::get<2>(smf);
         if(!(U * m * V == A))
         {
           qDebug() << U*m*V;
           qDebug() << A;
         }
         QCOMPARE(U * m * V, A);
       }
     }
  }
}
