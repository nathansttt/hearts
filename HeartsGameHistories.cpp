/*
 *  HeartsGameHistories.cpp
 *  Hearts
 *
 *  Created by Nathan Sturtevant on 2/15/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "HeartsGameHistories.h"


SingleGameHistory::SingleGameHistory(HeartsGameState *g)
{
	strncpy(pl[0], g->getPlayer(0)->getName(), 1024);
	strncpy(pl[1], g->getPlayer(1)->getName(), 1024);
	strncpy(pl[2], g->getPlayer(2)->getName(), 1024);
	rules = g->getRules();
}

SingleGameHistory::SingleGameHistory(FILE *f)
{
	int count;
	fscanf(f, "%d\n", &count);
	fscanf(f, "%s %s %s %d\n", pl[0], pl[1], pl[2], &rules);
	//printf("Loading %d hands in game\n", count);
	for (int x = 0; x < count; x++)
	{
		SingleHandHistory shh(f);
		hands.push_back(shh);
	}
}

void SingleGameHistory::save(FILE *f)
{
	fprintf(f, "%d\n", hands.size());
	fprintf(f, "%s %s %s %d\n", pl[0], pl[1], pl[2], rules);
	for (unsigned int x = 0; x < hands.size(); x++)
		hands[x].save(f);
}

void SingleGameHistory::getString(std::string &str) const
{
	addNum(str, hands.size());
	str += "\n";
	str += pl[0];
	str += " ";
	str += pl[1];
	str += " ";
	str += pl[2];
	str += " ";
	addNum(str, rules);
	//str += rules;
	str += "\n";
//	printf("Current results:\n%s\n", str.c_str());
	for (unsigned int x = 0; x < hands.size(); x++)
		hands[x].getString(str);
}

void SingleGameHistory::AddHand(SingleHandHistory &h)
{
	hands.push_back(h);
}

unsigned int SingleGameHistory::GetNumHands() const
{
	return hands.size();
}

const SingleHandHistory &SingleGameHistory::GetHand(unsigned int which) const
{
	assert (which < hands.size());
	return hands[which];
}

int SingleGameHistory::GetTotalScore(int who, int limit) const
{
	int sum=0;
	for (unsigned int x = 0; (x<=limit) && (x < hands.size()); x++)
		sum+=hands[x].scores[who];
	return sum;
}

int SingleGameHistory::GetTotalScore(int who) const
{
	int sum=0;
	for (unsigned int x = 0; x < hands.size(); x++)
		sum+=hands[x].scores[who];
	return sum;
}

int SingleGameHistory::GetScore(int who, int game) const
{
	return hands[game].scores[who];
}

void SingleGameHistory::GetIntermediateScores(std::vector<int> &players, int hand, int trick) const
{
	players.resize(0);
	SingleHandHistory h = GetHand(hand);
	if (trick >= 13)
	{
		players.push_back(h.scores[0]);
		players.push_back(h.scores[1]);
		players.push_back(h.scores[2]);
		players.push_back(h.scores[3]);
		return;
	}
	players.resize(4);
	for (int x = 0; x < trick; x++)
	{
		int points = 0;
		for (int y = 0; y < 4; y++)
		{
			if (!(rules&kHeartsArentPoints))
				points += (Deck::getsuit(h.plays[x*4+y]) == HEARTS)?1:0;
			if ((rules&kQueenPenalty) && (h.plays[x*4+y] == Deck::getcard(SPADES, QUEEN)))
				points += 13;
			if ((rules&kJackBonus) && (h.plays[x*4+y] == Deck::getcard(DIAMONDS, JACK)))
				points -= 10;
		}
		for (int y = 0; y < 4; y++)
			if (h.cards[y].has(h.plays[(x+1)*4]))
				players[y] += points;
	}
}

int SingleGameHistory::GetRank(int who) const
{
	int rank = 1;
	for (int x = 0; x < 4; x++)
		if (GetTotalScore(x) < GetTotalScore(who)) rank++;
	return rank;
}

bool SingleGameHistory::IsWinner(int who) const
{
	for (int x = 0; x < 4; x++)
	if (GetTotalScore(x) < GetTotalScore(who)) return false;
	return true;
}



GameHistories::GameHistories()
:savePath(0), userName(0)
{
	newGameNotInitialized = false;
}

GameHistories::~GameHistories()
{
	if (savePath)
	{
		SaveHistory();
	}
}


void GameHistories::SaveHistory()
{
	if (savePath)
	{
		char path[strlen(savePath)+strlen(userName)+100];
		
		sprintf(path, "mkdir '%s/xinxin'", savePath);
		system(path);
		
		sprintf(path, "%s/xinxin/%s.play", savePath, userName);
		FILE *f = fopen(path, "w+");
		if (!f) return;
		fprintf(f, "%d\n", (int)games.size());
		for (unsigned int x = 0; x < games.size(); x++)
			games[x].save(f);
		fclose(f);
	}
}

void GameHistories::LoadHistory()
{
	if (savePath)
	{
		char path[strlen(savePath)+strlen(userName)+100];
		
		sprintf(path, "mkdir '%s/xinxin'", savePath);
		system(path);
		
		sprintf(path, "%s/xinxin/%s.play", savePath, userName);
		FILE *f = fopen(path, "r");
		if (!f) return;
		int count;
		fscanf(f, "%d\n", &count);
		printf("Loading %d games\n", count);
		for (unsigned int x = 0; x < count; x++)
		{
			SingleGameHistory h(f);
			games.push_back(h);
		}
		fclose(f);

//		char path[strlen(savePath)+100];
//		sprintf(path, "ls '%s/Sharts/*.play' > '%s/Sharts/index.txt'", savePath);
//		system(path);
	}
}

void GameHistories::StartNewGame()
{
	newGameNotInitialized = true;
}

void GameHistories::AddToLastGame(HeartsGameState *g)
{
	if (newGameNotInitialized)
	{
		games.push_back(SingleGameHistory(g));
		newGameNotInitialized = false;
	}
	SingleHandHistory h(g);
	games.back().AddHand(h);
	SaveHistory();
}

double GameHistories::GetAverageScore(int who)
{
	int sum=0;
	for (unsigned int x = 0; x < games.size(); x++)
		sum+=games[x].GetTotalScore(who);
	return (double)sum/games.size();
}

double GameHistories::GetAverageHandScore(int who)
{
	int sum=0;
	int hands = 0;
	for (unsigned int x = 0; x < games.size(); x++)
	{
		sum+=games[x].GetTotalScore(who);
		hands+=games[x].GetNumHands();
	}
	return (double)sum/hands;
}

double GameHistories::GetAverageRank(int who)
{
	int sum=0;
	for (unsigned int x = 0; x < games.size(); x++)
	{
		sum+=games[x].GetRank(who);
	}
	return (double)sum/games.size();
}

unsigned int GameHistories::GetNumGamesPlayed()
{
	return games.size();
}

const SingleGameHistory &GameHistories::GetGame(unsigned int which)
{
	assert(which < games.size());
	return games[which];
}

int GameHistories::GetNumGamesWon(int who)
{
	int sum=0; for (unsigned int x = 0; x < games.size(); x++)
		sum+=(games[x].IsWinner(who)?1:0);
	return sum;
}

void GameHistories::SetSavePath(const char *c)
{ 
	if (c == 0)
	{
		delete[] savePath;
		savePath = 0;
		return;
	}
	savePath = new char[strlen(c)+1];
	strcpy(savePath, c);
	if (userName && savePath)
	{
		LoadHistory();
		CreateUserData();
	}
}

void GameHistories::SetUserName(const char *c)
{ 
	if (c == 0)
	{
		delete[] userName;
		userName = 0;
		return;
	}
	userName = new char[strlen(c)+1];
	strcpy(userName, c);
	if (userName && savePath)
	{
		LoadHistory();
		CreateUserData();
	}
}

void GameHistories::CreateUserData()
{
	char path[strlen(savePath)+strlen(userName)+100];
	
	sprintf(path, "%s/xinxin/%s.upload", savePath, userName);
	FILE *f = fopen(path, "r");
	if (f)
	{
		fclose(f);
	}
	else {
		f = fopen(path, "w+");
		if (!f) return; // can't create for some reason -- should we throw error?
		
		// contains 2 things: (1) last game sent (2) random user id
		fprintf(f, "%d\n", -1);
		fclose(f);
	}

	sprintf(path, "%s/xinxin/%s.keyname", savePath, userName);

	f = fopen(path, "r");
	if (f) { fclose(f); return; }

	f = fopen(path, "w+");
	if (!f) return; // can't create for some reason -- should we throw error?

	//srandom(time(NULL));
	long vals[129];
	for (int x = 0; x < 129; x++)
		vals[x] = random();
	long val=random();
	for (int x = 0; x < strlen(userName); x++)
		val ^= vals[(userName[x]+17*x)%129];
	srandom(val);
	fprintf(f, "%X%X\n", random()^random(), random());
	fclose(f);
}

int GameHistories::GetLastSentGame()
{
	char path[strlen(savePath)+strlen(userName)+100];	
	sprintf(path, "%s/xinxin/%s.upload", savePath, userName);
	FILE *f = fopen(path, "r");
	if (!f) return -2;
	int last;
	fscanf(f, "%d\n", &last);
	fclose(f);
	return last;
}

void GameHistories::SetGameUploaded(int which)
{
	char path[strlen(savePath)+strlen(userName)+100];	
	sprintf(path, "%s/xinxin/%s.upload", savePath, userName);
	FILE *f = fopen(path, "w+");
	if (!f) return;
	fprintf(f, "%d\n", which); 
	fclose(f);
}

const char *GameHistories::GetUserRandomName()
{
	static char name[32];
	char path[strlen(savePath)+strlen(userName)+100];	
	sprintf(path, "%s/xinxin/%s.keyname", savePath, userName);
	FILE *f = fopen(path, "r");
	if (!f) 
	{
		name[0] = 0;
		return 0;
	}
	fscanf(f, "%s\n", name);
	fclose(f);
	return name;
}

const char *GameHistories::GetHistoryString(int whichGame)
{
	static std::string str;
	str.clear();
	GetGame(whichGame).getString(str);
//	printf("Data1\n-----\n");
//	printf(str.c_str());
//	printf("\n-----\n");
	
	return str.c_str();
}

SingleHandHistory::SingleHandHistory(FILE *f)
{
	//	int scores[4];
	int val;
	for (int x = 0; x < 4; x++)
		fscanf(f, "%d ", &scores[x]);
	
	// Deck cards[4];
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			fscanf(f, "%d ", &val);
			cards[x].setSuit(y, (uint16_t)val);
			if (cards[x].getSuit(y) != (uint16_t)val)
			{
				printf("Oh No!! Read failed! (%d, %d)\n", x, y);
				printf("Val: %d (0x%X) => %d (0x%X)\n", val, val,
					   cards[x].getSuit(y), cards[x].getSuit(y));
			}
		}
		
		//	std::vector<card> passes[4];
		for (int y = 0; y < 3; y++)
		{
			fscanf(f, "%d ", &val);
			passes[x].push_back(val);
		}
	}
	
	//	std::vector<card> plays;
	for (unsigned int x = 0; x < 52; x++)
	{
		fscanf(f, "%d ", &val);
		plays.push_back(val);
	}
}

SingleHandHistory::SingleHandHistory(HeartsGameState *g)
{
	assert(g->Done());
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < g->passes[x].size(); y++)
		{
			passes[x].push_back(g->passes[x][y]);
		}
		cards[x].setHand(&(g->original[x]));
		scores[x] = g->score(x);
	}
	for (int x = 0; x < 13; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			plays.push_back(g->getTrick(x)->play[y]);
		}
	}
}

void SingleHandHistory::save(FILE *f)
{
//	
//	int scores[4];
	for (int x = 0; x < 4; x++)
		fprintf(f, "%d ", scores[x]);
	fprintf(f, "\n");

	// Deck cards[4];
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			fprintf(f, "%d ", cards[x].getSuit(y));
		}
		fprintf(f, "\n");

		//	std::vector<card> passes[4];
		for (int y = 0; y < 3; y++)
		{
			if (passes[x].size() > 0)
				fprintf(f, "%d ", passes[x][y]);
			else
				fprintf(f, "%d ", -1);
		}
		fprintf(f, "\n");
	}

	//	std::vector<card> plays;
	for (unsigned int x = 0; x < plays.size(); x++)
	{
		fprintf(f, "%d ", plays[x]);
	}
	fprintf(f, "\n");
}

void SingleHandHistory::getString(std::string &str) const
{
	const char *mone = "-1 ";
	for (int x = 0; x < 4; x++)
	{
		addNum(str, scores[x]);
		str += " ";
	}
	str += "\n";
	
	// Deck cards[4];
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			addNum(str, cards[x].getSuit(y));
			str += " ";
			//fprintf(f, "%d ", cards[x].getSuit(y));
		}
		str += "\n";
		
		//	std::vector<card> passes[4];
		for (int y = 0; y < 3; y++)
		{
			if (passes[x].size() > 0)
			{
				addNum(str, passes[x][y]);
				str += " ";
			}
			else {
				str += mone;
			}
		}
		str += "\n";
	}
	
	//	std::vector<card> plays;
	for (unsigned int x = 0; x < plays.size(); x++)
	{
		addNum(str, plays[x]);
		str += " ";
	}
	str += "\n";
//	printf("Current results:\n%s\n", str.c_str());
}

void addNum(std::string &str, int num, bool first)
{
	if (num < 0)
	{
		str += "-";
		addNum(str, -num, false);
		return;
	}
	if (num == 0)
	{
		if (first)
			str += "0";
		return;
	}
	addNum(str, num/10, false);
	str += ('0'+(num%10));
}
