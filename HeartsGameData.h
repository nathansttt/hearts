/*
 *  HeartsGameData.h
 *  Hearts
 *
 *  Created by Nathan Sturtevant on 11/27/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Hearts.h"
#include <string>

#ifndef HEARTSGAMEDATA_H
#define HEARTSGAMEDATA_H

namespace hearts {

enum {
	kFileError = -2,
	kNoData = -1
};


class HeartsGameData {
public:
	HeartsGameData():g(0), waiting(false)
	{
		gameOver = false;
		waitClick = false;
		passPhase = false;
		//		passDir = kLeft;
	}
	~HeartsGameData()
	{
		while (moves.size() > 0)
		{
			delete moves.back();
			moves.pop_back();
		}
	}
	int GetScore(int who);
	int GetLastScore(int who);
	HeartsGameState *g;
	bool waiting;
	bool passPhase;
	bool gameOver;
	int firstPlayerOffset;
	//tPassDir passDir;
	std::vector<card> passes;
	float waitClick;
	
	// change to be vector for history and keep the sum too
	std::vector<int> scores[4];
	std::vector<CardMove *> moves;
};

card GetCardPlay(int which, HeartsGameState *g);
void prepareToTransmit(std::string &str);

} // namespace hearts

#endif
