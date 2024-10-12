#ifndef BANKING_H
#define BANKING_H

#include "user.h"
#include "database.h"
#include <vector> 
#include <sqlite3.h>

namespace Banking
{

    class Backend : public Database 
    {
    public: 
        Backend();
        ~Backend(); 
    protected: 
        void login_user(Username username, int pin); 
        void register_user(Username username, int pin); 

        // utils functions
        bool user_exists(Username username); 
        User find(Username username); 

        // wrappers for existing functions of User 
        int get_balance(); 
        bool validate_pin(const Username& username, int pin);
        void deposit(int amount); 
        void send(int amount, User& to, int pin); 
        void withdraw(int amount, int pin); 

    private: 
        User* client; 
                
        int generate_user_id(); 
        bool is_logged_in(); 
        bool user_id_exists(int user_id); 
        bool update_balance(Username username, int amount); 
        bool deduct(Username username, int amount); 
        bool add(Username username, int amount);  
        void log_users_table(); 
    }; 

    /**
     * Class that handles all visual prompts
     */
    class Frontend : public Backend
    {
    public: 
        Frontend();

        void starting_prompt(); 
        void login_prompt(); 
        void register_prompt(); 
    private: 
        void client_view(); 
        void send_prompt(); 
        bool over_max_attempts();

        int max_attempts = 5; // for both login and register
        int current_attempts = 0; 

        Username username; 
        int pin; 
    }; 

}

#endif