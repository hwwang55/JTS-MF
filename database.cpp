#include "database.h"
#include "paras.h"
#include "DS.h"
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <cassert>

using namespace std;

extern Id_Index id_Index;


database::database(string server, string user, string pass, string dbname) {
    mysql_init(&mysql);
    mysql_real_connect(&mysql, server.c_str(), user.c_str(), pass.c_str(), dbname.c_str(), 0, NULL, 0);
    mysql_set_character_set(&mysql, "gbk");
}


void database::close() {
    mysql_close(&mysql);
    delete this;
}


MYSQL_RES* database::querySQLCmd(string sql) {
    if (mysql_query(&mysql, sql.c_str())) {
        return NULL;
    }

    MYSQL_RES* result = mysql_store_result(&mysql);
    return result;
}


bool database::exeSQLCmd(string sql) {
    if (mysql_query(&mysql, sql.c_str())) {
        return false;
    }
    return true;
}


int database::getTotalRows(string table) {
    string sql = "select count(*) from " + table;
    MYSQL_RES* result = querySQLCmd(sql);
    if (result == NULL) {
        return -1;
    }

    int count = -1;
    MYSQL_ROW row;
    while(row = mysql_fetch_row(result)) {
        sscanf(row[0], "%d", &count);
    }
    mysql_free_result(result);
	result = NULL;

    return count;
}


void database::getAllId(string table, vector<size_u>& Ids) {

	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
    result = mysql_store_result(&mysql);

    while(row = mysql_fetch_row(result)) {
		string id = row[0];
		std::stringstream id_s(id);
		size_u id_i;
		id_s >> id_i;
		Ids.push_back(id_i);
	}
	mysql_free_result(result);
}


void database::getUserItemInteract(string table, string channel, map<size_u, User>& users, map< string, map<size_u, Item> >& multi_items) {
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
    result = mysql_store_result(&mysql);
	int count = 0;

    while(row = mysql_fetch_row(result)) {
        string id1 = row[0];
		string id2 = row[1];
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;
		users[id1_i].ratings[channel].push_back(id2_i);
		multi_items[channel][id2_i].user_list.push_back(id1_i);
		count++;
	}
	std::cout << "records #: " << count << endl;
	mysql_free_result(result);
}


void database::getUserItemInteract(string table, string channel, map<size_u, User>& users, map< string, map<size_u, Item> >& multi_items, bool isTest) {
	MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
    result = mysql_store_result(&mysql);
	int count = 0;

    while(row = mysql_fetch_row(result)) {
        string id1 = row[0];	// user id
		string id2 = row[1];	// item id
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		size_u id1_i, id2_i;
		id1_s >> id1_i;
		id2_s >> id2_i;
		users[id1_i].testset[channel].push_back(id2_i);
		count++;
	}
	std::cout << "records #: " << count << endl;
	mysql_free_result(result);
}


/* added by Hongwei Wang at 2016/07/20 19:14 */
void database::getMatchingScore(string table, pair<string, string> pair_channel, map< pair<string, string>, map< size_u, map<size_u, float> > >& multi_matching_score) {
    MYSQL_RES* result;
	string sql;
	MYSQL_ROW row;

	sql = "select * from " + table;
	mysql_query(&mysql, sql.c_str());
    result = mysql_store_result(&mysql);
	int count = 0;

    while(row = mysql_fetch_row(result))
    {
        string id1 = row[0];	// id 1
		string id2 = row[1];	// id 2
		string val = row[2];    // matching score
		std::stringstream id1_s(id1);
		std::stringstream id2_s(id2);
		std::stringstream val_s(val);
		size_u id1_i, id2_i;
		float val_f;
		id1_s >> id1_i;
		id2_s >> id2_i;
		val_s >> val_f;

		if (pair_channel.first == "voting" && pair_channel.second == "voting") {
			/* !!! watch out the threshold value !!! */
			if (val_f > 0) {
				multi_matching_score[pair_channel][id1_i][id2_i] = val_f;
				multi_matching_score[pair_channel][id2_i][id1_i] = val_f;
				count++;
			}
		}
		else if (pair_channel.first == "user" && pair_channel.second == "user") {
			/* !!! watch out the threshold value !!! */
			if (val_f > 0.8) {
				multi_matching_score[pair_channel][id1_i][id2_i] = val_f;
				multi_items["user"][id2_i].user_list.push_back(id1_i);
				count++;
			}
		}
		else if (pair_channel.first == "user" && pair_channel.second == "group") {
			/* !!! watch out the threshold value !!! */
			if (val_f > 0.685) {
				multi_matching_score[pair_channel][id1_i][id2_i] = val_f;
				multi_items["group"][id2_i].user_list.push_back(id1_i);
				count++;
			}
		}
	}
	std::cout << "records #: " << count << endl;
	mysql_free_result(result);
}
