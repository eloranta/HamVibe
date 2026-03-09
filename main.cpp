#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>
#include <QRegularExpression>

const QStringList calls = {
"AL1G","ZA1AK","ZA1EM","LU1ANG","LU1IYL","LU1WCU","LU1XJJ","LU2HYL","LU4DJB","LU4JVE","LU5OIJ","LU7DYL","LU8DMA","LU8GCJ","LW6ERY","RY9KYL","TA3TGC","VK4DI","VK5MAZ","OE3JLY","OE3LZA","OE3YSC","OE5BAE","OE7MPN","OE8VLK","OE8YXK","OE8YYY","CS8ACM","CU2YK","ON3ANN","ON3EBR","ON3NYL","ON4CHT","ON4EK","ON5RS","ON7SAS","V31YL","E71AJL","E71DK","E74BJJ","E79HI","VA3PZA","VA3WHU","VE9GLF","EA8DPX","HK3YL","OK1LYL","OK2APY","OK3FLY","2E0OHT","G0BIR","M6EDA","M7KNO","M7KVC","M9YLR","ES3YJ","ES3YW","ES8KRI","R1BIG","R2CC","R6DVC","R6DVG","RU3XY","UB1AUH","DL3HD","DA2KT","DA2NM","DA6BC","DC1YL","DC2CT","DD2VX","DD7MR","DF2REY","DF4RBM","DF4UM","DF6QP","DF8UY","DG7OY","DH8LAR","DJ0DE","DJ5YL","DJ9YL","DK1FE","DK1LJ","DK2OL","DK2YLL","DK5YL","DK8HUU","DL1FKB","DL1STN","DL1TM","DL2GRC","DL2LBK","DL3CR","DL3HD","DL4CR","DL4DXF","DL4VER","DL5FM","DL5PIA","DL5YL","DL6SAK","DL6TNT","DL7PIA","DL8DYL","DL8JC","DM3UX","DM4EAX","DM4STG","DN9CAT","DN9CY","DO3HTV","DO9JBP","OH5KIZ","OH5YL","F4GDI","F4JWK","F4LCM","F4LQS","F4LSM","F4MNO","F4MXJ","F5CDE","SV2TCC","SV2TSU","SV4TTE","SV8OVH","SY1EDC","HA1AS","HA4NI","HA4ZS","HA5BA","HA5BN","HA5VP","HA5YG","HA5YN","VU2RBI","VU2MGS","VU2RBI","VU3GGU","VU3GGU","VU3IVO","VU3ZOE","YB0BIP","YB1CAS","YB1CWO","YB1JYL","YB1KQV","YB1TIA","YB3ETY","YB5SLA","YC1EBM","YC1RJN","YD4LUM","YG1CAZ","EI5IXB","IN3FHE","IU1SCQ","IU1TKT","IU2SMJ","IU2VUD","IU2VZW","IU4QSV","IU5AWQ","IU5JZQ","IU5LVM","IZ5BRO","IZ7AUK","7K4TKB","JI1JRE","JP3AYQ","XE1ADY","XE1DEL","XE1FAV","XE1LIN","XE1YL","XE2IHA","PA2KM","PD0HI","PD0YL","PD3GVQ","PD4LYN","GI0AZA","LB5QK","HP3GNG","4F1CAK","4G1QLK","DU1YL","HF9SL","SP2FF","SP6WE","SP8SAN","SP9AJP","SP9ARX","SQ3TGY","SQ5CAT","SQ5EC","SQ6ALS","SQ6PLH","SQ8BWA","SQ8KJC","WP4MTS","WP4NWK","WP4SMO","YO3BEL","IS0JRL","HZ0YL","GM4YMM","GM5CAR","M0YRN","MM7TKP","YT3NR","IT9KAX","IT9KKB","OM1ADA","S54TIM","S55BA","EA1FRQ","EA1IQR","EB5AN","HB9EPE","HB9INY","E20NKB","E25KAE","HS0ZRR","UR1CED","UR3PHG","AC9XK","AG6V","K4SAF","K6YYL","K8ZI","K9JJR","K9UET","KA8MNV","KC1OHT","KC3VQP","KC3ZZO","KC5BOO","KD2GUT","KD5ZZU","KD9WAI","KD9YAY","KE0TL","KE0WPA","KE9APN","KE9CKA","KE9DDT","KF8CYL","KI5SSR","KI5WEP","KJ0WHOO","KJ4ULZ","KJ5LXP","KJ6GHN","KM4WSK","KN4DDC","KQ3Q","KQ4JNW","KQ4ZFX","KR4CXD","KR4FTE","KS4YT","KY2MMM","N0QQ","N2RJ","N3PBD","N5ALG","N6ECW","N8PTL","NX8Z","NZ5W","W1GRL","W4AA","W4CMG","W4KRN","W7TEE","WA4SHA","WB0ICT","WD4AGF","WI7NGS"};

