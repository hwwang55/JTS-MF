#ifndef ITEM_H
#define ITEM_H

#include <vector>
#include <string>
#include "paras.h"

using namespace std;

// item can be group or voting.
class Item{
    public:
        vector<size_u> user_list;	// store users who rated this item, if it is a group, mean users who belong to this group.
        vector<float> item_latent_feature;
        vector<float> item_latent_feature_previous;

        Item() {
            item_latent_feature.resize(dim);
            item_latent_feature_previous.resize(dim);
        }
};

#endif
