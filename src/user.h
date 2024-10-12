#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "database.h"

using Username = std::string; 

class User 
{
public: 

    User(int user_id, Username username, int pin, Database& database); 

    void receive(int amount, User& from); // tested
    void send(int amount, User& to, int pin); // tested
    void deposit(int amount); // tested
    void withdraw(int amount, int pin);  // tested
    bool validate_pin(int pin); // tested
    void update_balance(); 
    
    long get_balance() const; 
    int get_user_id() const; 
    Username get_username() const; 

private: 
    long balance; 
    int user_id; 
    Username username; 
    Database* database; 
    db_client* database_client; 

    int pin; 
    void add(int amount, int pin);
    void subtract(int amount, int pin);
}; 

#endif 