bool setupWwaTable(QSqlDatabase db)
{
    QSqlQuery query(db);

    const QString tableName = "modes";

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

static QMap<QString, QString> dxccPrefixMap()
{
    static const QString raw = R"RAW(
    ?=SPRATLY ISLANDS
    1A0KM=SOVEREIGN MILITARY ORDER OF MALTA
    3A=MONACO
    3B7=AGALEGA & SAINT BRANDON ISLANDS
    3B8=MAURITIUS ISLAND
    3B9=RODRIGUEZ ISLAND
    3C=EQUATORIAL GUINEA
    3C0=ANNOBON
    3D2=CONWAY REEF
    3D2=FIJI ISLANDS
    3D2=ROTUMA
    3DA=KINGDOM OF ESWATINI
    3V=TUNISIA
    3W, XV=VIET NAM
    3XA=GUINEA
    3Y=BOUVET ISLAND
    3Y=PETER 1 ISLAND
    4J=AZERBAIJAN
    4L=GEORGIA
    4O=MONTENEGRO
    4S=SRI LANKA
    4U1ITU=ITU HQ
    4U1UN=UNITED NATIONS HQ
    4W=TIMOR - LESTE
    4X=ISRAEL
    5A=LIBYA
    5B=CYPRUS
    5H=TANZANIA
    5N=NIGERIA
    5R=MADAGASCAR
    5T=MAURITANIA
    5U=NIGER
    5V7=TOGO
    5W=SAMOA
    5X=UGANDA
    5Z=KENYA
    6W=SENEGAL
    6Y=JAMAICA
    7O=YEMEN
    7P=LESOTHO
    7Q=MALAWI
    7X=ALGERIA
    8P=BARBADOS
    8Q=MALDIVES
    8R=GUYANA
    9A=CROATIA
    9G=GHANA
    9H=MALTA
    9J=ZAMBIA
    9K=KUWAIT
    9L=SIERRA LEONE
    9M2=WEST MALAYSIA
    9M6=EAST MALAYSIA
    9N=NEPAL
    9Q=DEMOCRATIC REPUBLIC OF THE CONGO
    9U=BURUNDI
    9V=SINGAPORE
    9X=RWANDA
    9Y=TRINIDAD & TOBAGO
    A2=BOTSWANA
    A3=TONGA
    A4=OMAN
    A5=BHUTAN
    A6=UNITED ARAB EMIRATES
    A7=QATAR
    A9=BAHRAIN
    AP=PAKISTAN
    BS7H=SCARBOROUGH REEF
    BV=TAIWAN
    BV9P=PRATAS ISLAND
    BY=CHINA
    C21=NAURU
    C31=ANDORRA
    C5=THE GAMBIA
    C6A=BAHAMAS
    C9=MOZAMBIQUE
    CE=CHILE
    CE0X=SAN FELIX ISLAND
    CE0Y=EASTER ISLAND
    CE0Z=JUAN FERNANDEZ ISLAND
    CE9, KC4=ANTARCTICA
    CN=MOROCCO
    CO=CUBA
    CP=BOLIVIA
    CT=PORTUGAL
    CT3=MADEIRA ISLANDS
    CU=AZORES
    CX=URUGUAY
    CY0=SABLE ISLAND
    CY9=SAINT PAUL ISLAND
    D2=ANGOLA
    D4=CAPE VERDE
    D6=COMOROS
    DL=FEDERAL REPUBLIC OF GERMANY
    DU=PHILIPPINES
    E3=ERITREA
    E4=PALESTINE
    E5=NORTH COOK ISLANDS
    E5=SOUTH COOK ISLANDS
    E6=NIUE
    E7=BOSNIA-HERZEGOVINA
    EA=SPAIN
    EA6=BALEARIC ISLANDS
    EA8=CANARY ISLANDS
    EA9=CEUTA & MELILLA
    EI=IRELAND
    EK=ARMENIA
    EL=LIBERIA
    EP=IRAN
    ER=MOLDOVA
    ES=ESTONIA
    ET=ETHIOPIA
    EU=BELARUS
    EX=KYRGYZSTAN
    EY=TAJIKISTAN
    EZ=TURKMENISTAN
    F=FRANCE
    FG=GUADELOUPE
    FH=MAYOTTE ISLAND
    FJ=SAINT BARTHELEMY
    FK=NEW CALEDONIA
    FK/C=CHESTERFIELD ISLANDS
    FM=MARTINIQUE
    FO=AUSTRAL ISLANDS
    FO=FRENCH POLYNESIA
    FO=MARQUESAS ISLANDS
    FO0=CLIPPERTON ISLAND
    FP=SAINT PIERRE & MIQUELON
    FR=REUNION ISLAND
    FR/G=GLORIOSO ISLAND
    FR/J=JUAN DE NOVA, EUROPA
    FR/T=TROMELIN ISLAND
    FS=SAINT MARTIN
    FT5W=CROZET ISLAND
    FT5X=KERGUELEN ISLAND
    FT5Z=AMSTERDAM & SAINT PAUL ISLANDS
    FW=WALLIS & FUTUNA ISLANDS
    FY=FRENCH GUIANA
    G,M=ENGLAND
    GD=ISLE OF MAN
    GI,MI=NORTHERN IRELAND
    GJ=JERSEY
    GM,MM=SCOTLAND
    GU=GUERNSEY
    GW,MW=WALES
    H4=SOLOMON ISLANDS
    H40=TEMOTU PROVINCE
    HA=HUNGARY
    HB=SWITZERLAND
    HB0=LIECHTENSTEIN
    HC=ECUADOR
    HC8=GALAPAGOS ISLANDS
    HH=HAITI
    HI=DOMINICAN REPUBLIC
    HK=COLOMBIA
    HK0=MALPELO ISLAND
    HK0=SAN ANDRES ISLAND
    HL,DS=REPUBLIC OF KOREA
    HP=PANAMA
    HR=HONDURAS
    HS=THAILAND
    HV=VATICAN CITY
    HZ=SAUDI ARABIA
    I=ITALY
    IS0,IMO=SARDINIA
    J2=DJIBOUTI
    J3=GRENADA
    J5=GUINEA-BISSAU
    J6=SAINT LUCIA
    J7=DOMINICA
    J8=SAINT VINCENT
    JA=JAPAN
    JD1=MINAMI TORISHIMA
    JD1=OGASAWARA
    JT=MONGOLIA
    JW=SVALBARD
    JX=JAN MAYEN
    JY=JORDAN
    K, W=UNITED STATES OF AMERICA
    KG4=GUANTANAMO BAY
    KH0=MARIANA ISLANDS
    KH1=BAKER & HOWLAND ISLANDS
    KH2=GUAM
    KH3=JOHNSTON ISLAND
    KH4=MIDWAY ISLAND
    KH5=PALMYRA & JARVIS ISLANDS
    KH6,KH7=HAWAII
    KH7K=KURE ISLAND
    KH8=AMERICAN SAMOA
    KH8=SWAINS ISLAND
    KH9=WAKE ISLAND
    KL7=ALASKA
    KP1=NAVASSA ISLAND
    KP2=US VIRGIN ISLANDS
    KP3,KP4=PUERTO RICO
    KP5=DESECHEO ISLAND
    LA=NORWAY
    LU=ARGENTINA
    LX=LUXEMBOURG
    LY=LITHUANIA
    LZ=BULGARIA
    OA=PERU
    OD=LEBANON
    OE=AUSTRIA
    OH=FINLAND
    OH0=ALAND ISLANDS
    OH0M=MARKET REEF
    OK,OL=CZECH REPUBLIC
    OM=SLOVAK REPUBLIC
    ON=BELGIUM
    OX=GREENLAND
    OY=FAROE ISLANDS
    OZ=DENMARK
    P2=PAPUA NEW GUINEA
    P4=ARUBA
    P5,HM=DPRK (NORTH KOREA)
    PA=NETHERLANDS
    PJ2=CURACAO
    PJ4=BONAIRE
    PJ5,PJ6=SABA & SAINT EUSTATIUS
    PJ7=SINT MAARTEN
    PY=BRAZIL
    PY0F=FERNANDO DE NORONHA
    PY0S=SAINT PETER AND PAUL ROCKS
    PY0T=TRINDADE & MARTIM VAZ ISLANDS
    PZ=SURINAME
    R1F=FRANZ JOSEF LAND
    S0=WESTERN SAHARA
    S2=BANGLADESH
    S5=SLOVENIA
    S7=SEYCHELLES ISLANDS
    S9=SAO TOME & PRINCIPE
    SM=SWEDEN
    SP=POLAND
    ST=SUDAN
    SU=EGYPT
    SV=GREECE
    SV/A=MOUNT ATHOS
    SV5=DODECANESE
    SV9=CRETE
    T2=TUVALU
    T30=WESTERN KIRIBATI
    T31=CENTRAL KIRIBATI
    T32=EASTERN KIRIBATI
    T33=BANABA ISLAND
    T5=SOMALIA
    T7=SAN MARINO
    T8=PALAU
    TA=TURKEY
    TF=ICELAND
    TG=GUATEMALA
    TI=COSTA RICA
    TI9=COCOS ISLAND
    TJ=CAMEROON
    TK=CORSICA
    TL=CENTRAL AFRICAN REPUBLIC
    TN=REPUBLIC OF THE CONGO
    TR=GABON
    TT=CHAD
    TU=COTE D'IVOIRE
    TY=BENIN
    TZ=MALI
    UA=EUROPEAN RUSSIA
    UA2=KALININGRAD
    UA9,UA0=ASIATIC RUSSIA
    UJ=UZBEKISTAN
    UN=KAZAKHSTAN
    UT=UKRAINE
    V2=ANTIGUA & BARBUDA
    V3=BELIZE
    V4=SAINT KITTS & NEVIS
    V5=NAMIBIA
    V6=MICRONESIA
    V7=MARSHALL ISLANDS
    V8=BRUNEI
    VE=CANADA
    VK=AUSTRALIA
    VK0=HEARD ISLAND
    VK0=MACQUARIE ISLAND
    VK9C=COCOS (KEELING) ISLANDS
    VK9L=LORD HOWE ISLAND
    VK9M=MELLISH REEF
    VK9N=NORFOLK ISLAND
    VK9W=WILLIS ISLAND
    VK9X=CHRISTMAS ISLAND
    VP2E=ANGUILLA
    VP2M=MONTSERRAT
    VP2V=BRITISH VIRGIN ISLANDS
    VP5=TURKS & CAICOS ISLANDS
    VP6=DUCIE ISLAND
    VP6=PITCAIRN ISLAND
    VP8=FALKLAND ISLANDS
    VP8=SOUTH GEORGIA ISLAND
    VP8=SOUTH ORKNEY ISLANDS
    VP8=SOUTH SANDWICH ISLANDS
    VP8=SOUTH SHETLAND ISLANDS
    VP9=BERMUDA
    VQ9=CHAGOS ISLANDS
    VR=HONG KONG
    VU=INDIA
    VU4=ANDAMAN & NICOBAR ISLANDS
    VU7=LAKSHADWEEP ISLANDS
    XE=MEXICO
    XF4=REVILLAGIGEDO
    XT=BURKINA FASO
    XU=CAMBODIA
    XW=LAOS
    XX9=MACAO
    XZ=MYANMAR
    YA=AFGHANISTAN
    YB=INDONESIA
    YI=IRAQ
    YJ=VANUATU
    YK=SYRIA
    YL=LATVIA
    YN=NICARAGUA
    YO=ROMANIA
    YS=EL SALVADOR
    YU=SERBIA
    YV=VENEZUELA
    YV0=AVES ISLAND
    Z2=ZIMBABWE
    Z3=NORTH MACEDONIA
    Z6=REPUBLIC OF KOSOVO
    Z8=REPUBLIC OF SOUTH SUDAN
    ZA=ALBANIA
    ZB2=GIBRALTAR
    ZC4=U K BASES ON CYPRUS
    ZD7=SAINT HELENA
    ZD8=ASCENSION ISLAND
    ZD9=TRISTAN DA CUNHA & GOUGH ISLANDS
    ZF=CAYMAN ISLANDS
    ZK3=TOKELAU ISLANDS
    ZL=NEW ZEALAND
    ZL7=CHATHAM ISLAND
    ZL8=KERMADEC ISLAND
    ZL9=NEW ZEALAND SUBANTARCTIC ISLANDS
    ZP=PARAGUAY
    ZS=REPUBLIC OF SOUTH AFRICA
    ZS8=PRINCE EDWARD & MARION ISLANDS
    )RAW";

    QMap<QString, QString> map;
    const QStringList lines = raw.split(QRegularExpression(R"(\r?\n)"), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const int sep = line.indexOf('=');
        if (sep <= 0) {
            continue;
        }
        QString prefix = line.left(sep).trimmed();
        QString entity = line.mid(sep + 1).trimmed();
        if (entity.isEmpty()) {
            continue;
        }
        map.insert(entity.toUpper(), prefix);
    }
    return map;
}

