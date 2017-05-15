#ifndef USER_H
#define USER_H

#include <vector>
#include <map>
#include <utility>
#include <list>
#include <string>
#include "paras.h"

using namespace std;

class User{
    public:
        map< string, vector<size_u> > ratings;	// multi-channel rated items, including user's voting, user's group
        map< string, vector<size_u> > testset;	// multi-channel test data
        vector<float> user_latent_feature;
        vector<float> user_latent_feature_previous;

        // no use
        map<size_u, float> utag_weight;	        //key: tag_id, val: tag weight(TF-IDF weight)
        map< string, list< pair <size_u, float> > > sim_list; // map key: channel, map val: list of similar users, pair.first: item_id, pair_second: similarity value

        User() {
            user_latent_feature.resize(dim);
            user_latent_feature_previous.resize(dim);
        }
};

#endif
