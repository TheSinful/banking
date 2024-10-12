#include "user.h"
#include <iostream>
#include <stdexcept>
#include <sqlite3.h>
#include "database.h"

using namespace std; 

User::User(int user_id, Username username, int pin, Database& database)
{
    this->user_id = user_id;
    this->username = username;
    this->pin = pin;
    this->balance = 0;
    this->database = &database;
    this->update_balance();  
};

void User::deposit(int amount)
{
    this->add(amount, this->pin);

    std::cout << "User " << this->username << " deposited $" << amount << std::endl;
}

void User::withdraw(int amount, int pin)
{
    if (amount > this->balance)
    {
        throw std::runtime_error("There is not enough balance to withdraw!");
    }

    this->subtract(amount, pin);

    std::cout << "User" << this->username << "withdrew" << amount << std::endl;
}

void User::receive(int amount, User &from)
{
    this->add(amount, this->pin);

    std::cout << "User" << this->username << "received" << amount << "from" << from.username << std::endl;
}

void User::send(int amount, User &to, int pin)
{
    this->subtract(amount, pin);
    to.receive(amount, *this);

    std::cout << "User" << this->username << "sent" << amount << "to" << to.username << std::endl;
}

void User::add(int amount, int pin)
{
    if (amount < 0)
    {
        throw std::invalid_argument("Amount must be non-negative");
    }

    if (!this->validate_pin(pin))
    {
        throw std::invalid_argument("Invalid pin");
    }

    this->balance += amount;
}

void User::subtract(int amount, int pin)

{
    if (amount < 0)
    {
        throw std::invalid_argument("Amount must be non-negative");
    }

    if (!this->validate_pin(pin))
    {
        throw std::invalid_argument("Invalid pin");
    }

    this->balance -= amount;
}

bool User::validate_pin(int pin)
{
    if (pin == this->pin)
        return true;
    else
        return false;
}

void User::update_balance()
{
    std::string query = "SELECT user_id, pin FROM users WHERE username = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(this->database->get_db_client(), query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare statement\n");
    }

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to bind username\n");
    }

    int step = sqlite3_step(stmt);
    if (step == SQLITE_ROW) {
        int balance = sqlite3_column_int(stmt, 2);
        this->balance = balance; 
        sqlite3_finalize(stmt);
    } else {
        sqlite3_finalize(stmt);
        throw runtime_error("User not found\n"); 
    }
}

long User::get_balance() const
{
    return this->balance;
}

int User::get_user_id() const
{
    return this->user_id;
}

Username User::get_username() const
{
    return this->username;
}
