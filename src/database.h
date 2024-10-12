#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"

using db_client = sqlite3; 

class Database
{
public: 
    Database(); 
    void open_db(); 
    void close_db();
    db_client* get_db_client() const; 
protected: 
    db_client* database_client; 
}; 

#endif