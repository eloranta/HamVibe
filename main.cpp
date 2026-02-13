#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>

static bool setupDatabase(const QString &connectionName,
                          const QString &dbFile,
                          const QString &tableName,
                          const QStringList &calls)
{
    QSqlDatabase db = connectionName.isEmpty()
        ? QSqlDatabase::addDatabase("QSQLITE")
        : QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(dbFile);

    if (!db.open()) {
        qFatal("Cannot open database!");
        return false;
    }

    QSqlQuery query(db);

    const QString createSql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            callsign TEXT,
            "10" INTEGER,
            "12" INTEGER,
            "15" INTEGER,
            "17" INTEGER,
            "20" INTEGER,
            "30" INTEGER,
            "40" INTEGER,
            "80" INTEGER
        )
    )").arg(tableName);

    if (!query.exec(createSql)) {
        qWarning() << "Failed to create table:" << query.lastError();
        return false;
    }

    const QString countSql = QString("SELECT COUNT(*) FROM %1").arg(tableName);
    if (!query.exec(countSql)) {
        qWarning() << "Failed to count rows:" << query.lastError();
        return false;
    }

    if (query.next() && query.value(0).toInt() == 0) {
        const QString insertSql = QString(R"(
            INSERT INTO %1 (callsign, "10", "12", "15", "17", "20", "30", "40", "80")
            VALUES (?, 0, 0, 0, 0, 0, 0, 0, 0)
        )").arg(tableName);

        for (const QString &call : calls) {
            query.prepare(insertSql);
            query.addBindValue(call);
            if (!query.exec()) {
                qWarning() << "Insert failed for call:" << call;
            }
        }

        qDebug() << "Inserted" << calls.size() << "calls";
    } else {
        qDebug() << "Data already exists, skipping insert.";
    }

    return true;
}

