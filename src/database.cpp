#include "database.h"
#include <iostream>

using namespace std; 

Database::Database() {
    this->open_db(); 
}

void Database::open_db() {
    int exit = sqlite3_open("banking.db", &this->database_client); 
    if (exit) {
        throw runtime_error("Error opening db."); 
    }

    const char* sql = 
    "CREATE TABLE IF NOT EXISTS users ("
    "username TEXT PRIMARY KEY, "
    "user_id INTEGER UNIQUE, "
    "pin INTEGER, "
    "balance INTEGER);";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(this->database_client, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        string error_msg = "Failed to create table: " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw runtime_error(error_msg);
    }
}

void Database::close_db() {
    sqlite3_close(this->database_client); 
}

db_client* Database::get_db_client() const {
    return this->database_client; 
}