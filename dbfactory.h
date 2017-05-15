#ifndef	__dbfactory_h
#define	__dbfactory_h

#include <string>
#include "database.h"
using namespace std;

class dbfactory {
    private:
        dbfactory();

    public:
        static database* createDataBase(string server, string user, string pass, string dbname);
};

#endif
