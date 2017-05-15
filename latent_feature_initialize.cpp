#include "DS.h"
#include <sys/time.h>
#include <algorithm>
#include <iterator>
#include <cstring>
#include "user.h"
#include "item.h"
#include <stdio.h>

extern map<size_u, User> users;
extern map< string, map<size_u, Item> > multi_items;
extern Id_Index id_Index;

// [-0.1, 0.1]
float rands() {
	return (rand() - RAND_MAX / 2.0) / RAND_MAX / 5;
}

void latent_feature_initialize() {
	//srand((unsigned) time(0));
	srand(0);
	cout << "\ninit begin..." << endl;
	//struct timeval start, end;
	//gettimeofday(&start, NULL);
	const int USER_NUM = id_Index.id_index[Channels().channel_map[0]].size();
	const int ITEM_NUM = id_Index.id_index[Channels().channel_map[1]].size();
	long rand_num = (USER_NUM + ITEM_NUM) * dim;
	vector<float> V(rand_num);

	generate_n(V.begin(), rand_num, rands);

	long current_pos = 0;
	for (int i = 0; i < USER_NUM; i++) {
		size_u user_id = id_Index.id_index[Channels().channel_map[0]][i];
		copy(V.begin() + current_pos, V.begin() + current_pos + dim, users[user_id].user_latent_feature.begin());
		copy(V.begin() + current_pos, V.begin() + current_pos + dim, users[user_id ].user_latent_feature_previous.begin());
		current_pos += dim;
	}

	for (int i = 0; i < ITEM_NUM; i++) {
        size_u item_id = id_Index.id_index[Channels().channel_map[1]][i];
		copy(V.begin() + current_pos, V.begin() + current_pos + dim, multi_items[Channels().channel_map[1]][item_id].item_latent_feature.begin());
		copy(V.begin() + current_pos, V.begin() + current_pos + dim, multi_items[Channels().channel_map[1]][item_id].item_latent_feature_previous.begin());
		current_pos += dim;
    }

	//gettimeofday(&end,NULL);
	//cout << "time elapse: " << end.tv_sec - start.tv_sec << endl;

	cout << "init done" << endl;
}
