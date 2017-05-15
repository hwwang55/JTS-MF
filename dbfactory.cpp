#include "dbfactory.h"

database* dbfactory::createDataBase(string server, string user, string pass, string dbname) {
    database* db = new database(server, user, pass, dbname);
    return db;
}
