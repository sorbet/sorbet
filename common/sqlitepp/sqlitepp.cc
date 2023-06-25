#include "sqlitepp.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"

namespace sorbet::sqlitepp {
void SqliteDb::create(const SqliteTable table) {
    auto query = absl::StrCat("CREATE TABLE ", table.tableName, table.schema.toString(), ";");
    simpleExec(query);
}

void SqliteDb::insert(const SqliteTable table, const std::vector<std::variant<int, float, std::string>> &values) {
    auto query = absl::StrCat("INSERT INTO ", table.tableName, " ", table.schema.toStringOnlyNames(), " VALUES (",
                              absl::StrJoin(values, ","), ")", ";");
    simpleExec(query);
}
void SqliteDb::simpleExec(const std::string &query) {
    char *errMsg;
    auto rc = sqlite3_exec(db, query.c_str(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        fprintf(stdout, "Table created successfully\n");
    }
}

} // namespace sorbet::sqlitepp
