#include <iostream>
#include "banking.h"

using namespace Banking;
using namespace std;

Frontend::Frontend() : Backend::Backend()
{
    this->starting_prompt();
}

void Frontend::starting_prompt()
{
    cout << "[1] Login\n[2] Register" << endl;

    int choice;
    cin >> choice;

    switch (choice)
    {
    case 1:
        this->login_prompt();
        break;
    case 2:
        this->register_prompt();
        break;
    default:
        cout << "Invalid choice. Please try again." << endl;
        this->starting_prompt();
        break;
    }
}

void Frontend::login_prompt()
{
    if (this->over_max_attempts())
        return;

    cout << "Please enter your username." << endl;

    Username username;
    cin >> username;

    if (!this->user_exists(username))
    {
        this->current_attempts += 1;
        cout << "This username was not found" << endl;
        this->login_prompt();
        return;
    }

    goto enter_pin;

enter_pin:
    if (this->current_attempts >= this->max_attempts)
    {
        cout << "You have made too many attempts, please try again later." << endl;
        return;
    }

    cout << "Please enter your pin to login." << endl;

    int pin;
    cin >> pin;

    if (!this->validate_pin(username, pin))
    {
        this->current_attempts += 1;
        cout << "Invalid pin." << endl;
        goto enter_pin;
    }

    this->login_user(username, pin);
    this->username = username; 
    this->client_view();
}

void Frontend::client_view()
{
    this->current_attempts = 0;

    cout << "Welcome " << this->username << " you have $" << this->get_balance() << endl;
    cout << "[1] Deposit\n[2] Send\n[3] Withdraw" << endl;

    int choice;
    cin >> choice;

    switch (choice)
    {
        // deposit
        case 1: {
        
            cout << "How much do you wish to deposit?" << endl;

            int amount;
            cin >> amount;

            this->deposit(amount);
            break;
        }
        
        // send
        case 2: {
            this->send_prompt(); 
            break;
        }
        
        // withdraw
        case 3: {
            cout << "How much do you wish to withdraw?" << endl;

            int amount;
            cin >> amount;

            this->withdraw(amount, pin); 
            break; 
        }
    }
}

void Frontend::send_prompt()
{
    if (this->over_max_attempts())
        return;

    cout << "Whom do you want to send to? (username)" << endl;

    Username username;
    cin >> username;

    if (!this->user_exists(username))
    {
        cout << "This user does not exist." << endl;
        this->send_prompt();
    }

    cout << "How much do you want to send?" << endl;

    int amount;
    cin >> amount;

    if (amount >= this->get_balance())
    {
        this->current_attempts += 1;
        cout << "You do not have a large enough balance for this transaction." << endl;
        this->send_prompt();
    }

    cout << "Please confirm your pin." << endl;

    int pin;
    cin >> pin;

    if (!this->validate_pin(username, pin))
    {
        this->current_attempts += 1;
        cout << "Invalid pin." << endl;
        this->send_prompt();
    }

    User to = this->find(username);
    this->send(amount, to, pin);
}

bool Frontend::over_max_attempts()
{
    if (this->current_attempts >= this->max_attempts)
    {
        cout << "You have been locked out because you made too many attempts, please try again later" << endl;

        return true;
    }
    else
    {
        return false;
    }
}

void Frontend::register_prompt()
{
    cout << "Please enter a username." << endl; 

    Username username; 
    cin >> username;

    cout << "Create a pin" << endl;
    
    int pin;
    cin >> pin; 

    this->username = username; 
    this->pin = pin; 
    this->register_user(username, pin); 
    this->client_view(); 
}