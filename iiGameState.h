#include "GameState.h"

#ifndef ii_GAMESTATE_H
#define ii_GAMESTATE_H

// for an imperfect information game, there is 1 master GameState containing the
// complete information in the game. Then, there is a iiGameState for each player,
// defining the knowledge they have. Each player can only access their own iiGameState.

// This allows them to query game-specific knowledge if needed. But, more importantly,
// it allows them to create models of their opponents which they can try to solve.

// I need to spend some time thinking about imperfect information search algorithms;
// how do they generate moves? Is it the same as a regular game, or do we do something
// else special. Is the existing framework suitable for those, or do I need to add
// something more like this framework?

class iiGameState {
public:
	iiGameState();
	virtual ~iiGameState();
	virtual GameState *getGameState(double &prob) { return 0; }
	virtual void getGameStates(int count, std::vector<GameState *> &states, std::vector<double> &prob);
	virtual unsigned int getNumGameStates() { return uINF; }
	virtual void Played(Move *) = 0;
	virtual void Unplayed(Move *) = 0;
	virtual void reset() {}
	virtual const char *GetName() = 0;
};


#endif

