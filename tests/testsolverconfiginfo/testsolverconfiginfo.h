#ifndef TESTSOLVERCONFIGINFO_H
#define TESTSOLVERCONFIGINFO_H

#include <QtTest/QTest>

class TestSolverConfigInfo : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testSolverConfigInfo();

    void testSolvers();

    void testSolverId();
    void testSolverIdLowerCase();
    void testSolverIdMixedCase();
    void testSolverIdInvalid();

    void testSolverName();
    void testSolverNameZeroIndex();
    void testSolverNameNegativeIndex();
    void testSolverNameOutOfRange();

    void testSolverNames();

    void testSolverOptDefFilename_data();
    void testSolverOptDefFilename();

    void testModelTypeNames();

    void testSolverCapability();
    void testSolverCapabilityInvalidSolver();
    void testSolverCapabilityInvalidModelType();
    void testSolverCapabilityInvalidSolverNegative();
    void testSolverCapabilityInvalidModelTypeNegative();
    void testSolverCapabilityBothInvalid();
    void testSolverCapabilityBothInvalidNegative();

};


#endif // TESTSOLVERCONFIGINFO_H
