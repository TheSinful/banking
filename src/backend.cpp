#include <iostream>
#include <stdexcept>
#include <sqlite3.h>
#include <random>
#include "banking.h"
#include "database.h"

using namespace Banking; 
using namespace std;

Backend::Backend() : Database::Database() 
{
    this->client = nullptr;
}

Backend::~Backend() 
{
    delete this->client; 
    this->close_db(); 
}

User Backend::find(Username username) {
    std::string query = "SELECT user_id, pin FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->database_client, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw runtime_error("Failed to prepare statement\n");
    }

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind username\n"); 
    }

    int step = sqlite3_step(stmt);
    if (step == SQLITE_ROW) {
        int user_id = sqlite3_column_int(stmt, 0);
        int pin = sqlite3_column_int(stmt, 1);
        User user = User(user_id, username, pin, *this);
        sqlite3_finalize(stmt);
        return user;
    } else {
        sqlite3_finalize(stmt);
        throw runtime_error("User not found\n"); 
    }
}

bool Backend::user_id_exists(int user_id) {
    std::string query = "SELECT COUNT(*) FROM users WHERE user_id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->database_client, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw runtime_error("Failed to prepare statement\n");
        return false;
    }

    if (sqlite3_bind_int(stmt, 1, user_id) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind user_id\n");
    }

    int step = sqlite3_step(stmt);
    bool exists = false;
    if (step == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        exists = (count > 0);
    } else {
        throw runtime_error("Failed to execute statement\n"); 
    }

    sqlite3_finalize(stmt);
    return exists;
}

int Backend::generate_user_id() {
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> int_distribution(1, 1000000); 

    int user_id = int_distribution(generator);
    if (user_id_exists(user_id)) {
        return generate_user_id(); 
    } else {
        return user_id;
    }
}

bool Backend::is_logged_in() {
    if (this->client == nullptr) {
        return false; 
    } else {
        return true; 
    }
}

bool Backend::user_exists(Username username) {
    if (this->database_client == nullptr) 
        throw runtime_error("Database not opened\n"); 
    

    std::string query = "SELECT COUNT(*) FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->database_client, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
        throw runtime_error("Failed to prepare statement\n");
    

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind username\n"); 
    }

    int step = sqlite3_step(stmt);
    bool exists = false;
    if (step == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
}

bool Backend::validate_pin(const Username& username, int pin)
{
    User user = this->find(username); 

    return user.validate_pin(pin);     
}


void Backend::send(int amount, User& to, int pin) {
    if (!this->is_logged_in()) {
        throw std::runtime_error("User not logged in."); 
    }
    
    this->client->send(amount, to, pin);
    this->deduct(this->client->get_username(), amount); 
    this->add(to.get_username(), amount); 
}

void Backend::deposit(int amount) {
    if (!this->is_logged_in()) {
        throw std::runtime_error("User not logged in."); 
    }

    this->client->deposit(amount);
    this->add(this->client->get_username(), amount); 
}

void Backend::withdraw(int amount, int pin) {
    if (!this->is_logged_in()) {
        throw std::runtime_error("User not logged in."); 
    }

    this->client->withdraw(amount, pin); 
    this->deduct(this->client->get_username(), amount); 
}

void Backend::login_user(Username username, int pin) {
    User user = this->find(username);
    
    if (!user.validate_pin(pin)) 
        throw runtime_error("Invalid pin"); 
    

    // chatgpt clutch (i hope)
    delete this->client;
    this->client = new User(user);
    this->client->update_balance();
}

void Backend::register_user(Username username, int pin) {
    if (this->database_client == nullptr) {
        throw runtime_error("this->client not initialized."); 
    }

    std::string query = "INSERT INTO users (username, user_id, pin, balance) VALUES (?, ?, ?, ?)"; 
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->database_client, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw runtime_error("Failed to prepare statement\n");
    }

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind username\n"); 
    }

    int user_id = this->generate_user_id(); 
    if (sqlite3_bind_int(stmt, 2, user_id) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind user_id\n"); 
    }

    if (sqlite3_bind_int(stmt, 3, pin) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind pin\n"); 
    }

    if (sqlite3_bind_int(stmt, 4, 0) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind balance\n"); 
    }

    int step = sqlite3_step(stmt);
    if (step != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to execute statement\n"); 
    }

    sqlite3_finalize(stmt);

    this->client = new User(user_id, username, pin, *this);  
}

void Backend::log_users_table() {
    const char* sql = "SELECT username, user_id, pin, balance FROM users";
    char* errMsg = nullptr;

    auto callback = [](void* NotUsed, int argc, char** argv, char** azColName) -> int {
        for (int i = 0; i < argc; i++) {
            std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << std::endl;
        }
        std::cout << std::endl;
        return 0;
    };

    int rc = sqlite3_exec(this->database_client, sql, callback, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error_msg = "Failed to log table: " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw runtime_error(error_msg);
    }
}

int Backend::get_balance() {
    // cout << "\nLogging users\n" << endl;
    // this->log_users_table(); 

    std::string query = "SELECT balance FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->database_client, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw runtime_error("Failed to prepare statement\n");
    }

    Username username = this->client->get_username(); 
    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind username\n"); 
    }

    int balance = -1;
    int step = sqlite3_step(stmt);
    if (step == SQLITE_ROW) {
        balance = sqlite3_column_int(stmt, 0);
    } else {
        throw runtime_error("User not found"); 
    }

    sqlite3_finalize(stmt);
    return balance;
}

bool Backend::update_balance(Username username, int amount) {
    int current_balance = get_balance();
    if (current_balance == -1) {
        return false;
    }

    int new_balance = current_balance + amount;
    std::string query = "UPDATE users SET balance = ? WHERE username = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->database_client, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
        throw runtime_error("Failed to prepare statement\n");
    

    if (sqlite3_bind_int(stmt, 1, new_balance) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind new balance\n"); 
    }

    if (sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to bind username\n");
    }

    int step = sqlite3_step(stmt);
    if (step != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw runtime_error("Failed to execute statement\n"); 
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Backend::add(Username username, int amount) {
    return update_balance(username, amount); 
}

bool Backend::deduct(Username username, int amount) {
    return update_balance(username, -amount); 
}