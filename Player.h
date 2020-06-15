#include <vector>
#include <stdint.h>
#include "GameState.h"
#include "Algorithm.h"

#ifndef PLAYER_H
#define PLAYER_H

// return from rankMove if we don't want this move generated
// (usually if the optional flag is set)
enum {
	kIgnoreMove = -999999999
};

/*
 * Class Player
 *
 * This class defines player specific information to be used by a game.
 * That information includes:
 *  - evaluation functions (using the upper/lower bound functions 
 *    to describe the evaluation function being used may speed the search.)
 *  - move generation
 */
class Player {  
 public:
	Player(Algorithm *alg);
	virtual ~Player();
	void setAlgorithm(Algorithm *alg) { algorithm = alg; }
	Algorithm *getAlgorithm() { return algorithm; }
	void setGameState(GameState *g); // called automatically when added to a game
	virtual Move *Play();
	virtual void Played(int, Move *) {}
	virtual Player *clone() const { return new Player(algorithm); }
	/*
	 * Return a list of possible moves.
	 */
	//virtual Move *getMoves(int x, int y, int z)
	//{ printf("Can't call virtual Player::Move\n"); exit(1); return 0; }
	void setUseHistory(bool use);
//	void setRandomizeMoves(bool use);
	virtual uint64_t bestMove(const Move *, GameState *g); // for history heuristic
	virtual int rankMove(Move *m, GameState *g, bool optional = false);

	virtual iiGameState *getiiModel() { return 0; }
//	virtual double eval(GameState *g, int who)
//	{ printf("Can't call virtual Player::eval\n"); exit(1); return 0.0; }

	/*
	 * For an interative search, getNextDepth returns the next search depth.
	 * It should not return a value greater than getMaxDepth. If the current
	 * depth is equal to the maxdepth, the search should stop after the current
	 * iteration.
	 */
	virtual int getNextDepth(int curr) { if (curr == -1) return 1; return curr+1; }
	virtual unsigned int getMaxDepth() { return uINF; }

	/*
	 * The scoreXXX functions are used when computing information related
	 * to the exact score of the player. Maxn can use this information to do
	 * shallow pruning. Heuristic pruning can also be based on this information
	 *
	 * score should always return a value between the UpperBound and LowerBound.
	 * scoreMinDelta should return the smallest change in score possible
	 * scoreMaxDelta should return the maximum change in score from one iteration to the next
	 *
	 * cutoffEval is used when no score bounds are desired
	 *
	 * if bounds are not available, score should return the same thing as cutoffEval,
	 * and upper and lower bound should return INF and NINF
	 */
	// who of uINF indicates current player, ie "this"
	//virtual double maxsum() { return INF; }
	virtual double score(unsigned int who) { return g->score(who); }
	virtual double scoreUpperBound(unsigned int ) { return INF; }
	virtual double scoreLowerBound(unsigned int ) { return NINF; }
	virtual double scoreMinDelta() { return 0.001; }
	virtual double scoreMaxDelta() { return INF; }
	virtual double cutoffEval(unsigned int who);
	virtual void cutoffEvals(std::vector<double> &values);
	virtual double getMaxsum(GameState *) { return INF; }

	virtual const char *getName();

	// if you can predict the score of the game immediately, use this function
	virtual bool endGameEval(unsigned int , double &)
	{ return false; } 
	// tells the player the score they got in that round
	virtual void addScore(double) { if (algorithm) algorithm->resetGameState(); }
	// this doesn't belong here; deal with it later!
	virtual int getBid(int forbidden) { return (forbidden==0)?1:0; }

 protected:
		// in case no algorithm is defined, we just take max of
		// all moves (recursively, if we can move twice in a row)
		returnValue *getGreedyBestMove();

	friend class Game;
	int extras;
//	unsigned int maxp;
// private:
 	Algorithm *algorithm;
	GameState *g;
	double minimumScore;
	bool history;
	//bool randomize;
	bool learn;
	HashTable *historyTable;
 private:
 	void loadHistory();
 	void saveHistory();
};

class historyState : public State {
 public:
	historyState(unsigned long hk) { hash = hk; }
	unsigned long hash_key() { return hash; }
	bool equals(State *val) { return (((historyState*)val)->hash == hash); }
	int type() { return 0; }
	void Print(int val = 0) const { printf("%lu\n", hash); }

  uint64_t value;
  unsigned long hash;
private:
	historyState() {}
};


#endif
