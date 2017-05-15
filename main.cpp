#include "DS.h"
#include "user.h"
#include "item.h"
#include <fstream>
#include <string>
#include <sstream>

void load_data(string localhost, string username, string password, string table);
void latent_feature_initialize();
void item_update();
void user_update();
bool converge();
int top_n_test(string channel);

map<size_u, User> users;
map< string, map<size_u, Item> > multi_items;

/* added by Hongwei Wang at 2016/07/20 19:26 */
map< pair<string, string>, map< size_u, map<size_u,float> > > multi_matching_score;
map< size_u, map<size_u, float> > user_user_group_level_matching_score;

float step_size = 0.001;


int main() {
	load_data("10.88.88.241", "root", "123456", "weibo_bkp");

	latent_feature_initialize();

    int iteration = 1;
	bool flag;
	do {
		cout << "iteration: " << iteration << endl;
		item_update();
		user_update();

		// output info every 10 iterations
		if (iteration % 10 == 0)  top_n_test("voting");
		iteration++;
	} while(iteration <= 200);
}
