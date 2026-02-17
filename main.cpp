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
    "3B8WWA","3Z6I","4M5A","4M5DX","4U1A","8A1A","9M2WWA","9M8WWA","A43WWA","AT2WWA","AT3WWA","AT4WWA","AT6WWA","AT7WWA","BA3RA","BA7CK","BG0DXC","BH9CA","BI4SSB","BY1RX",
    "BY2WL","BY5HB","BY6SX","BY8MA","CQ7WWA","CR2WWA","CR5WWA","CR6WWA","D4W","DA0WWA","DL0WWA","DU0WWA","E2WWA","E7W","EG1WWA","EG2WWA","EG3WWA","EG4WWA","EG5WWA","EG6WWA",
    "EG7WWA","EG9WWA","EM0WWA","GB0WWA","GB1WWA","GB2WWA","GB4WWA","GB5WWA","GB6WWA","GB8WWA","GB9WWA","HB9WWA","HI3WWA","HI6WWA","HI7WWA","HI8WWA","HZ1WWA","II0WWA","II1WWA",
    "II2WWA","II3WWA","II4WWA","II5WWA","II6WWA","II7WWA","II8WWA","II9WWA","IR0WWA","IR1WWA","LR1WWA","N0W","N1W","N4W","N6W","N8W","N9W","OL6WWA","OP0WWA","PA26WWA","PC26WWA",
    "PD26WWA","PE26WWA","PF26WWA","RW1F","S53WWA","SB9WWA","SC9WWA","SD9WWA","SN0WWA","SN1WWA","SN2WWA","SN3WWA","SN4WWA","SN6WWA","SO3WWA","SX0W","TK4TH","TM18WWA","TM1WWA",
    "TM29WWA","TM7WWA","TM9WWA","UP7WWA","VB2WWA","VC1WWA","VE9WWA","W4I","YI1RN","YL73R","YO0WWA","YU45MJA","Z30WWA"
};

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
        const QStringList names = dxccNames();
        const QMap<QString, QString> prefixMap = dxccPrefixMap();
        for (const QString &name : names) {
            const QString prefix = prefixMap.value(name.toUpper(), "");
            insert.addBindValue(prefix);
            insert.addBindValue(name);
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

    return true;
}


