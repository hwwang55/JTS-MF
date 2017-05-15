#include "DS.h"
#include "user.h"
#include "item.h"
#include <stdlib.h>
#include <math.h>

extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern Id_Index id_Index;


bool converge() {
	float max = 0;
	// cout << "converge test ..." << endl;

	const int USER_NUM = id_Index.id_index["user"].size();
	#pragma omp parallel for schedule(static, 1000)
	for (int u = 0; u < USER_NUM; u++) {
		size_u user_id = id_Index.id_index["user"][u];
		for (int j = 0; j < dim; j++) {
			float tmp = fabs(users[user_id].user_latent_feature_previous[j] - users[user_id].user_latent_feature[j]);
			if (tmp > max) {
				#pragma omp ctitical
				max = tmp;
			}
		}
	}

	const int ITEM_NUM = id_Index.id_index["voting"].size();
	#pragma omp parallel for schedule(static, 1000)
	for (int i = 0; i < ITEM_NUM; i++) {
		size_u item_id = id_Index.id_index["voting"][i];
		for (int j = 0; j < dim; j++) {
			float tmp = fabs(multi_items["voting"][item_id].item_latent_feature[j] - multi_items["voting"][item_id].item_latent_feature_previous[j]);
			if (tmp > max) {
				#pragma omp critical
				max = tmp;
			}
		}
	}

	// cout << "max: " << max << endl << endl;
	cout << max << "\t";
	return epsino > max;
}
