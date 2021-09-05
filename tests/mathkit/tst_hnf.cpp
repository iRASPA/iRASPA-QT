#include <mathkit.h>
#include <stdlib.h>
#include "tst_hnf.h"

HermiteNormalForm::HermiteNormalForm()
{
}

HermiteNormalForm::~HermiteNormalForm()
{

}


void HermiteNormalForm::initTestCase()
{

}

void HermiteNormalForm::cleanupTestCase()
{

}

void HermiteNormalForm::test_case()
{
  for(int rows=2;rows<10;rows++)
  {
    for(int columns=2;columns<10;columns++)
    {
       for(int count=0;count<500;count++)
       {
         RingMatrix m = RingMatrix::createRandomRingMatrix(rows, rows, 4);
         std::tuple<RingMatrix,RingMatrix,std::vector<int>> hnf = m.HermiteNormalForm();
         const RingMatrix U = std::get<0>(hnf);
         const RingMatrix A = std::get<1>(hnf);
         if(!(U*m == A))
         {
           qDebug() << m;
           qDebug() << U*m;
           qDebug() << U;
           qDebug() << A;
         }
         QCOMPARE(U*m, A);
       }
     }
  }
}