bool setupDxccTable(QSqlDatabase db)
{
    QSqlQuery query(db);

    const QString tableName = "dxcc";

    const QString createSql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            Prefix TEXT,
            Entity TEXT,
            Mix TEXT,
            Ph TEXT,
            CW TEXT,
            RT TEXT,
            SAT TEXT,
            "160" INTEGER,
            "80" INTEGER,
            "40" INTEGER,
            "30" INTEGER,
            "20" INTEGER,
            "17" INTEGER,
            "15" INTEGER,
            "12" INTEGER,
            "10" INTEGER,
            "6"  INTEGER,
            "2"  INTEGER
        )
    )").arg(tableName);

    if (!query.exec(createSql)) {
        qWarning() << "Failed to create table:" << query.lastError();
        return false;
    }

    if (!query.exec("SELECT COUNT(*) FROM dxcc")) {
        qWarning() << "Failed to count DXCC rows:" << query.lastError();
        return false;
    }
    if (query.next() && query.value(0).toInt() == 0) {
        QSqlQuery insert(db);
        if (!insert.prepare("INSERT INTO dxcc (prefix, entity, Mix, Ph, CW, RT, SAT) VALUES (?, ?, '', '', '', '', '')")) {
            qWarning() << "Failed to prepare DXCC insert:" << insert.lastError();
            return false;
        }
        const QMap<QString, QString> prefixMap = dxccPrefixMap();
        QStringList entities = prefixMap.keys();
        entities.sort();
        for (const QString &entity : entities) {
            const QString prefix = prefixMap.value(entity.toUpper(), "");
            insert.addBindValue(prefix);
            insert.addBindValue(entity);
            if (!insert.exec()) {
                qWarning() << "DXCC insert failed:" << insert.lastError();
                return false;
            }
            insert.finish();
        }
    }

    return true;
}

