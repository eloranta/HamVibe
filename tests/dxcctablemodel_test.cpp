#include <QtTest/QtTest>
#include <QSqlQuery>
#include <QSet>

#include "dxcctablemodel.h"

class DxccTableModelTest : public QObject
{
    Q_OBJECT

private slots:
    void staysUniqueAcrossDatabaseRebuilds();
};

void DxccTableModelTest::staysUniqueAcrossDatabaseRebuilds()
{
    const QString connection = "dxcc-model-test";
    {
        auto db = QSqlDatabase::addDatabase("QSQLITE", connection);
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("CREATE TABLE dxcc (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                           "Entity TEXT UNIQUE, Mix TEXT)"));
        auto seed = [&query] {
            if (!query.prepare("INSERT INTO dxcc (Entity) VALUES (?)")) {
                return false;
            }
            for (int i = 0; i < 340; ++i) {
                query.bindValue(0, QString("Entity %1").arg(i, 3, 10, QLatin1Char('0')));
                if (!query.exec()) {
                    return false;
                }
            }
            return true;
        };
        QVERIFY(seed());

        DxccTableModel model(nullptr, db);
        model.setTable("dxcc");
        model.setEditStrategy(QSqlTableModel::OnFieldChange);
        QVERIFY(model.select());
        QCOMPARE(model.rowCount(), 340);
        QVERIFY(!model.canFetchMore());

        // Reproduce an older app instance rebuilding the table while this
        // instance still displays it. A partially fetched model repeats rows.
        QVERIFY(query.exec("DELETE FROM dxcc"));
        QVERIFY(seed());
        model.fetchMore();
        QCOMPARE(model.rowCount(), 340);
        auto verifyUnique = [&model] {
            QSet<QString> entities;
            for (int row = 0; row < model.rowCount(); ++row) {
                entities.insert(model.index(row, model.fieldIndex("Entity")).data().toString());
            }
            return entities.size() == 340;
        };
        QVERIFY(verifyUnique());

        for (Qt::SortOrder order : {Qt::AscendingOrder, Qt::DescendingOrder}) {
            model.sort(model.fieldIndex("Entity"), order);
            QVERIFY(model.select());
            QCOMPARE(model.rowCount(), 340);
            QVERIFY(!model.canFetchMore());
            QVERIFY(verifyUnique());
        }
        QVERIFY(model.setData(model.index(0, model.fieldIndex("Mix")), "X"));
        QVERIFY(model.select());
        QCOMPARE(model.rowCount(), 340);
        QVERIFY(verifyUnique());
        QVERIFY(!model.insertRows(model.rowCount(), 1));
    }
    QSqlDatabase::removeDatabase(connection);
}

QObject *createDxccTableModelTest()
{
    return new DxccTableModelTest;
}

#include "dxcctablemodel_test.moc"
