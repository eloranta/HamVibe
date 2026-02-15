#include <QtTest/QtTest>

QObject *createFrequencyLabelTest();
QObject *createRigTest();
QObject *createCountryTest();

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    int status = 0;
    QObject *freqTest = createFrequencyLabelTest();
    status |= QTest::qExec(freqTest, argc, argv);
    delete freqTest;

    QObject *rigTest = createRigTest();
    status |= QTest::qExec(rigTest, argc, argv);
    delete rigTest;

    QObject *countryTest = createCountryTest();
    status |= QTest::qExec(countryTest, argc, argv);
    delete countryTest;

    return status;
}
