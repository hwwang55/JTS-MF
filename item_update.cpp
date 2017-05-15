/* created by Hongwei Wang at 2016/07/20 16:05 */

#include <iostream>
#include <algorithm>
#include "DS.h"
#include "user.h"
#include "item.h"

using namespace std;

extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern map< pair<string, string>, map< size_u, map<size_u, float> > > multi_matching_score;
extern Id_Index id_Index;
extern float step_size;


void item_update() {
	//cout << "item update begin ..." << endl;

	float r_m = RM().rm["voting"];
	float w_m = Weights().weights["voting"];
	const int ITEM_NUM = id_Index.id_index["voting"].size();

	// public qTq matrix
	float qq_public[dim][dim] = { 0 };
	#pragma omp parallel for schedule(dynamic, 1)
	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			for (map<size_u, User>::iterator It = users.begin(); It != users.end(); It++) {
				qq_public[i][j] += It->second.user_latent_feature[i] * It->second.user_latent_feature[j];
			}
			qq_public[i][j] *= 2 * w_m;
		}
	}

	// for each item
	#pragma omp parallel for schedule(dynamic, 100)
	for (int m = 0; m < ITEM_NUM; m++) {
		/*
		if (m % 10000 == 0) {
			cout << "item no: " << m << endl;
		}
		*/

		size_u item_id = id_Index.id_index["voting"][m];

		// save
		for (int n = 0; n < dim; n++) {
			multi_items["voting"][item_id].item_latent_feature_previous[n] = multi_items["voting"][item_id].item_latent_feature[n];
		}

		vector<float> gradient(dim, 0.0);  // gradient: dL / dPj
		vector<float> first_part(dim, 0.0);
		vector<float> second_part(dim, 0.0);
		vector<float> third_part(dim, 0.0);
		vector<float> forth_part(dim, 0.0);


		// first part
		for (vector<size_u>::iterator It = multi_items["voting"][item_id].user_list.begin(); It != multi_items["voting"][item_id].user_list.end(); It++) {
			size_u user_id = *It;
			for (int n = 0; n < dim; n++) {
				first_part[n] += -2 * w * (DEFAULT_RATING - r_m) * users[user_id].user_latent_feature[n];
			}
		}

		float qq[dim][dim] = { 0 };
		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				for (vector<size_u>::iterator It = multi_items["voting"][item_id].user_list.begin(); It != multi_items["voting"][item_id].user_list.end(); It++){
					size_u user_id = *It;
					qq[i][j] += users[user_id].user_latent_feature[i] * users[user_id].user_latent_feature[j];
				}
				qq[i][j] *= 2 * (w - w_m);
				qq[i][j] += qq_public[i][j];
			}
		}

		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				first_part[i] += qq[i][j] * multi_items["voting"][item_id].item_latent_feature_previous[j];
			}
		}


		// second part
		for (int n = 0; n < dim; n++) {
			second_part[n] = 2 * lamda * multi_items["voting"][item_id].item_latent_feature_previous[n];
		}


		if (gama != 0) {
			if (multi_matching_score[Pair_Channels().pair_channel_map[3]].find(item_id) != multi_matching_score[Pair_Channels().pair_channel_map[3]].end()) {
				// third part
				for (int n = 0; n < dim; n++) {
					third_part[n] = multi_items["voting"][item_id].item_latent_feature_previous[n];
				}
				for (map<size_u, float>::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[3]][item_id].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[3]][item_id].end(); It++) {
					size_u item_id_t = It->first;
					float T = It->second;

					for (int n = 0; n < dim; n++) {
						third_part[n] -= T * multi_items["voting"][item_id_t].item_latent_feature_previous[n];
					}
				}
				for (int n = 0; n < dim; n++) {
					third_part[n] *= 2 * gama;
				}

				// forth part
				for (map<size_u, float>::iterator It = multi_matching_score[Pair_Channels().pair_channel_map[3]][item_id].begin(); It != multi_matching_score[Pair_Channels().pair_channel_map[3]][item_id].end(); It++) {
					size_u item_id_k = It->first;
					float T_kj = multi_matching_score[Pair_Channels().pair_channel_map[3]][item_id_k][item_id];

					vector<float> tmp(dim, 0.0);
					for (int n = 0; n < dim; n++) {
						tmp[n] = multi_items["voting"][item_id_k].item_latent_feature_previous[n];
					}
					for (map<size_u, float>::iterator It_k = multi_matching_score[Pair_Channels().pair_channel_map[3]][item_id_k].begin(); It_k != multi_matching_score[Pair_Channels().pair_channel_map[3]][item_id_k].end(); It_k++) {
						size_u item_id_t = It_k->first;
						float T_kt = It_k->second;
						for (int n = 0; n < dim; n++) {
							tmp[n] -= T_kt * multi_items["voting"][item_id_t].item_latent_feature_previous[n];
						}
					}
					for (int n = 0; n < dim; n++) {
						tmp[n] *= -T_kj;
					}

					for (int n = 0; n < dim; n++) {
						forth_part[n] += tmp[n];
					}
				}
				for (int n = 0; n < dim; n++) {
					forth_part[n] *= 2 * gama;
				}
			}
		}
		

		// computing gradient
		for (int n = 0; n < dim; n++) {
			gradient[n] = first_part[n] + second_part[n] + third_part[n] + forth_part[n];
		}

		// update item latent feature
		for (int n = 0; n < dim; n++) {
			multi_items["voting"][item_id].item_latent_feature[n] -= gradient[n] * step_size;
		}


		// =============================================================== //

		/* deprecated */
		/*
		// first part
		for (map<size_u, User>::iterator It = users.begin(); It != users.end(); It++) {
			size_u user_id = It->first;

			float I, R;
			vector<size_u> *vec = &multi_items["voting"][item_id].user_list;
			if (find(vec->begin(), vec->end(), user_id) != vec->end()) {    // if the item is rated by the user
				I = w;
				R = DEFAULT_RATING;
			}
			else {
				I = w_m;
				R = r_m;
			}
			float QP = 0.0;
			for (int n = 0; n < dim; n++) {
				QP += It->second.user_latent_feature_previous[n] * multi_items["voting"][item_id].item_latent_feature_previous[n];
			}
			for (int n = 0; n < dim; n++) {
				first_part[n] += -2 * I * (R - r_m - QP) * It->second.user_latent_feature_previous[n];
			}
		}
		*/
	}
}
