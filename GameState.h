#ifndef _GAMESTATE_H
#define _GAMESTATE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "mt_random.h"
#include "ProblemState.h"
#include "States.h"

namespace hearts {

const double INF  =  999999999999999.9;
const double NINF = -999999999999999.9;
const unsigned int uINF = 0xFFFFFFFF;
const unsigned int MAXPLAYERS = 6;

class iiGameState;
class Game;
/*
 * Class GameState describes a particular game. It defines the next player,
 * the current player, and the state of the game.
 *
 * A game state should be able to provide an optimized game state for
 * a hash table.
 *
 * If the game supports partition search, getPartition should return a valid
 * partition.
 *
 */

class GameState : public ProblemState {
 public:
	GameState();
	virtual ~GameState();
	void randomlySwitchPlayers();
	void addPlayer(Player *p);
	void deletePlayers();
	void setTeam(Player *p, int team);
	void setTeam(int who, int team);
	int getTeam(int who);
	void setGame(Game *g)
	{ game = g; }
	Game *getGame()
	{ return game; }
	
	virtual void Reset(int NEWSEED = -1);
	virtual void Print(int val = 0) const = 0;
	virtual Player *getNextPlayer() const = 0;
	virtual int getNextPlayerNum() const = 0;
	virtual Player *getPreviousPlayer() const = 0;
	virtual int getPreviousPlayerNum() const = 0;
	virtual void ApplyMove(Move *move) {}
	virtual void UndoMove(Move *move) {}
	virtual bool IsLegalMove(Move *m);
	virtual Move *getMoves() = 0; // get moves for next player
	virtual Move *getAllMoves() { return getMoves(); } // get moves for next player
	virtual Move *getRandomMove();
	Move *getNewMove();
	void freeMove(Move *);
	
	virtual bool Done() const;
	virtual int Winner() const { return -1; }
	virtual double score(int who) const { return -1; } // this is the *actual* game score

	// return score if we can calculate the end score from here
	//virtual returnValue *checkEndGame() { return 0; }

	// redefine to use hash table
	// the argument, if provided, indicates to use that memory instead of allocating new
	// memory. The indicated memory will be the result of a previous call to getHashState
	virtual HashState *getHashState(HashState *mem = 0) { return 0; }
	// define to use history heuristic
	virtual uint32_t getMoveHash(const Move *m) { return 0; }
	virtual Move *getMoveFromHash(uint32_t hash) { return 0; }
	
	virtual uint64_t HashCurrentState(int who) const { return 0; }
	// get partition representations of the current state
	// virtual partition *getPartition(int me, double value) { return 0; }
	// virtual HashState *getPartitionHashState(GameState *g, int me) { return 0; }

	Player *getPlayer(int which) const;
	int getPlayerNum(Player *) const;

	inline unsigned int getNumPlayers() const { return numPlayers; }
//	unsigned int getDepth() { return movesSoFar; } // our current depth in the tree
//	unsigned int getTurns() { return movesSoFar/numPlayers; }
	virtual bool isImperfectInformation() { return false; }
	virtual iiGameState *getiiGameState(bool consistent, int who, Player *playerModel=0) { return 0; }
	double gameScore[MAXPLAYERS];
	mt_random r;
	void copyMoveList(GameState *g);
protected:
	virtual Move *allocateMoreMoves(int n) = 0;
	Player *men[MAXPLAYERS];
	int teams[MAXPLAYERS];
	unsigned int numPlayers;
 private:
	// this is a cache of allocated moves
	Move *moveList;
	Game *game;
};

} // namespace hearts

#endif