bool setupDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    const QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("HamVibe.db");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qFatal("Cannot open database!");
        return false;
    }

    if (!setupWwaTable(db)) {
        qFatal("Cannot set WWA table!");
        return false;
    }

    if (!setupDxccTable(db)) {
        qFatal("Cannot set DXCC table!");
        return false;
    }

    {
        QSqlQuery query(db);
        const QString createSpots = R"(
            CREATE TABLE IF NOT EXISTS spots (
                time TEXT,
                call TEXT,
                freq TEXT,
                mode TEXT,
                country TEXT,
                spotter TEXT,
                message TEXT
            )
        )";
        if (!query.exec(createSpots)) {
            qWarning() << "Failed to create spots table:" << query.lastError();
            return false;
        }
        QSet<QString> spotCols;
        QSqlQuery info(db);
        if (info.exec("PRAGMA table_info(spots)")) {
            while (info.next()) {
                spotCols.insert(info.value(1).toString().toLower());
            }
        }
        if (!spotCols.contains("message")) {
            if (!query.exec("ALTER TABLE spots ADD COLUMN message TEXT")) {
                qWarning() << "Failed to add message column to spots:" << query.lastError();
                return false;
            }
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("HamVibe");
    QCoreApplication::setApplicationName("HamVibe");

    if (!setupDatabase())
        return -1;

    MainWindow window;
    window.show();
    return app.exec();
}
