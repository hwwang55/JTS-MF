#include "DS.h"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include "database.h"
#include "dbfactory.h"
#include <math.h>

extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern map< pair<string, string>, map< size_u, map<size_u, float> > > multi_matching_score;
extern map< size_u, map<size_u, float> > user_user_group_level_matching_score;
Id_Index id_Index;


void load_data(string localhost, string username, string password, string table) {
	database* db = dbfactory::createDataBase(localhost, username, password, table);

	map< string, vector<size_u> > id_index;
	Channels channels;
	cout << "loading all ids ...\n";

	// loading user id
	db->getAllId("user_id_all", id_Index.id_index[channels.channel_map[0]]);
	cout << "user id loading done\t" << id_Index.id_index[channels.channel_map[0]].size() << endl;

	// loading vote id
	db->getAllId("vote_id_all_1", id_Index.id_index[channels.channel_map[1]]);
	cout << "vote id loading done\t" << id_Index.id_index[channels.channel_map[1]].size() << endl;

	// loading group id
	db->getAllId("group_id_all_1", id_Index.id_index[channels.channel_map[2]]);
	cout << "group id loading done\t" << id_Index.id_index[channels.channel_map[2]].size() << endl << endl;

	// loading training set
	cout << "loading user_vote ...\n";
	db->getUserItemInteract("user_vote_80", Channels().channel_map[1], users, multi_items);
	cout << "user vote loading done\n\n";

	// loading test set
	cout << "loading user_vote test ...\n";
	db->getUserItemInteract("user_vote_20", Channels().channel_map[1], users, multi_items, 1);
	cout<<"loading vote testing data done\n\n";


	/* added by Hongwei Wang at 2016/07/20 19:38 */
	if (gama != 0) {
		// loading vote-vote topic similarity
		cout << "loading vote-vote topic similarity ...\n";
		db->getMatchingScore("JT1_vote_vote", Pair_Channels().pair_channel_map[3], multi_matching_score);
		// normalize
		cout << "normalizing vote_vote topic similarity ...\n";
		for (map< size_u, map<size_u, float> >::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[3]].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[3]].end(); It++) {
			float total = 0.0;
			for (map<size_u, float>::iterator It1 = It->second.begin(); It1 != It->second.end(); It1++) {
				total += It1->second;
			}
			if (total != 0) {
				for (map<size_u, float>::iterator It1 = It->second.begin(); It1 != It->second.end(); It1++) {
					It1->second /= total;
				}
			}
		}
	}
	
	if (alpha != 0) {
		// loading user-user similarity
		cout << "\nloading user-user similarity ...\n";
		db->getMatchingScore("JT_user_user", Pair_Channels().pair_channel_map[1], multi_matching_score);
		// compute user-user trust value
		cout << "computing user-user trust value ...\n";
		for (map< size_u, map<size_u, float> >::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[1]].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[1]].end(); It++) {
			map<size_u, float> *followees = &It->second;
			int self_out_degree = followees->size() + 1;
			for (map<size_u, float>::iterator It1 = followees->begin(); It1 != followees->end(); It1++) {
				size_u followee_id = It1->first;
				int friend_in_degree = multi_items["user"][followee_id].user_list.size() + 1;
				float trust_val = sqrt(friend_in_degree * 1.0f / (friend_in_degree + self_out_degree));
				It1->second *= trust_val;
			}
		}
		// normalize
		cout << "normalizing user-user friend level parameter ...\n";
		for (map< size_u, map<size_u, float> >::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[1]].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[1]].end(); It++) {
			float total = 0.0;
			for (map<size_u, float>::iterator It1 = It->second.begin(); It1 != It->second.end(); It1++) {
				total += It1->second;
			}
			if (total != 0) {
				for (map<size_u, float>::iterator It1 = It->second.begin(); It1 != It->second.end(); It1++) {
					It1->second /= total;
				}
			}
		}
	}
	
	if (beta != 0) {
		// loading user-group similarity
		cout << "\nloading user-group similarity ...\n";
		db->getMatchingScore("JT_user_group", Pair_Channels().pair_channel_map[2], multi_matching_score);
		// compute user-user group level matching score
		cout << "computing user-user group level matching score ...\n";
		for (map< size_u, map<size_u, float> >::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[2]].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[2]].end(); It++) {
			size_u user_id = It->first;
			map<size_u, float> *groups = &It->second;
			for (map<size_u, float>::iterator It1 = groups->begin(); It1 != groups->end(); It1++) {
				size_u group_id = It1->first;
				float sim = It1->second;
				vector<size_u> *other_users = &multi_items["group"][group_id].user_list;
				for (vector<size_u>::iterator It2 = other_users->begin(); It2 != other_users->end(); It2++) {
					size_u other_user_id = *It2;
					if (other_user_id != user_id) {
						user_user_group_level_matching_score[user_id][other_user_id] += sim;
					}
				}
			}
		}
		// normalize
		cout << "normalizing user-user group level parameter ...\n";
		for (map< size_u, map<size_u, float> >::iterator It = user_user_group_level_matching_score.begin(); It != user_user_group_level_matching_score.end(); It++) {
			float total = 0.0;
			for (map<size_u, float>::iterator It1 = It->second.begin(); It1 != It->second.end(); It1++) {
				size_u user_id_2 = It1->first;
				total += It1->second;
			}
			if (total != 0) {
				for (map<size_u, float>::iterator It1 = It->second.begin(); It1 != It->second.end(); It1++) {
					It1->second /= total;
				}
			}
		}
	}

	cout << "\nload data done\n\n";

	// ============================================================== //


	/** 
	 * directly load user-group information
	 * deprecated
	 */
	/*
    // loading user-group similarity
	cout << "loading user-group similarity ...\n";
	db->getMatchingScore("user_group_score", Pair_Channels().pair_channel_map[2], multi_matching_score);
	cout << "loading user-group similarity done\n";
	// normalize user-group matching score
	cout << "normalizing user-group similarity ...\n";
	for (map< size_u, map<size_u, float> >::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[2]].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[2]].end(); It++) {
        size_u user_id = It->first;//cout << user_id << endl;
        map<size_u, float> *groups = &It->second;
        float total = 0.0;
        for (map<size_u, float>::iterator It1 = groups->begin(); It1 != groups->end(); It1++) {
            size_u group_id = It1->first;
            float sim = It1->second;
            total += sim * (multi_items["group"][group_id].user_list.size() - 1);
        }
        if (total != 0) {
            for (map<size_u, float>::iterator It1 = groups->begin(); It1 != groups->end(); It1++) {
                It1->second /= total;
            }
        }
	}
	cout << "normalizing user-group similarity done\n\n";
	*/
}
