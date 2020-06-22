//#include "random.h"
#include "GameState.h"
#include "States.h"

#ifndef _ALGORITHM_H
#define _ALGORITHM_H

namespace hearts {

class returnValue;
class Player;

const double kMaxTimeLimit = INF;

/*
 * Class Algorithm
 *
 * The algorithm class forms a framework for an algorithm that can be run on a
 * search tree. The only function that should be defined is the function Play,
 * which does the actual search. But, there are a few things that need to be done
 * during the search.
 *
 * use Algorithm::getMoves() to get the possible moves.
 *
 * You should check searchExpired at the beginning of each loop in your search 
 * procedure.
 *
 * The algorithm class is currently set up to do iterative searches according to the
 * results returned from setupNextIteration()
 *
 * The Play function demonstrates how to do the iterative search. You can use this
 * function to do your iterative search, and just overload the search procedure
 * if you like. If you need more control, overload Play.
 */
class Algorithm {
public:
	Algorithm();
	Algorithm(const Algorithm& a);
	virtual ~Algorithm();
	virtual Algorithm *clone() const { return 0; }
	Move *getMoves(GameState *g, Player *p);
	Move *getRandomMove(GameState *g);
	virtual returnValue *Play(GameState *g, Player *p);
	virtual returnValue *Analyze(GameState *g, Player *p);
	virtual returnValue *DispatchSearch(unsigned int depth, int cp, GameState *g)
	{ printf("Error; using default Play/Analyze, but not overriding Algorithm::DispatchSearch\n"); exit(10); }
	virtual HashState *getHashState(GameState *g, HashState *mem = 0);
	//virtual returnValue *search(GameState *g, Player *p) = 0;
	void ApplyMove(GameState *g, Move *m);
	void UndoMove(GameState *g, Move *m);

	/*
	 * set___DepthLimit:
	 *
	 * this functions set the limits on search based on time, nodes and/or depth
	 */
	void setSearchDepthLimit(int val);
	int  getSearchDepthLimit() { return SEARCHDEPTHLIMIT; }
	void setSearchTimeLimit(double time);
	double getSearchTimeLimit() { return SEARCHTIMELIMIT; }
	void setSearchNodeLimit(unsigned long val=uINF);
	unsigned long getSearchNodeLimit() { return SEARCHNODELIMIT; }
	
	void setUseAfterstates(bool use) { 	useAFTERSTATES = use; }
	void setUseHashTable(bool use);
	void setVerifyHashTable(bool verify);
	void setUseThreads(bool use) { useTHREADS = use; }
	bool usingThreads() { return useTHREADS; }
	void setRandomizeMoves(bool use) { randomize = use; }
	void setVerbose(bool verb) { VERBOSE = verb; }
	void setUsePruning(bool prune) { PRUNING = prune; }
	void setSaveTree(bool save) { SAVE = save; }

	void logNodes();

	void setBranchingFactorLimit(unsigned long val);
	int getIterationSearchLimit();

	/*
	 * Reset our node/time counters
	 */
	void resetCounters(GameState *g);
	unsigned long getNodesExpanded() { return nodesExpanded; }
	unsigned long getTotalNodesExpanded() { return totalNodesExpanded; }
	void resetTotalNodesExpanded() { totalNodesExpanded = 0; }
	virtual void resetGameState() {} // resets game-length states (eg hash tables, etc)

	/*
	 * Hash Table functions:
	 *  - checkHashTable looks up a state and returns a match, if any.
	 *  - addHashTable takes a state and a move associated with it,
	 *    and puts it in the table
	 */
	const HashState *checkHashTable(HashState *s);
	virtual void addHashTable(HashState *s, returnValue *r);
	void clearHashTable(GameState *g);

	/*
	 * setupNextIteration - this should be called once before each search iteration
	 * in the game tree.
	 */ 
	int setupNextIteration(GameState *g, Player *p);
	virtual const char *getName() { return "undefined"; }


	double timeElapsed() { t2 = ((double)clock()/CLOCKS_PER_SEC); return t2-t1; }
	unsigned int getCurrentSearchDepth() { return searchDepth; }
	unsigned int getCurrentSearchDepthLimit() { return iterDepth; }
	//unsigned int getCurrentIterationDepth() { return totalMoves; }
		
	/*
	 *  public variables
	 */
	Player *who; // the player doing the search
protected:
	void setReturnValueType(returnValue *v);
	returnValue *getNewReturnValue(GameState *g);
	void freeReturnValue(returnValue *);
	// returns true if our search is done.
	bool searchExpired(GameState *g);
	HashTable *ht;
	int iterDepth;
	bool USEHASH;
	bool SAVE;
	bool VERIFYHASH;
	bool VERBOSE;
	bool PRUNING;
	Move *prevBest;
	mt_random rand;
private:
	returnValue *analyzeHelper(unsigned int depth, unsigned int cp, GameState *g);
	bool timeExpired();
	int searchDepth;
	unsigned int totalMoves; // the depth when we start our search
	unsigned long nodesExpanded;
	unsigned long totalNodesExpanded;
	bool useDEPTHLIMIT;
	bool useAFTERSTATES;
	bool useNODELIMIT;
	bool useTIMELIMIT;
	bool useTHREADS;
	bool randomize;
	unsigned int SEARCHDEPTHLIMIT, startDepth;
	double SEARCHTIMELIMIT;
	unsigned long SEARCHNODELIMIT;
	unsigned long BFLIMIT;
	double t1, t2;

	returnValue *returnValueList;
};

} // namespace hearts

#endif
