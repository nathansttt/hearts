/*
 *  HeartsGameHistories.h
 *  Hearts
 *
 *  Created by Nathan Sturtevant on 2/15/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Hearts.h"
#include "fpUtil.h"
#include <string>

#ifndef HEARTSGAMEHISTORIES_H
#define HEARTSGAMEHISTORIES_H

class SingleHandHistory {
public:
	SingleHandHistory(FILE *f);
	SingleHandHistory(HeartsGameState *g);
	void save(FILE *f);
	void getString(std::string &str) const;
	Deck cards[4];
	int scores[4];
	std::vector<card> passes[4];
	std::vector<card> plays;
};

class SingleGameHistory {
public:
	SingleGameHistory(HeartsGameState *g);
	SingleGameHistory(FILE *f);
	void save(FILE *f);
	const char *GetPlayerName(int which) const { return pl[which]; }
	void getString(std::string &str) const;
	void AddHand(SingleHandHistory &h);
	unsigned int GetNumHands() const;
	const SingleHandHistory &GetHand(unsigned int which) const;
	int GetTotalScore(int who, int limit) const;
	int GetTotalScore(int who) const;
	int GetScore(int who, int game) const;
	void GetIntermediateScores(std::vector<int> &players, int hand, int trick) const;
	int GetRank(int who) const;
	int GetRules() const { return rules; }
	bool IsWinner(int who) const;
private:
	char pl[3][1024];
	int rules;
	std::vector<SingleHandHistory> hands;
};

class GameHistories {
public:
	GameHistories();
	~GameHistories();
	void SaveHistory();
	void LoadHistory();
	const char *GetHistoryString(int whichGame);
	
	void StartNewGame();
	void AddToLastGame(HeartsGameState *g);
	double GetAverageScore(int who);
	double GetAverageHandScore(int who);
	double GetAverageRank(int who);
	unsigned int GetNumGamesPlayed();
	const SingleGameHistory &GetGame(unsigned int which);
	int GetNumGamesWon(int who);
	void SetSavePath(const char *c);
	void SetUserName(const char *c);
	const char *GetUserRandomName();
	int GetLastSentGame();
	void SetGameUploaded(int which);
private:
	std::vector<SingleGameHistory> games;
	char *savePath, *userName;
	bool newGameNotInitialized;

	void CreateUserData();
};

void addNum(std::string &str, int num, bool first = true);

#endif
