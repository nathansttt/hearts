/*
 *  UCT.h
 *  games
 *
 *  Created by Nathan Sturtevant on 11/16/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef UCT_H
#define UCT_H

#include "Algorithm.h"
#include "algorithmStates.h"

namespace hearts {

class UCTNode {
public:
	UCTNode()
	:m(0), reward(0), count(0), parent(-1), depth(0), children()
	{}
	UCTNode(Move *move, int par)
	:m(move), reward(0), count(0), parent(par), depth(0), children()
	{}
	~UCTNode() { children.resize(0); }
	//UCTNode &operator=(const UCTNode&);
	Move *m;
	double reward;
	int count;
	int parent;
	int depth;
	std::vector<int> children;	
};


//UCTNode &UCTNode::operator=(const UCTNode &source)  
//{
//	m = source.m;
//	reward = source.reward;
//	count = source.count;
//	parent = source.parent;
//	children = source.children;
//	return *this;
//}

class UCTModule {
public:
	virtual ~UCTModule() {}
	virtual maxnval *DoRandomPlayout(GameState *g, Player *p, double epsilon) = 0;
	virtual const char *GetModuleName() = 0;
	virtual void GetPreInformation(GameState *g, unsigned int who,
								   int &experience, double &value)
	{ experience = 0; }
};

class UCT : public Algorithm {
public:
	UCT(int numRuns, double cval1, double cval2);
	UCT(int numRuns, int crossOver, double cval1, double cval2);
	UCT(int numRuns = 10000, double cval = -1);
	UCT(char *n, int numRuns = 10000, double cval = 2);
	Algorithm *clone() const { return new UCT(*this); }
	virtual const char *getName();// { return name; }
	
	void setPlayoutModule(UCTModule *m);
	void setEpsilonPlayout(double v);
	void setUseHH(bool use) { HH = use; }
	
	void resetGameState() {  }

	virtual returnValue *Play(GameState *g, Player *p); // choose next move
	returnValue *Analyze(GameState *g, Player *p); // return eval of all moves
	void Played(int, Move *);

	maxnval *PlayUCTTree(GameState *g, int location);	
	maxnval *DoRandomPlayout(GameState *g);
	maxnval *GetValue(GameState *g);
	double GetUCTVal(GameState *g, int parent, int child);
	double GetC(GameState *g, int parent, int child);
	void ExpandChildren(GameState *g, int location);
	void FreeTree(GameState *g);
	void PrintTreeStats();
	void SetShowMoveStats(bool verboseMove) { verboseMoves = verboseMove; }
protected:
	void PrintTreeNode(int location, int indent = 0);
	Move *GibbsSample(GameState *g);

	UCTModule *pm;
	mt_random rand;
	char name[64];
	int currTreeLoc;
	int numSamples, currentSample, switchLimit;
	double C1, C2;
	bool verboseMoves;
	bool HH;
	std::vector<UCTNode> tree;
	double epsilon;
};

} // namespace hearts

#endif
