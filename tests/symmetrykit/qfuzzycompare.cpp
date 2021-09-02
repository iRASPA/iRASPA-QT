#include "qfuzzycompare.h"

bool qFuzzyCompare(double3x3 p1, double3x3 p2, double epsilon)
{
    return ((qAbs(p1.m11 - p2.m11) <= epsilon) &&
            (qAbs(p1.m12 - p2.m12) <= epsilon) &&
            (qAbs(p1.m13 - p2.m13) <= epsilon) &&
            (qAbs(p1.m21 - p2.m21) <= epsilon) &&
            (qAbs(p1.m22 - p2.m22) <= epsilon) &&
            (qAbs(p1.m23 - p2.m23) <= epsilon) &&
            (qAbs(p1.m31 - p2.m31) <= epsilon) &&
            (qAbs(p1.m32 - p2.m32) <= epsilon) &&
            (qAbs(p1.m33 - p2.m33) <= epsilon));
}
