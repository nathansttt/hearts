#include <stdio.h>
//#include <sys/resource.h>
#include <time.h>
#include <math.h>
#include "GameState.h"
#include "Player.h"

namespace hearts {

//#define min(x, y) (((x)>(y))?(y):(x))
//#define max(x, y) (((x)<(y))?(y):(x))
//#define VERIFYHASH 0
//#define USEHASH 1
//#define RANDOMIZEMOVE
//#define RANDOMIZEMAXN
//Move *lastbest;
//mt_random Move::rand;

GameState::GameState()
{
	Reset();
	for (unsigned int x = 0; x < MAXPLAYERS; x++)
	{ men[x] = 0; teams[x] = -1; }
	moveList = 0;
	game = 0;
	//r.srand(time(0));
}

GameState::~GameState()
{
//	Move *tmp = moveList;
	delete moveList;
	for (unsigned int x = 0; x < MAXPLAYERS; x++)
	{ delete men[x]; men[x] = 0; }
}

void GameState::randomlySwitchPlayers()
{
	for (int x = 0; x < (int)numPlayers; x++)
	{
		int swap = r.rand_long()%(numPlayers-x)+x;
		Player *p = men[swap];
		men[swap] = men[x];
		men[x] = p;
	}
}

Move *GameState::getRandomMove()
{
	double cnt = 1;
	Move *m = this->getMoves();
//	printf("%d random moves available\n", m->length());
	Move *ret = m;
	m = m->next;
	ret->next = 0;
	while (m)
	{
		cnt++;
		double ran = r.rand_double();
//		printf("Comparing %1.2f to %1.2f\n", ran, 1.0/cnt);
		if (ran < 1.0/cnt)
		{
			freeMove(ret);
			ret = m;
			m = m->next;
			ret->next = 0;
		}
		else {
			Move *tmp = m;
			m = m->next;
			tmp->next = 0;
			freeMove(tmp);
		}
	}
	return ret;
}

bool GameState::IsLegalMove(Move *m)
{
	bool result = false;
	Move *n = getAllMoves();
	for (Move *t = n; t; t = t->next)
		if (m->equals(t))
			result = true;
	freeMove(n);
	return result;
}

Move *GameState::getNewMove()
{
	if (moveList == 0)
	{
		moveList = allocateMoreMoves(100);
		//printf("100 moves allocated\n");
	}
	Move *ret = moveList;
	moveList = moveList->next;
	ret->next = 0;
	return ret;
}

void GameState::freeMove(Move *m)
{
	if (m == 0)
		return;
	while (m->next)
	{
		Move *tmp = m->next;
		m->next = moveList;
		moveList = m;
		m = tmp;
	}
	m->next = moveList;
	moveList = m;
}

void GameState::copyMoveList(GameState *g)
{
//	static int cnt = 0;
	if (moveList == 0)
	{
		moveList = g->moveList;
		g->moveList = 0;
	}
	else {
		//printf("opps Movelist: %d\n", moveList->length());
	}
//	if (moveList && (((++cnt)%5000) == 0))
//		printf("Movelist: %d\n", moveList->length());
}


void GameState::addPlayer(Player *p)
{
	gameScore[numPlayers] = 0.0;
	men[numPlayers] = p;
	teams[numPlayers] = numPlayers;
	p->setGameState(this);
	numPlayers++;
}

void GameState::deletePlayers()
{
	for (unsigned int x = 0; x < numPlayers; x++)
	{
		delete men[x];
		men[x] = 0;
	}
	numPlayers = 0;
}

void GameState::setTeam(Player *p, int team)
{
	setTeam(getPlayerNum(p), team);
}

void GameState::setTeam(int who, int team)
{
	teams[who] = team;
}

int GameState::getTeam(int who)
{
	return teams[who];
}

void GameState::Reset(int NEWSEED)
{
	numPlayers = 0;
}

int GameState::getPlayerNum(Player *p) const
{
	for (unsigned int x = 0; x < numPlayers; x++)
		if (men[x] == p)
			return x;
	return -1;
}

Player *GameState::getPlayer(int which) const
{
	if (which < (int)numPlayers)
		return men[which];
	return 0;
}
/*
void GameState::Print(int val) const
{ printf("Can't call virtual GS::Print\n"); exit(1); }

Player *GameState::getNextPlayer()
{ printf("Can't call virtual GS::getNPlayer\n"); exit(1); return 0; }

int GameState::getNextPlayerNum()
{ printf("Can't call virtual GS::getNPlayerNum\n"); exit(1); return 0; }

int GameState::getCurrPlayerNum()
{ printf("Can't call virtual GS::getCurrPlayerNum\n"); exit(1); return 0; }

void GameState::ApplyMove(Move *move)
{ movesSoFar++; }

void GameState::UndoMove(Move *move)
{ movesSoFar--; }
*/
bool GameState::Done() const
{ printf("Can't call virtual GS::Done\n"); exit(1); return 0; }

} // namespace hearts
