#include <stdio.h>
#include <time.h>
#include <math.h>
#include "ProblemState.h"
#include "Player.h"

//#define min(x, y) (((x)>(y))?(y):(x))
//#define max(x, y) (((x)<(y))?(y):(x))
#define VERIFYHASH 0
#define USEHASH 1
//#define RANDOMIZEMOVE
//#define RANDOMIZEMAXN
//Move *lastbest;
//mt_random Move::rand;

ProblemState::ProblemState()
{
	searchAgent = 0;
}

void ProblemState::addPlayer(Player *p)
{
	delete searchAgent;
	searchAgent = p;
}

ProblemState::~ProblemState()
{
}

void ProblemState::deletePlayers()
{
	delete searchAgent;
}

void ProblemState::Reset(int NEWSEED)
{
}

int ProblemState::getPlayerNum(Player *p) const
{
	return 0;
}

Player *ProblemState::getPlayer(int which) const
{
	return searchAgent;
}

void ProblemState::Print(int val) const
{ printf("Can't call virtual GS::Print\n"); exit(1); }

bool ProblemState::Done() const
{ printf("Can't call virtual PS::Done\n"); exit(1); return 0; }

