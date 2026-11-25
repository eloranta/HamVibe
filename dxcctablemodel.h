#pragma once

#include <QSqlTableModel>

class DxccTableModel final : public QSqlTableModel
{
public:
    using QSqlTableModel::QSqlTableModel;

    bool select() override
    {
        if (!QSqlTableModel::select()) {
            return false;
        }
        // Keep all 340 entities cached: a live, partially fetched SQLite query
        // can repeat rows when another writer changes the table.
        while (canFetchMore()) {
            fetchMore();
        }
        return true;
    }

    bool insertRows(int, int, const QModelIndex & = QModelIndex()) override
    {
        return false;
    }
};
