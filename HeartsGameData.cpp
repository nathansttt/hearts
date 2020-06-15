/*
 *  HeartsGameData.cpp
 *  Hearts
 *
 *  Created by Nathan Sturtevant on 11/27/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "HeartsGameData.h"

card GetCardPlay(int which, HeartsGameState *g)
{
	for (int x = 0; x < 64; x++)
	{
		if (g->cards[3].has(x))
		{
			if (which == 0)
				return x;
			which--;
		}
	}
	return -1;
}

int HeartsGameData::GetScore(int who)
{
	if (scores[who].size() <= 0)
		return 0;
	int sum = 0;
	for (unsigned int x = 0; x < scores[who].size(); x++)
		sum+=scores[who][x];
	return sum;
}

int HeartsGameData::GetLastScore(int who)
{
	if (scores[who].size() <= 0)
		return 0;
	//	if (scores[who].size() == 1)
	return scores[who].back();
	//	return scores[who][scores[who].size()-2];
}

//void prepareToTransmit(std::string &str)
//{
////	std::iterator<std::string> i;
//	while (str.find(std::string(" ")) != str.end())
//	{
////		i = str.find(' ');
////		*i = '0';
//		
//		str.insert(str.insert(str.find(' '), '%'), '2');
//		//iterator insert(iterator pos, const T& x);
//	}
//}