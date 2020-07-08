#include <string.h>
#include "statistics.h"
#include "Player.h"
#include "fpUtil.h"
#include <sstream>

namespace hearts {
	
template <typename T>
std::string ToString(T aValue)
{
	std::stringstream ss;
	ss << aValue;
	return ss.str();
}

void statistics::collect(Game *g, GameState *gs, bool show)
{
	int num = gs->getNumPlayers();

	for (int x = 0; x < num; x++)
	{
		int gtype = -1;
		int ptype = -1;
		Player *p = gs->getPlayer(x);
		for (unsigned int y = 0; y < data.size(); y++)
			if ((strcmp(p->getName(), data[y].algorithms.c_str()) == 0) && (data[y].type == kPlayerStat) &&
				(data[y].player == x))
				ptype = y;
		for (unsigned int y = 0; y < data.size(); y++)
			if ((strcmp(p->getName(), data[y].algorithms.c_str()) == 0) && (data[y].type == kAlgorithmStat))//(data[y].player == -1)
				gtype = y;
		if (gtype == -1)
		{
			gtype = data.size();
			data.resize(gtype+1);
			//count++;
			data[gtype].algorithms = p->getName();
			data[gtype].type = kAlgorithmStat;
			data[gtype].player = -1;
			data[gtype].wins=0;
			data[gtype].plays=0;
			data[gtype].score=0;
			data[gtype].rank=0;
		}
		if (ptype == -1)
		{
			ptype = data.size();
			data.resize(ptype+1);
			//count++;
			data[ptype].algorithms = p->getName();
			data[ptype].type = kPlayerStat;
			data[ptype].player = x;
			data[ptype].wins=0;
			data[ptype].plays=0;
			data[ptype].score=0;
			data[ptype].rank=0;
		}
		if (g->PlayerWon(x))
			data[ptype].wins++;
		data[ptype].score+=(int)g->score(x);
		data[ptype].scores.push_back((int)g->score(x));
		data[ptype].plays++;
		data[ptype].rank+=g->Rank(x);
		data[ptype].ranks.push_back((int)g->Rank(x));

		if (g->PlayerWon(x))
			data[gtype].wins++;
		data[gtype].score+=(int)g->score(x);
		data[gtype].scores.push_back((int)g->score(x));
		data[gtype].plays++;
		data[gtype].rank+=g->Rank(x);
		data[gtype].ranks.push_back((int)g->Rank(x));
		
		if (show)
			printf("Player %d, (%s), got %d.\n", x, data[ptype].algorithms.c_str(), (int)g->score(x));
	}
	sort();
}

void statistics::save(const char *fname)
{
	FILE *f = fopen(fname, "a+");
	if (f) {
		fprintf(f, "plays\twins\trank\t(avg)\t(std)\tscore\t(avg)\t(std)\talgorithm\n");
		for (unsigned int x = 0; x < data.size(); x++)
		{
			switch (data[x].type)
			{
				case kAlgorithmStat:
					fprintf(f, "%d\t%d\t%d\t%1.4f\t%1.4f\t%d\t%1.4f\t%1.4f\t%s\n",
							data[x].plays, data[x].wins,
							data[x].rank, (double)data[x].rank/data[x].plays, getStdDev(data[x].ranks),
							data[x].score, (double)data[x].score/data[x].plays, getStdDev(data[x].scores),
							data[x].algorithms.c_str());
					break;
				case kPlayerStat:
					fprintf(f, "%d\t%d\t%d\t%1.4f\t%1.4f\t%d\t%1.4f\t%1.4f\t%s (%d)\n",
							data[x].plays, data[x].wins,
							data[x].rank, (double)data[x].rank/data[x].plays, getStdDev(data[x].ranks),
							data[x].score, (double)data[x].score/data[x].plays,  getStdDev(data[x].scores),
							data[x].algorithms.c_str(), data[x].player);
					break;
				case kCheckpointStat:
					fprintf(f, "%d\t-\t%d\t%5.2f\t%5.2f\t%d\t%5.2f\t%5.2f\t%s\n",
							data[x].plays,
							data[x].rank, (double)data[x].rank/data[x].plays, getStdDev(data[x].ranks),
							data[x].score, (double)data[x].score/data[x].plays,  getStdDev(data[x].scores),
							data[x].algorithms.c_str());
					break;
			}
		}
		fprintf(f, "\n");
		showSignificance(f);
		fclose(f);
	}
//	f = fopen("results.dat", "w+");
//	if (f)
//	{
//		fprintf(f, "%d\n", (int)data.size());
//		for (unsigned int x = 0; x < data.size(); x++)
//			data[x].save(f);
//		fclose(f);
//	}
}

void statistics::sort()
{
	if (checkpointed)
		return;
	for (unsigned int x = 0; x < data.size(); x++)
	{
		for (unsigned int y = x+1; y < data.size(); y++)
		{
			if ((data[x].algorithms > data[y].algorithms) ||
				((data[x].algorithms == data[y].algorithms) &&
				 (data[x].player > data[y].player)))
			{
				playData temp = data[x];
				data[x] = data[y];
				data[y] = temp;
			}
		}
	}
}

double statistics::getStdDev(std::vector<int> &vec)
{
	double n = 0;
	double mean = 0;
	double S = 0;
	
	for (unsigned int x = 0; x < vec.size(); x++)
	{
		n++;
		double delta = (double)vec[x] - mean;
		mean = mean + delta/n;
		S = S + delta*(vec[x] - mean);      // This expression uses the new value of mean
	}
	double variance = S/(n - 1);
	return sqrt(variance);
}

void statistics::checkpoint()
{
	checkpointed = true;
	// find every unsummarized statistic, and checkpoint it
	for (unsigned int y = 0; y < data.size(); y++)
	{
		int which = -1;
		if (data[y].type == kCheckpointStat)
			continue;
		for (unsigned int x = 0; x < data.size(); x++)
		{
			if ((data[x].type == kCheckpointStat) &&
				(data[x].player == (int)y))
			{
				which = x;
				break;
			}
		}
		if (which == -1)
		{
			which = data.size();
			data.resize(which+1);
			if (data[y].type == kPlayerStat)
			{
				data[which].algorithms = "[c] " + data[y].algorithms + " (" + ToString(data[y].player) + ")";
			}
			else
				data[which].algorithms = "[c] " + data[y].algorithms;
			data[which].type = kCheckpointStat;
			data[which].player = y;
			data[which].wins=0;
			data[which].plays=0;
			data[which].score=0;
			data[which].rank=0;
		}

		data[which].scores.push_back(data[data[which].player].score-data[which].score);
		data[which].score += data[which].scores.back();

		data[which].ranks.push_back(data[data[which].player].rank-data[which].rank);
		data[which].rank = data[which].ranks.back();

		data[which].plays++;

//		data[ptype].score+=(int)g->score(x);
//		data[ptype].scores.push_back((int)g->score(x));
//		data[ptype].plays++;
//		data[ptype].rank+=g->Rank(x);
//		data[ptype].ranks.push_back((int)g->Rank(x));		
	}
}

// s1 = standard deviation
// s1^2 = variance
// SE = sqrt ( s1^2/n1 + s2^2/n2 )
// 99% confidence = (x1-x2) +/- 2.58*SE
// 95% confidence = (x1-x2) +/- 1.96*SE
// if 0 is included in range, then they "are the same"
void statistics::showSignificance(FILE *f)
{
	std::vector<unsigned int> algs;
	for (unsigned int x = 0; x < data.size(); x++)
	{
		if (data[x].type == kAlgorithmStat)
			algs.push_back(x);
	}
	printf("Found %d algorithms\n", (int)algs.size());
	for (unsigned int x = 0; x < data.size(); x++)
	{
		if (data[x].plays <= 2)
			continue;
		for (unsigned int y = x+1; y < data.size(); y++)
		{
			if (data[y].plays <= 2)
				continue;
			for (unsigned int t = 0; t < algs.size(); t++)
			{
				for (unsigned int u = t+1; u < algs.size(); u++)
				{
					if (((algs[t] == x) && (algs[u] == y)) ||
						((algs[t] == y) && (algs[u] == x)) ||
						((data[x].type == kCheckpointStat) && (data[y].type == kCheckpointStat) &&
						 (((data[x].player == (int)algs[t]) && (data[y].player == (int)algs[u])) ||
						  ((data[x].player == (int)algs[u]) && (data[y].player == (int)algs[t])))))
					{
						double tmp1 = getStdDev(data[x].scores);
						double tmp2 = getStdDev(data[y].scores);
						double SE = sqrt(tmp1*tmp1/data[x].plays+
										 tmp2*tmp2/data[y].plays);
						double meandiff = fabs((double)data[x].score/data[x].plays-
											   (double)data[y].score/data[y].plays);
						fprintf(f, "%s vs %s: diff(%f)", data[x].algorithms.c_str(), data[y].algorithms.c_str(), meandiff);
						if (fgreater(meandiff-1.96*SE, 0))
							fprintf(f, " 95%%");
						if (fgreater(meandiff-2.58*SE, 0))
							fprintf(f, " 99%%");
						fprintf(f, "\n");
//						fprintf(f, "Standard error for %s (%f^2/%d) and %s (%f^2/%d) is %f; mean diff is %f\n",
//								data[x].algorithms, getStdDev(data[x].scores), data[x].plays,
//								data[y].algorithms, getStdDev(data[y].scores), data[y].plays,
//								SE, meandiff);
//						fprintf(f, "They %s different with 99%% confidence\n",
//								fless(meandiff-2.58*SE, 0)?"are not":"are");
//						fprintf(f, "They %s different with 95%% confidence\n",
//								fless(meandiff-1.96*SE, 0)?"are not":"are");
					}
				}
			}
		}
	}
}

}
