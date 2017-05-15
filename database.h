#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <list>
#include <map>
#include <set>
#include <cstring>
#include <vector>
#include <stdio.h>
#include "user.h"
#include "item.h"
#include "DS.h"
#include <mysql/mysql.h>

using namespace std;

extern map<size_u, User> users;
extern map<string, map<size_u, Item> > multi_items;

class database
{
    private:
        database(string server, string user, string pass, string dbname);
        MYSQL mysql;

    public:
        friend class dbfactory;
        void close();
        MYSQL_RES* querySQLCmd(string sql);
        bool exeSQLCmd(string sql);
		int getTotalRows(string table);
		void getAllId(string table, vector<size_u>& Ids);
		void getUserItemInteract(std::string table, string channel, std::map<size_u,User>& users, std::map<string,map<size_u,Item> >& multi_items);
		void getUserItemInteract(std::string table, string channel, std::map<size_u,User>& users, std::map<string,map<size_u,Item> >& multi_items, bool isTest);
		void getMatchingScore(string table, pair<string, string> pair_channel, map< pair<string, string>, map< size_u, map<size_u, float> > >& multi_matching_score);
};

#endif
