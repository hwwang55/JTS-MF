/* created by Hongwei Wang at 2016/07/21 15:35 */

#include <iostream>
#include <algorithm>
#include "DS.h"
#include "user.h"
#include "item.h"

using namespace std;

extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern map< pair<string, string>, map< size_u, map<size_u, float> > > multi_matching_score;
extern map< size_u, map<size_u, float> > user_user_group_level_matching_score;
extern Id_Index id_Index;
extern float step_size;


void user_update() {
	//cout << "user update begin ..." << endl;

	float r_m = RM().rm["voting"];
	float w_m = Weights().weights["voting"];
	const int USER_NUM = id_Index.id_index["user"].size();

	// public pTp matrix
	float pp_public[dim][dim] = { 0 };
	#pragma omp parallel for schedule(dynamic, 1)
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			for (map<size_u, Item>::iterator It = multi_items["voting"].begin(); It != multi_items["voting"].end(); It++) {
				pp_public[i][j] += It->second.item_latent_feature_previous[i] * It->second.item_latent_feature_previous[j];
			}
			pp_public[i][j] *= 2 * w_m;
		}
	}

	// for each user
	#pragma omp parallel for schedule(dynamic, 100)
	for (int m = 0; m < USER_NUM; m++) {
		/*
		if (m % 10000 == 0) {
		cout << "user no: " << m << endl;
		}
		*/

		size_u user_id = id_Index.id_index["user"][m];

		for (int n = 0; n < dim; n++){
			users[user_id].user_latent_feature_previous[n] = users[user_id].user_latent_feature[n];
		}

		vector<float> gradient(dim, 0.0);  // gradient: dL / dQi
		vector<float> first_part(dim, 0.0);
		vector<float> second_part(dim, 0.0);
		vector<float> third_part(dim, 0.0);
		vector<float> forth_part(dim, 0.0);
		vector<float> fifth_part(dim, 0.0);
		vector<float> sixth_part(dim, 0.0);

		// first part
		for (vector<size_u>::iterator It = users[user_id].ratings["voting"].begin(); It != users[user_id].ratings["voting"].end(); It++) {
			size_u item_id = *It;
			for (int n = 0; n < dim; n++) {
				first_part[n] += -2 * w * (DEFAULT_RATING - r_m) * multi_items["voting"][item_id].item_latent_feature_previous[n];
			}
		}

		float pp[dim][dim] = { 0 };
		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				for (vector<size_u>::iterator It = users[user_id].ratings["voting"].begin(); It != users[user_id].ratings["voting"].end(); It++) {
					size_u item_id = *It;
					pp[i][j] += multi_items["voting"][item_id].item_latent_feature_previous[i] * multi_items["voting"][item_id].item_latent_feature_previous[j];
				}
				pp[i][j] *= 2 * (w - w_m);
				pp[i][j] += pp_public[i][j];
			}
		}

		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				first_part[i] += users[user_id].user_latent_feature_previous[j] * pp[j][i];
			}
		}


		// second part
		for (int n = 0; n < dim; n++) {
			second_part[n] = 2 * lamda * users[user_id].user_latent_feature_previous[n];
		}


		if (alpha != 0) {
			// third part
			if (multi_matching_score[Pair_Channels().pair_channel_map[1]].find(user_id) != multi_matching_score[Pair_Channels().pair_channel_map[1]].end()) {
				for (int n = 0; n < dim; n++) {
					third_part[n] = users[user_id].user_latent_feature_previous[n];
				}
				for (map<size_u, float>::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[1]][user_id].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[1]][user_id].end(); It++) {
					size_u user_id_k = It->first;
					float SC = It->second;
					for (int n = 0; n < dim; n++) {
						third_part[n] -= SC * users[user_id_k].user_latent_feature_previous[n];
					}
				}
				for (int n = 0; n < dim; n++) {
					third_part[n] *= 2 * alpha;
				}
			}

			// forth part
			if (multi_items["user"].find(user_id) != multi_items["user"].end()) {
				for (vector<size_u>::iterator It = multi_items["user"][user_id].user_list.begin(); It != multi_items["user"][user_id].user_list.end(); It++) {
					size_u user_id_t = *It;
					float SC_ti = multi_matching_score[Pair_Channels().pair_channel_map[1]][user_id_t][user_id];

					vector<float> tmp(dim, 0.0);
					for (int n = 0; n < dim; n++) {
						tmp[n] = users[user_id_t].user_latent_feature_previous[n];
					}
					for (map<size_u, float>::iterator It_t = multi_matching_score[Pair_Channels().pair_channel_map[1]][user_id_t].begin(); It_t != multi_matching_score[Pair_Channels().pair_channel_map[1]][user_id_t].end(); It_t++) {
						size_u user_id_k = It_t->first;
						float SC_tk = It_t->second;
						for (int n = 0; n < dim; n++) {
							tmp[n] -= SC_tk * users[user_id_k].user_latent_feature_previous[n];
						}
					}
					for (int n = 0; n < dim; n++) {
						tmp[n] *= -SC_ti;
					}

					for (int n = 0; n < dim; n++) {
						forth_part[n] += tmp[n];
					}
				}
				for (int n = 0; n < dim; n++) {
					forth_part[n] *= 2 * alpha;
				}
			}
		}
		

		if (beta != 0) {
			if (user_user_group_level_matching_score.find(user_id) != user_user_group_level_matching_score.end()) {
				// fifth part
				for (int n = 0; n < dim; n++) {
					fifth_part[n] = users[user_id].user_latent_feature_previous[n];
				}
				for (map<size_u, float>::iterator It = user_user_group_level_matching_score[user_id].begin(); It != user_user_group_level_matching_score[user_id].end(); It++) {
					size_u user_id_k = It->first;
					float A = It->second;
					for (int n = 0; n < dim; n++) {
						fifth_part[n] -= A * users[user_id_k].user_latent_feature_previous[n];
					}
				}
				for (int n = 0; n < dim; n++) {
					fifth_part[n] *= 2 * beta;
				}


				// sixth part
				for (map<size_u, float>::iterator It = user_user_group_level_matching_score[user_id].begin(); It != user_user_group_level_matching_score[user_id].end(); It++) {
					size_u user_id_t = It->first;
					float A_ti = user_user_group_level_matching_score[user_id_t][user_id];

					vector<float> tmp(dim, 0.0);
					for (int n = 0; n < dim; n++) {
						tmp[n] = users[user_id_t].user_latent_feature_previous[n];
					}
					for (map<size_u, float>::iterator It_t = user_user_group_level_matching_score[user_id_t].begin(); It_t != user_user_group_level_matching_score[user_id_t].end(); It_t++) {
						size_u user_id_k = It_t->first;
						float A_tk = It_t->second;
						for (int n = 0; n < dim; n++) {
							tmp[n] -= A_tk * users[user_id_k].user_latent_feature_previous[n];
						}
					}
					for (int n = 0; n < dim; n++) {
						tmp[n] *= -A_ti;
					}

					for (int n = 0; n < dim; n++) {
						sixth_part[n] += tmp[n];
					}
				}
				for (int n = 0; n < dim; n++) {
					sixth_part[n] *= 2 * beta;
				}
			}
		}
		

		// computing gradient
		for (int n = 0; n < dim; n++) {
			gradient[n] = first_part[n] + second_part[n] + third_part[n] + forth_part[n] + fifth_part[n] + sixth_part[n];
		}

		// update item latent feature
		for (int n = 0; n < dim; n++) {
			users[user_id].user_latent_feature[n] -= gradient[n] * step_size;
		}
		



		// ===================================================== //

		/* deprecated */
		/*
		// first part
		for (map<size_u, Item>::iterator It = multi_items["voting"].begin(); It != multi_items["voting"].end(); It++) {
		size_u item_id = It->first;

		float I, R;
		vector<size_u> *vec = &users[user_id].ratings["voting"];
		if (find(vec->begin(), vec->end(), item_id) != vec->end()) {    // if the user rates the item
		I = w;
		R = DEFAULT_RATING;
		}
		else {
		I = w_m;
		R = r_m;
		}
		float QP = 0.0;
		for (int n = 0; n < dim; n++) {
		QP += users[user_id].user_latent_feature_previous[n] * It->second.item_latent_feature_previous[n];
		}
		for (int n = 0; n < dim; n++) {
		first_part[n] += -2 * I * (R - r_m - QP) * It->second.item_latent_feature_previous[n];
		}
		}
		*/

		/*
		// fifth part
		bool flag = false;
		map<size_u, float> *groups = &multi_matching_score[Pair_Channels().pair_channel_map[2]][user_id];
		// for each group
		for (map<size_u, float>::iterator It = groups->begin(); It != groups->end(); It++) {
			size_u group_c_id = It->first;
			float G_ic = It->second;
			vector<size_u> *other_users = &multi_items["group"][group_c_id].user_list;
			// for each user in the group
			for (vector<size_u>::iterator It1 = other_users->begin(); It1 != other_users->end(); It1++) {
				size_u user_k_id = *It1;
				if (user_k_id != user_id) {
					flag = true;
					for (int n = 0; n < dim; n++) {
						fifth_part[n] -= G_ic * users[user_k_id].user_latent_feature_previous[n];
					}
				}
			}
		}
		if (flag) {
			for (int n = 0; n < dim; n++) {
				fifth_part[n] += users[user_id].user_latent_feature_previous[n];
				fifth_part[n] *= 2 * beta;
			}
		}


		// sixth part
		for (map<size_u, float>::iterator It = groups->begin(); It != groups->end(); It++) {
			size_u group_d_id = It->first;
			vector<size_u> *other_users = &multi_items["group"][group_d_id].user_list;
			for (vector<size_u>::iterator It1 = other_users->begin(); It1 != other_users->end(); It1++) {
				size_u user_t_id = *It1;
				if (user_t_id != user_id) {
					float G_td = multi_matching_score[Pair_Channels().pair_channel_map[2]][user_t_id][group_d_id];

					vector<float> tmp(dim, 0.0);
					bool flag1 = false;
					map<size_u, float> *groups_t = &multi_matching_score[Pair_Channels().pair_channel_map[2]][user_t_id];
					for (map<size_u, float>::iterator It2 = groups_t->begin(); It2 != groups_t->end(); It2++) {
						size_u group_c_id = It2->first;
						float G_tc = It2->second;
						vector<size_u> *other_users = &multi_items["group"][group_c_id].user_list;
						for (vector<size_u>::iterator It3 = other_users->begin(); It3 != other_users->end(); It3++) {
							size_u user_k_id = *It3;
							if (user_k_id != user_t_id) {
								flag = true;
								for (int n = 0; n < dim; n++) {
									tmp[n] -= G_tc * users[user_k_id].user_latent_feature_previous[n];
								}
							}
						}
					}
					if (flag) {
						for (int n = 0; n < dim; n++) {
							tmp[n] += users[user_t_id].user_latent_feature_previous[n];
							tmp[n] *= -G_td;
						}
					}
					for (int n = 0; n < dim; n++) {
						sixth_part[n] += tmp[n];
					}
				}
			}
		}
		for (int n = 0; n < dim; n++) {
			sixth_part[n] *= 2 * beta;
		}
		*/
	}
}
