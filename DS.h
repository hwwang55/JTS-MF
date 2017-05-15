#ifndef DS_H_
#define DS_H_

#include <vector>
#include <iostream>
#include <map>
#include <set>
#include <cassert>
#include <omp.h>
#include <limits.h>
#include <string>
#include "paras.h"
#include <string>
using namespace std;

/*
In this model, we learning user interest from multiple channels, e.g., user-voting, user-group, user-tag, user-user etc.
Thus, there are multiple items in this model.
*/


class Pair_Channels {
    public:
        map< int, pair<string, string> > pair_channel_map;

        Pair_Channels() {
            this->pair_channel_map[0] = make_pair("user", "voting");
            this->pair_channel_map[1] = make_pair("user", "user");
            this->pair_channel_map[2] = make_pair("user", "group");
            this->pair_channel_map[3] = make_pair("voting", "voting");
        }
};


class Channels{
    public:
        map<int, string> channel_map;

        Channels() {
            this->channel_map[0] = "user";
            this->channel_map[1] = "voting";
            this->channel_map[2] = "group";
            this->channel_map[3] = "tag";
        }
};


class Id_Index {
    public:
        map<string, vector<size_u> > id_index;  // key: channel, val: Ids
};


class RM {
    public:
        map<string,double> rm;

        RM() {
            Channels channels;
            this->rm[channels.channel_map[0]] = r_m_0;
            this->rm[channels.channel_map[1]] = r_m_1;
            this->rm[channels.channel_map[2]] = r_m_2;
            this->rm[channels.channel_map[3]] = r_m_3;
        }
};


class Weights {
    public:
        map<string, double> weights;

        Weights() {
            Channels channels;
            this->weights[channels.channel_map[0]] = w_m_0;
            this->weights[channels.channel_map[1]] = w_m_1;
            this->weights[channels.channel_map[2]] = w_m_2;
            this->weights[channels.channel_map[3]] = w_m_3;
        }
};

/*
weights for observed rating in given channel i: GAMMA_i * w
weights for missing rating in given channel: GAMMA_i * w_m_i
*/

class TopN {
    public:
        vector<int> top_n;

        TopN() {
            this->top_n.push_back(1);
            this->top_n.push_back(2);
            this->top_n.push_back(5);
            this->top_n.push_back(10);
            this->top_n.push_back(20);
            this->top_n.push_back(50);
            this->top_n.push_back(100);
            this->top_n.push_back(500);
            this->top_n.push_back(1000);
            this->top_n.push_back(5000);
            this->top_n.push_back(10000);
        }
};

class Total_liked {
    public:
        map<int, int> total_liked;	//key: top n, val: total svd

	Total_liked() {
		TopN topN;
		vector<int>::iterator VIt;
		for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
            this->total_liked[*VIt] = 0;
        }
	}
};

class Hit_liked{
    public:
        map<int, int> hit_liked;	//key: top n, val: total svd

	Hit_liked() {
		TopN topN;
		vector<int>::iterator VIt;
		for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
            this->hit_liked[*VIt] = 0;
        }
	}
};

class Recall{
    public:
        map<int,double> recall_total;

	Recall() {
		TopN topN;
		vector<int>::iterator VIt;
		for(VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
            this->recall_total[*VIt]= 0;
        }
	}
};


/* added by Hongwei Wang at 2016/09/25 16:56 */
class Precision{
public:
	map<int, double> precision_total;

	Precision() {
		TopN topN;
		vector<int>::iterator VIt;
		for (VIt = topN.top_n.begin(); VIt != topN.top_n.end(); VIt++) {
			this->precision_total[*VIt] = 0;
		}
	}
};

#endif
