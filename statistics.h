#include "GameState.h"
#include "Game.h"
#include <vector>
#include <string>

#ifndef STATISTICS_CPP
#define STATISTICS_CPP

namespace hearts {
	
enum tStatType {
	kPlayerStat = 1,
	kAlgorithmStat = 2,
	kCheckpointStat = 3
};

class playData {
public:
	std::string algorithms;
	tStatType type;
	int player;
	int wins;
	int plays;
	int score;
	int rank;
	std::vector<int> scores;
	std::vector<int> ranks;
	void save(FILE *f)
	{
		fprintf(f, "%s %d %d %d %d %d %d\n", algorithms.c_str(), (int)type, player, wins, plays, score, rank);
		fprintf(f, "%d %d\n", (int)scores.size(), (int)ranks.size());
		for (unsigned int x = 0; x < scores.size(); x++)
			fprintf(f, "%d ", scores[x]);
		fprintf(f, "\n");
		for (unsigned int x = 0; x < ranks.size(); x++)
			fprintf(f, "%d ", ranks[x]);
		fprintf(f, "\n");
	}
};

class statistics {
public:
	statistics() { checkpointed = false; }
	~statistics() { save(); }
	void collect(Game *g, GameState *gs, bool show=true);
	void checkpoint();
	void save(const char *file = "results.txt");
	void reset() { data.resize(0); }
private:
	void sort();
	double getStdDev(std::vector<int> &vec);
	void showSignificance(FILE *f);	
	std::vector<playData> data;
	bool checkpointed;
};

}
	
#endif
