#include <vector>
#include "Player.h"
#include "States.h"

#ifndef _GAME_H_
#define _GAME_H_

namespace hearts {

class Game {
 public:
	Game(GameState *gs);
	virtual ~Game();
	virtual void addPlayer(Player *p);
	virtual int Play();
	virtual void doOnePlay();
	virtual void Print();
	virtual bool Done();
	virtual int Winner() const;
	virtual bool PlayerWon(int who) const;
	virtual int Rank(int who);
	virtual void addScore(int who, double score);
	virtual void Reset();
	void setVerboseDisplay(bool verb) { verbose = verb; }
	void setMoveLimit(unsigned int val);
	double score(int who);
	GameState *getGameState() { return theGame; }
	int GetNumMovesInGame() { return gameMoves.size(); }
	Move *getNthMove(unsigned int n)
	{ if (n > gameMoves.size()) return 0; return gameMoves[n]; }
protected:
		std::vector<Move *>	gameMoves;
	bool verbose;
	int numPlayers;
	unsigned int moveLimit;
	Player** men;
	GameState *theGame;
};

} // namespace hearts

#endif
