#include <vector>
#include "GameState.h"

#ifndef ALGORITHMSTATES_H
#define ALGORITHMSTATES_H

namespace hearts {

class LimitedEvalPlayer;

/*************** states ********************/
class bayesState : public algorithmState {
public:
	bayesState() {}
	inline bool equals(State *val) { return true; }
	void Print(int val = 0) const { }
	unsigned long hash_key() { return 0; }
};

class maxnState : public algorithmState {
 public:
  maxnState(double lim=0, short after=0, short d = 0);
  inline bool equals(State *val)
	{ maxnState *ms = (maxnState *)val;
		if ((next == ms->next) &&
				(depth == ms->depth) &&
				(limit == ms->limit))
			return true;
		return false; }
  void Print(int val = 0) const;
  unsigned long hash_key() { return ((next<<10)|(depth<<20)|((int)limit)); }
  double limit;
  short next, depth;
};

class minimaxState : public algorithmState {
 public:
  minimaxState(double aa = 0, double bb = 0, short nex = 0, short d = 0, short dr=0);
  void Print(int val = 0) const;
  unsigned long hash_key() { return (next<<15)+depthRemaining+(depth<<23); }
  double a, b, result;
  short next, depth, depthRemaining;
  bool equals(State *val);
};

class tableMaxnState : public maxnState {
public:
  tableMaxnState(double lim=0, short after=0, short d = 0, uint32_t _nodeID=0)
  :maxnState(lim, after, d), nodeID(_nodeID) {}
	uint32_t nodeID;
};

/*************** return values *************/

class maxnval : public returnValue {
 public:
  maxnval();
  ~maxnval();
  returnValue *clone(GameState *g) const;
  void Print(int v = 1) const;
  double getValue(int who) const;
  double eval[6];
  int numPlayers;
};


class minimaxval : public returnValue {
 public:
  minimaxval();
  minimaxval(double v, Move *mv = 0);
  ~minimaxval();
  returnValue *clone(GameState *g) const;
  double getValue(int who) const;
  void Print(int v = 1) const;
  double val;
};

class partition : public returnValue {
public:
  partition(GameState *g, double score, double d);
  virtual void Union(partition *p, GameState *g) = 0;
  //virtual void appendMove(Move *, GameState *);                               
  virtual returnValue *clone(GameState *g) const = 0;
  virtual HashState *getHashState(GameState *, int) = 0;
};

class outcomePDF : public returnValue {
public:
	outcomePDF(Move *mm = 0, outcomePDF *nn = 0) :returnValue(mm, nn) {}
	~outcomePDF() { }
	returnValue *clone(GameState *g) const;
  void Print(int v = 1) const;
	//private:
	std::vector<std::vector<double> > utility;
	double realUtility;
	outcomePDF& operator=(const outcomePDF& opdf)
	{
		utility = opdf.utility;
		realUtility = opdf.realUtility;
		this->m = opdf.m->clone();
		return *this;
	}
};

} // namespace hearts

#endif
