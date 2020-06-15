#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
//#include "random.h"
#include "States.h"

#ifndef _PROBLEMSTATE_H
#define _PROBLEMSTATE_H

/*
 * Class Problemstate describes a particular game. It defines the next player,
 * the current player, and the state of the game.
 *
 * A game state should be able to provide an optimized game state for
 * a hash table.
 *
 * If the game supports partition search, getPartition should return a valid
 * partition.
 *
 */

class ProblemState {
 public:
	ProblemState();
	virtual ~ProblemState();
	void addPlayer(Player *p);
	void deletePlayers();
	Player *getPlayer(int which) const;
	int getPlayerNum(Player *) const;
	virtual void Reset(int NEWSEED = -1);
	virtual void Print(int val = 0) const;
	virtual void ApplyMove(Move *move) {}
	virtual void UndoMove(Move *move) {}
	virtual Move *getMoves()  // get moves for next player
	{ printf("Can't call virtual ProblemState::Move\n"); exit(1); return 0; }

	virtual bool Done() const;

	// redefine to use hash table
	//virtual HashState *getHashState() { return 0; }
	// define to use history heuristic
	virtual uint32_t getMoveHash(const Move *m) { return 0; }

 private:
	Player *searchAgent;
};


#endif
