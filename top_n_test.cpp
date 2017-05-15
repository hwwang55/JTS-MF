#include "DS.h"
#include "user.h"
#include "item.h"
#include <fstream>
#include <string>
#include <set>
#include <utility>
#include <list>

using namespace std;

extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern Id_Index id_Index;

extern float step_size;
extern float decay_rate;


bool comp2(pair<size_u, float> first, pair<size_u, float> second) {
	if( first.second > second.second) {
        return true;
	} else {
        return false;
    }
}


// top-k recommendation for given channel
int top_n_test(string channel) {
	Total_liked total_liked;
	Hit_liked hit_liked;
	const int USER_NUM = id_Index.id_index["user"].size();

	// for every user
	#pragma omp parallel for schedule(dynamic, 10)
	for (int u = 0; u < USER_NUM; u += 100) {
		/*
		if (u % 10000 == 0) {
            cout << "user: " << u << endl;
        }
		*/

		size_u user_id = id_Index.id_index["user"][u];

		// if this user's test set is available
		if (users[user_id].testset.size() > 0) {
			list< pair<size_u, float> > recomm_pool; // store all predicted item-rating pair
			set<size_u> training_set(users[user_id].ratings["voting"].begin(), users[user_id].ratings["voting"].end());
			map<size_u, Item>::iterator MIt1;

			// for every item
			for (MIt1 = multi_items[channel].begin(); MIt1 != multi_items[channel].end(); MIt1++) {
				size_u item_id = MIt1->first;

				// if this item is not rated by the user
				if (training_set.count(item_id) == 0) {
					float predict_rating = 0;

					// compute predicted rating
					for (int i = 0; i < dim; i++) {
                        predict_rating += MIt1->second.item_latent_feature[i] * users[user_id].user_latent_feature[i];
                    }
					pair<size_u, float> mypair(item_id, predict_rating);
					recomm_pool.push_back(mypair);
				}
			}

			// sort all predicted ratings
			recomm_pool.sort(comp2);

			// fill top-n sets		key: top_n, val: item_id
			map< int, set<size_u> > Recomm_item_set;
			TopN topN;
			vector<int>::iterator VIt;
			for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
				int top_n = *VIt;
				int count = 0;
				for (list< pair<size_u,float> >::iterator LIt = recomm_pool.begin(); LIt != recomm_pool.end() && count < top_n; LIt++, count++){
					Recomm_item_set[top_n].insert((*LIt).first);
				}
			}
			recomm_pool.clear();

			// the go through current user's test set to test hit ratio
			for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
				int top_n = *VIt;
				vector<size_u>::iterator It;
				for (It = users[user_id].testset[channel].begin(); It != users[user_id].testset[channel].end(); It++) {
					#pragma omp atomic
					total_liked.total_liked[top_n]++;

					if (Recomm_item_set[top_n].count(*It) > 0) {
						#pragma omp atomic
						hit_liked.hit_liked[top_n]++;
					}
				}
			}
		}

	}

	char str[1000];
	sprintf(str, "/home/centos/data/weibo_MF_code/voting_MF_hongwei/jt_results/%d_%d_%d_dim=%d.txt", alpha, beta, gama, dim);
	ofstream o1(str, ios_base::app);
	assert(o1);

	TopN topN;
	vector<int>::iterator VIt;

	// for iteration
	/*
	Recall recall;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		recall.recall_total[top_n] = float(hit_liked.hit_liked[top_n]) / total_liked.total_liked[top_n];
	}
	o1 << iteration << '\t' << recall.recall_total[10] << endl;
	cout << iteration << '\t' << recall.recall_total[10] << '\t' << step_size << endl;
	*/

	/* added by Hongwei Wang at 2016/09/25 17:01 */
	// recall
	Recall recall;
	o1 << "recall:" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		recall.recall_total[top_n] = float(hit_liked.hit_liked[top_n]) / total_liked.total_liked[top_n];
		o1 << recall.recall_total[top_n] << "\t";
	}
	o1 << endl;

	// precision
	Precision precision;
	o1 << "precision:" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		precision.precision_total[top_n] = float(hit_liked.hit_liked[top_n]) / (top_n * USER_NUM / 100);
		o1 << precision.precision_total[top_n] << "\t";
	}
	o1 << endl;

	// f value
	o1 << "fvalue:" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		float fvalue = 2 * precision.precision_total[top_n] * recall.recall_total[top_n] / (precision.precision_total[top_n] + recall.recall_total[top_n]);
		o1 << fvalue << "\t";
	}
	o1 << endl;

	// roc curve
	o1 << "roc curve points:" << endl;
	o1 << "x\ty" << endl;
	for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
		int top_n = *VIt;
		float fpr = float(top_n * USER_NUM / 100 - hit_liked.hit_liked[top_n]) / (id_Index.id_index["voting"].size() - top_n * USER_NUM / 100 - total_liked.total_liked[top_n] + hit_liked.hit_liked[top_n]);
		float tpr = precision.precision_total[top_n];
		o1 << fpr << "\t" << tpr << endl;
	}
	o1 << endl << endl;


	return 0;
}
