import csv
import sqlite3
from pathlib import Path

DB_PATH = Path(__file__).with_name("hamvibe.db")
LOG_CSV = Path(__file__).with_name("log.csv")

COLUMNS = [
    "Prefix",
    "Entity",
    "Mix",
    "Ph",
    "CW",
    "RT",
    "SAT",
    "160",
    "80",
    "40",
    "30",
    "20",
    "17",
    "15",
    "12",
    "10",
    "6",
    "2",
]

# Column names quoted for SQL (numeric names need quoting)
COLUMNS_SQL = [
    "Prefix",
    "Entity",
    "Mix",
    "Ph",
    "CW",
    "RT",
    "SAT",
    "[160]",
    "[80]",
    "[40]",
    "[30]",
    "[20]",
    "[17]",
    "[15]",
    "[12]",
    "[10]",
    "[6]",
    "[2]",
]


def main():
    if not LOG_CSV.exists():
        raise SystemExit(f"Missing {LOG_CSV}")

    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()

    cur.execute("DROP TABLE IF EXISTS items;")
    cur.execute(
        """
        CREATE TABLE IF NOT EXISTS items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            Prefix TEXT,
            Entity TEXT,
            Mix TEXT,
            Ph TEXT,
            CW TEXT,
            RT TEXT,
            SAT TEXT,
            [160] TEXT,
            [80] TEXT,
            [40] TEXT,
            [30] TEXT,
            [20] TEXT,
            [17] TEXT,
            [15] TEXT,
            [12] TEXT,
            [10] TEXT,
            [6] TEXT,
            [2] TEXT
        );
        """
    )

    with LOG_CSV.open(newline="", encoding="utf-8") as f:
        reader = csv.reader(f)
        rows = []
        for row in reader:
            if not row:
                continue
            if len(row) > len(COLUMNS):
                # If extra columns, assume entity contains commas; merge extras into Entity
                extra = len(row) - len(COLUMNS)
                entity_parts = row[1 : 2 + extra]
                entity_joined = ",".join(entity_parts)
                row = [row[0], entity_joined] + row[2 + extra :]
            if len(row) < len(COLUMNS):
                row += [""] * (len(COLUMNS) - len(row))
            row = [col.strip() for col in row[: len(COLUMNS)]]
            # Replace X markers with V
            row = ["V" if col.upper() == "X" else col for col in row]
            rows.append(row)

    cur.executemany(
        f"""
        INSERT INTO items ({", ".join(COLUMNS_SQL)})
        VALUES ({", ".join(["?"] * len(COLUMNS_SQL))})
        """,
        rows,
    )

    conn.commit()
    conn.close()
    print(f"Imported {len(rows)} rows into {DB_PATH}")


if __name__ == "__main__":
    main()
