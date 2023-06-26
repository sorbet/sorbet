#pragma once
#include "absl/strings/str_join.h"
#include "common/common.h"
#include "sqlite3.h"
#include <memory>

namespace sorbet::sqlitepp {

enum SqliteColumnType {
    Null,
    Integer,
    Real,
    Text,
    Blob,
};

enum SqliteColumnConstraint {
    PRIMARY_KEY,
    UNIQUE,
    NOT_NULL,
};

struct SqliteColumn {
    const std::string name;
    const SqliteColumnType dataType;
    const std::vector<SqliteColumnConstraint> constraints;

public:
    SqliteColumn(const std::string name, const SqliteColumnType dataType)
        : name(name), dataType(dataType), constraints() {}

    const std::string getName() const {
        return name;
    }

    const std::string toString() const {
        return absl::StrCat(name, " ", dataTypeToString(dataType), " ", constraintSetToString());
    }

    const std::string constraintSetToString() const {
        std::string s = "";
        for (auto &constraint : constraints) {
            absl::StrAppend(&s, constraintToString(constraint), " ");
        }

        return s;
    }

    const std::string constraintToString(SqliteColumnConstraint constraint) const {
        switch (constraint) {
            case UNIQUE:
                return "UNIQUE";
            case PRIMARY_KEY:
                return "PRIMARY KEY";
            case NOT_NULL:
                return "NOT NULL";
        }
    }

    const std::string dataTypeToString(SqliteColumnType type) const {
        switch (type) {
            case Null:
                return "NULL";
            case Integer:
                return "INTEGER";
            case Real:
                return "REAL";
            case Text:
                return "TEXT";
            case Blob:
                return "BLOB";
        }
    }
};

struct SqliteSchema {
    const std::vector<SqliteColumn> columns;

public:
    const std::string toString() const {
        std::string s = "(";
        std::vector<std::string> cols;
        for (auto &col : columns) {
            cols.push_back(col.toString());
        }
        return absl::StrCat("(", absl::StrJoin(cols, ","), ")");
    }

    const std::string toStringOnlyNames() const {
        std::string s = "(";
        std::vector<std::string> cols;
        for (auto &col : columns) {
            cols.push_back(col.getName());
        }
        return absl::StrCat("(", absl::StrJoin(cols, ","), ")");
    }
};

struct SqliteTable {
    const std::string tableName;
    const SqliteSchema schema;
    const std::string createStmt;
    sqlite3_stmt *insertStmt;

    static std::string generateCreateStmt(const std::string name, const SqliteSchema schema);

    SqliteTable(const std::string tableName, const SqliteSchema schema)
        : tableName(tableName), schema(schema), createStmt(generateCreateStmt(tableName, schema)), insertStmt(nullptr) {
    }
};

class SqliteDb {
    sqlite3 *db;
    sqlite3_stmt *insertStatement;
    void simpleExec(const std::string &query);

public:
    SqliteDb(const std::string dbName) {
        sqlite3_open(dbName.c_str(), &db);
    }

    ~SqliteDb() {
        sqlite3_close(db);
    }

    void create(const SqliteTable table);
    void generateInsertStmt(SqliteTable &table);
    void insert(const SqliteTable table, const std::vector<std::variant<int, double, std::string>> &values);
};
} // namespace sorbet::sqlitepp