static QStringList dxccNames()
{
    return {
        "AFGHANISTAN",
        "AGALEGA & SAINT BRANDON ISLANDS",
        "ALAND ISLANDS",
        "ALASKA",
        "ALBANIA",
        "ALGERIA",
        "AMERICAN SAMOA",
        "AMSTERDAM & SAINT PAUL ISLANDS",
        "ANDAMAN & NICOBAR ISLANDS",
        "ANDORRA",
        "ANGOLA",
        "ANGUILLA",
        "ANNOBON",
        "ANTARCTICA",
        "ANTIGUA & BARBUDA",
        "ARGENTINA",
        "ARMENIA",
        "ARUBA",
        "ASCENSION ISLAND",
        "ASIATIC RUSSIA",
        "AUSTRAL ISLANDS",
        "AUSTRALIA",
        "AUSTRIA",
        "AVES ISLAND",
        "AZERBAIJAN",
        "AZORES",
        "BAHAMAS",
        "BAHRAIN",
        "BAKER & HOWLAND ISLANDS",
        "BALEARIC ISLANDS",
        "BANABA ISLAND",
        "BANGLADESH",
        "BARBADOS",
        "BELARUS",
        "BELGIUM",
        "BELIZE",
        "BENIN",
        "BERMUDA",
        "BHUTAN",
        "BOLIVIA",
        "BONAIRE",
        "BOSNIA-HERZEGOVINA",
        "BOTSWANA",
        "BOUVET ISLAND",
        "BRAZIL",
        "BRITISH VIRGIN ISLANDS",
        "BRUNEI",
        "BULGARIA",
        "BURKINA FASO",
        "BURUNDI",
        "CAMBODIA",
        "CAMEROON",
        "CANADA",
        "CANARY ISLANDS",
        "CAPE VERDE",
        "CAYMAN ISLANDS",
        "CENTRAL AFRICAN REPUBLIC",
        "CENTRAL KIRIBATI",
        "CEUTA & MELILLA",
        "CHAD",
        "CHAGOS ISLANDS",
        "CHATHAM ISLAND",
        "CHESTERFIELD ISLANDS",
        "CHILE",
        "CHINA",
        "CHRISTMAS ISLAND",
        "CLIPPERTON ISLAND",
        "COCOS (KEELING) ISLANDS",
        "COCOS ISLAND",
        "COLOMBIA",
        "COMOROS",
        "CONWAY REEF",
        "CORSICA",
        "COSTA RICA",
        "COTE D'IVOIRE",
        "CRETE",
        "CROATIA",
        "CROZET ISLAND",
        "CUBA",
        "CURACAO",
        "CYPRUS",
        "CZECH REPUBLIC",
        "DEMOCRATIC REPUBLIC OF THE CONGO",
        "DENMARK",
        "DESECHEO ISLAND",
        "DJIBOUTI",
        "DODECANESE",
        "DOMINICA",
        "DOMINICAN REPUBLIC",
        "DPRK (NORTH KOREA)",
        "DUCIE ISLAND",
        "EAST MALAYSIA",
        "EASTER ISLAND",
        "EASTERN KIRIBATI",
        "ECUADOR",
        "EGYPT",
        "EL SALVADOR",
        "ENGLAND",
        "EQUATORIAL GUINEA",
        "ERITREA",
        "ESTONIA",
        "ETHIOPIA",
        "EUROPEAN RUSSIA",
        "FALKLAND ISLANDS",
        "FAROE ISLANDS",
        "FEDERAL REPUBLIC OF GERMANY",
        "FERNANDO DE NORONHA",
        "FIJI ISLANDS",
        "FINLAND",
        "FRANCE",
        "FRANZ JOSEF LAND",
        "FRENCH GUIANA",
        "FRENCH POLYNESIA",
        "GABON",
        "GALAPAGOS ISLANDS",
        "GEORGIA",
        "GHANA",
        "GIBRALTAR",
        "GLORIOSO ISLAND",
        "GREECE",
        "GREENLAND",
        "GRENADA",
        "GUADELOUPE",
        "GUAM",
        "GUANTANAMO BAY",
        "GUATEMALA",
        "GUERNSEY",
        "GUINEA",
        "GUINEA-BISSAU",
        "GUYANA",
        "HAITI",
        "HAWAII",
        "HEARD ISLAND",
        "HONDURAS",
        "HONG KONG",
        "HUNGARY",
        "ICELAND",
        "INDIA",
        "INDONESIA",
        "IRAN",
        "IRAQ",
        "IRELAND",
        "ISLE OF MAN",
        "ISRAEL",
        "ITALY",
        "ITU HQ",
        "JAMAICA",
        "JAN MAYEN",
        "JAPAN",
        "JERSEY",
        "JOHNSTON ISLAND",
        "JORDAN",
        "JUAN DE NOVA, EUROPA",
        "JUAN FERNANDEZ ISLAND",
        "KALININGRAD",
        "KAZAKHSTAN",
        "KENYA",
        "KERGUELEN ISLAND",
        "KERMADEC ISLAND",
        "KINGDOM OF ESWATINI",
        "KURE ISLAND",
        "KUWAIT",
        "KYRGYZSTAN",
        "LAKSHADWEEP ISLANDS",
        "LAOS",
        "LATVIA",
        "LEBANON",
        "LESOTHO",
        "LIBERIA",
        "LIBYA",
        "LIECHTENSTEIN",
        "LITHUANIA",
        "LORD HOWE ISLAND",
        "LUXEMBOURG",
        "MACAO",
        "MACQUARIE ISLAND",
        "MADAGASCAR",
        "MADEIRA ISLANDS",
        "MALAWI",
        "MALDIVES",
        "MALI",
        "MALPELO ISLAND",
        "MALTA",
        "MARIANA ISLANDS",
        "MARKET REEF",
        "MARQUESAS ISLANDS",
        "MARSHALL ISLANDS",
        "MARTINIQUE",
        "MAURITANIA",
        "MAURITIUS ISLAND",
        "MAYOTTE ISLAND",
        "MELLISH REEF",
        "MEXICO",
        "MICRONESIA",
        "MIDWAY ISLAND",
        "MINAMI TORISHIMA",
        "MOLDOVA",
        "MONACO",
        "MONGOLIA",
        "MONTENEGRO",
        "MONTSERRAT",
        "MOROCCO",
        "MOUNT ATHOS",
        "MOZAMBIQUE",
        "MYANMAR",
        "NAMIBIA",
        "NAURU",
        "NAVASSA ISLAND",
        "NEPAL",
        "NETHERLANDS",
        "NEW CALEDONIA",
        "NEW ZEALAND",
        "NEW ZEALAND SUBANTARCTIC ISLANDS",
        "NICARAGUA",
        "NIGER",
        "NIGERIA",
        "NIUE",
        "NORFOLK ISLAND",
        "NORTH COOK ISLANDS",
        "NORTH MACEDONIA",
        "NORTHERN IRELAND",
        "NORWAY",
        "OGASAWARA",
        "OMAN",
        "PAKISTAN",
        "PALAU",
        "PALESTINE",
        "PALMYRA & JARVIS ISLANDS",
        "PANAMA",
        "PAPUA NEW GUINEA",
        "PARAGUAY",
        "PERU",
        "PETER 1 ISLAND",
        "PHILIPPINES",
        "PITCAIRN ISLAND",
        "POLAND",
        "PORTUGAL",
        "PRATAS ISLAND",
        "PRINCE EDWARD & MARION ISLANDS",
        "PUERTO RICO",
        "QATAR",
        "REPUBLIC OF KOREA",
        "REPUBLIC OF KOSOVO",
        "REPUBLIC OF SOUTH AFRICA",
        "REPUBLIC OF SOUTH SUDAN",
        "REPUBLIC OF THE CONGO",
        "REUNION ISLAND",
        "REVILLAGIGEDO",
        "RODRIGUEZ ISLAND",
        "ROMANIA",
        "ROTUMA",
        "RWANDA",
        "SABA & SAINT EUSTATIUS",
        "SABLE ISLAND",
        "SAINT BARTHELEMY",
        "SAINT HELENA",
        "SAINT KITTS & NEVIS",
        "SAINT LUCIA",
        "SAINT MARTIN",
        "SAINT PAUL ISLAND",
        "SAINT PETER AND PAUL ROCKS",
        "SAINT PIERRE & MIQUELON",
        "SAINT VINCENT",
        "SAMOA",
        "SAN ANDRES ISLAND",
        "SAN FELIX ISLAND",
        "SAN MARINO",
        "SAO TOME & PRINCIPE",
        "SARDINIA",
        "SAUDI ARABIA",
        "SCARBOROUGH REEF",
        "SCOTLAND",
        "SENEGAL",
        "SERBIA",
        "SEYCHELLES ISLANDS",
        "SIERRA LEONE",
        "SINGAPORE",
        "SINT MAARTEN",
        "SLOVAK REPUBLIC",
        "SLOVENIA",
        "SOLOMON ISLANDS",
        "SOMALIA",
        "SOUTH COOK ISLANDS",
        "SOUTH GEORGIA ISLAND",
        "SOUTH ORKNEY ISLANDS",
        "SOUTH SANDWICH ISLANDS",
        "SOUTH SHETLAND ISLANDS",
        "SOVEREIGN MILITARY ORDER OF MALTA",
        "SPAIN",
        "SPRATLY ISLANDS",
        "SRI LANKA",
        "SUDAN",
        "SURINAME",
        "SVALBARD",
        "SWAINS ISLAND",
        "SWEDEN",
        "SWITZERLAND",
        "SYRIA",
        "TAIWAN",
        "TAJIKISTAN",
        "TANZANIA",
        "TEMOTU PROVINCE",
        "THAILAND",
        "THE GAMBIA",
        "TIMOR - LESTE",
        "TOGO",
        "TOKELAU ISLANDS",
        "TONGA",
        "TRINDADE & MARTIM VAZ ISLANDS",
        "TRINIDAD & TOBAGO",
        "TRISTAN DA CUNHA & GOUGH ISLANDS",
        "TROMELIN ISLAND",
        "TUNISIA",
        "TURKEY",
        "TURKMENISTAN",
        "TURKS & CAICOS ISLANDS",
        "TUVALU",
        "U K BASES ON CYPRUS",
        "UGANDA",
        "UKRAINE",
        "UNITED ARAB EMIRATES",
        "UNITED NATIONS HQ",
        "UNITED STATES OF AMERICA",
        "URUGUAY",
        "US VIRGIN ISLANDS",
        "UZBEKISTAN",
        "VANUATU",
        "VATICAN CITY",
        "VENEZUELA",
        "VIET NAM",
        "WAKE ISLAND",
        "WALES",
        "WALLIS & FUTUNA ISLANDS",
        "WEST MALAYSIA",
        "WESTERN KIRIBATI",
        "WESTERN SAHARA",
        "WILLIS ISLAND",
        "YEMEN",
        "ZAMBIA",
        "ZIMBABWE"
    };
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("HamVibe");
    QCoreApplication::setApplicationName("HamVibe");

    const QStringList calls = {
        "3B8WWA","3Z6I","4M5A","4M5DX","4U1A","8A1A","9M2WWA","9M8WWA","A43WWA","AT2WWA","AT3WWA","AT4WWA","AT6WWA","AT7WWA","BA3RA","BA7CK","BG0DXC","BH9CA","BI4SSB","BY1RX",
        "BY2WL","BY5HB","BY6SX","BY8MA","CQ7WWA","CR2WWA","CR5WWA","CR6WWA","D4W","DA0WWA","DL0WWA","DU0WWA","E2WWA","E7W","EG1WWA","EG2WWA","EG3WWA","EG4WWA","EG5WWA","EG6WWA",
        "EG7WWA","EG9WWA","EM0WWA","GB0WWA","GB1WWA","GB2WWA","GB4WWA","GB5WWA","GB6WWA","GB8WWA","GB9WWA","HB9WWA","HI3WWA","HI6WWA","HI7WWA","HI8WWA","HZ1WWA","II0WWA","II1WWA",
        "II2WWA","II3WWA","II4WWA","II5WWA","II6WWA","II7WWA","II8WWA","II9WWA","IR0WWA","IR1WWA","LR1WWA","N0W","N1W","N4W","N6W","N8W","N9W","OL6WWA","OP0WWA","PA26WWA","PC26WWA",
        "PD26WWA","PE26WWA","PF26WWA","RW1F","S53WWA","SB9WWA","SC9WWA","SD9WWA","SN0WWA","SN1WWA","SN2WWA","SN3WWA","SN4WWA","SN6WWA","SO3WWA","SX0W","TK4TH","TM18WWA","TM1WWA",
        "TM29WWA","TM7WWA","TM9WWA","UP7WWA","VB2WWA","VC1WWA","VE9WWA","W4I","YI1RN","YL73R","YO0WWA","YU45MJA","Z30WWA"
    };

    const QString dbPath = QDir(QCoreApplication::applicationDirPath())
                               .filePath("HamVibe.db");
    if (!setupDatabase(QString(), dbPath, "modes", calls))
        return -1;

    {
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        const QStringList bands = {"2","6","10","12","15","17","20","30","40","80","160"};
        const QStringList modes = {"cw","ph","dg"};
        QStringList cols;
        cols << "dxcc TEXT DEFAULT ''";
        cols << "prefix TEXT DEFAULT ''";
        for (const QString &band : bands) {
            for (const QString &mode : modes) {
                cols << QString("\"%1%2\" TEXT DEFAULT ''").arg(band, mode);
            }
        }
        const QString createSql = QString("CREATE TABLE IF NOT EXISTS dxcc (%1)").arg(cols.join(", "));
        if (!query.exec(createSql)) {
            qWarning() << "Failed to create DXCC table:" << query.lastError();
            return -1;
        }

        QSet<QString> existing;
        QSqlQuery info(db);
        if (info.exec("PRAGMA table_info(dxcc)")) {
            while (info.next()) {
                existing.insert(info.value(1).toString());
            }
        }
        if (!existing.contains("dxcc")) {
            if (!query.exec("ALTER TABLE dxcc ADD COLUMN dxcc TEXT DEFAULT ''")) {
                qWarning() << "Failed to add dxcc column:" << query.lastError();
                return -1;
            }
        }
        if (!existing.contains("prefix")) {
            if (!query.exec("ALTER TABLE dxcc ADD COLUMN prefix TEXT DEFAULT ''")) {
                qWarning() << "Failed to add prefix column:" << query.lastError();
                return -1;
            }
        }

        if (!query.exec("SELECT COUNT(*) FROM dxcc")) {
            qWarning() << "Failed to count DXCC rows:" << query.lastError();
            return -1;
        }
        if (query.next() && query.value(0).toInt() == 0) {
            QSqlQuery insert(db);
            if (!insert.prepare("INSERT INTO dxcc (dxcc, prefix) VALUES (?, '')")) {
                qWarning() << "Failed to prepare DXCC insert:" << insert.lastError();
                return -1;
            }
            const QStringList names = dxccNames();
            for (const QString &name : names) {
                insert.addBindValue(name);
                if (!insert.exec()) {
                    qWarning() << "DXCC insert failed:" << insert.lastError();
                    return -1;
                }
                insert.finish();
            }
        }
    }

    MainWindow window;
    window.show();
    return app.exec();
}