/*

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


static bool setupDxccTable(QSqlDatabase &db)
{
    QSqlQuery query(db);
    const QStringList bands = {"160","80","40","30","20","17","15","12","10","6","2"};

    QSet<QString> existing;
    QSqlQuery info(db);
    if (info.exec("PRAGMA table_info(dxcc)")) {
        while (info.next()) {
            existing.insert(info.value(1).toString());
        }
    }

    const bool hasModeColumns = existing.contains("2cw") || existing.contains("2ph") || existing.contains("2dg");
    if (hasModeColumns) {
        if (!query.exec("BEGIN TRANSACTION")) {
            qWarning() << "Failed to begin DXCC migration:" << query.lastError();
            return false;
        }
        QStringList cols;
        cols << "prefix TEXT DEFAULT ''" << "entity TEXT DEFAULT ''"
             << "Mix TEXT DEFAULT ''" << "Ph TEXT DEFAULT ''"
             << "CW TEXT DEFAULT ''" << "RT TEXT DEFAULT ''"
             << "SAT TEXT DEFAULT ''";
        for (const QString &band : bands) {
            cols << QString("\"%1\" TEXT DEFAULT ''").arg(band);
        }
        const QString createNew = QString("CREATE TABLE IF NOT EXISTS dxcc_new (%1)").arg(cols.join(", "));
        if (!query.exec(createNew)) {
            qWarning() << "Failed to create dxcc_new:" << query.lastError();
            return false;
        }

        QStringList selects;
        selects << "prefix"
                << (existing.contains("entity") ? "entity" : "dxcc")
                << "'' AS Mix" << "'' AS Ph" << "'' AS CW" << "'' AS RT" << "'' AS SAT";
        for (const QString &band : bands) {
            const QString cw = QString("\"%1cw\"").arg(band);
            const QString ph = QString("\"%1ph\"").arg(band);
            const QString dg = QString("\"%1dg\"").arg(band);
            selects << QString("(CASE WHEN COALESCE(%1,'')!='' OR COALESCE(%2,'')!='' OR COALESCE(%3,'')!='' THEN 'V' ELSE '' END) AS \"%4\"")
                          .arg(cw, ph, dg, band);
        }
        const QString insertSql = QString("INSERT INTO dxcc_new SELECT %1 FROM dxcc").arg(selects.join(", "));
        if (!query.exec(insertSql)) {
            qWarning() << "Failed to migrate DXCC data:" << query.lastError();
            return false;
        }
        if (!query.exec("DROP TABLE dxcc")) {
            qWarning() << "Failed to drop old dxcc:" << query.lastError();
            return false;
        }
        if (!query.exec("ALTER TABLE dxcc_new RENAME TO dxcc")) {
            qWarning() << "Failed to rename dxcc_new:" << query.lastError();
            return false;
        }
        if (!query.exec("COMMIT")) {
            qWarning() << "Failed to commit DXCC migration:" << query.lastError();
            return false;
        }
    }

    QStringList cols;
    cols << "prefix TEXT DEFAULT ''" << "entity TEXT DEFAULT ''"
         << "Mix TEXT DEFAULT ''" << "Ph TEXT DEFAULT ''"
         << "CW TEXT DEFAULT ''" << "RT TEXT DEFAULT ''"
         << "SAT TEXT DEFAULT ''";
    for (const QString &band : bands) {
        cols << QString("\"%1\" TEXT DEFAULT ''").arg(band);
    }
    const QString createSql = QString("CREATE TABLE IF NOT EXISTS dxcc (%1)").arg(cols.join(", "));
    if (!query.exec(createSql)) {
        qWarning() << "Failed to create DXCC table:" << query.lastError();
        return false;
    }

    QSet<QString> existingAfter;
    QSqlQuery infoAfter(db);
    if (infoAfter.exec("PRAGMA table_info(dxcc)")) {
        while (infoAfter.next()) {
            existingAfter.insert(infoAfter.value(1).toString());
        }
    }
    if (!existingAfter.contains("prefix")) {
        if (!query.exec("ALTER TABLE dxcc ADD COLUMN prefix TEXT DEFAULT ''")) {
            qWarning() << "Failed to add prefix column:" << query.lastError();
            return false;
        }
    }
    if (!existingAfter.contains("entity")) {
        if (!query.exec("ALTER TABLE dxcc ADD COLUMN entity TEXT DEFAULT ''")) {
            qWarning() << "Failed to add entity column:" << query.lastError();
            return false;
        }
    }
    if (!existingAfter.contains("Mix")) {
        if (!query.exec("ALTER TABLE dxcc ADD COLUMN Mix TEXT DEFAULT ''")) {
            qWarning() << "Failed to add Mix column:" << query.lastError();
            return false;
        }
    }
    if (!existingAfter.contains("Ph")) {
        if (!query.exec("ALTER TABLE dxcc ADD COLUMN Ph TEXT DEFAULT ''")) {
            qWarning() << "Failed to add Ph column:" << query.lastError();
            return false;
        }
    }
    if (!existingAfter.contains("CW")) {
        if (!query.exec("ALTER TABLE dxcc ADD COLUMN CW TEXT DEFAULT ''")) {
            qWarning() << "Failed to add CW column:" << query.lastError();
            return false;
        }
    }
    if (!existingAfter.contains("RT")) {
        if (!query.exec("ALTER TABLE dxcc ADD COLUMN RT TEXT DEFAULT ''")) {
            qWarning() << "Failed to add RT column:" << query.lastError();
            return false;
        }
    }
    if (!existingAfter.contains("SAT")) {
        if (!query.exec("ALTER TABLE dxcc ADD COLUMN SAT TEXT DEFAULT ''")) {
            qWarning() << "Failed to add SAT column:" << query.lastError();
            return false;
        }
    }
    for (const QString &band : bands) {
        if (!existingAfter.contains(band)) {
            if (!query.exec(QString("ALTER TABLE dxcc ADD COLUMN \"%1\" TEXT DEFAULT ''").arg(band))) {
                qWarning() << "Failed to add band column:" << band << query.lastError();
                return false;
            }
        }
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
        const QStringList names = dxccNames();
        const QMap<QString, QString> prefixMap = dxccPrefixMap();
        for (const QString &name : names) {
            const QString prefix = prefixMap.value(name.toUpper(), "");
            insert.addBindValue(prefix);
            insert.addBindValue(name);
            if (!insert.exec()) {
                qWarning() << "DXCC insert failed:" << insert.lastError();
                return false;
            }
            insert.finish();
        }
    }

    return true;
}

*/

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
