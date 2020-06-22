#include <stdio.h>
#include <stdint.h>
//#include "random.h"
#include "hash.h"

#ifndef STATES_H
#define STATES_H

namespace hearts {

class Player;
class Algorithm;
class GameState;


// Move conflicts with the OS 9/X function Move.
//This definition fixes the naming conflict.
#define Move _Move

class Move {
 public:
	Move(Move *n = 0, int d = 0) { next = n; dist = d; }
	virtual ~Move();
	virtual void Print(int ex, FILE *f=stdout)
	{ fprintf(f, "Can't print me--I'm generic!\n\n"); exit(0); }
	virtual Move *clone(GameState *g) const { return 0; }
	virtual Move *clone() const { return 0; }
	virtual bool equals(Move *m) { return false; }
	virtual uint32_t getHash() { return 0; }
	void insert(Move *m);
	void append(Move *m);
	int length() { if (next == 0) return 1; return next->length()+1; }
	int64_t dist;
	Move *next;
	int player;
//	mt_random rand;
};


class moveAbstraction
{
public:
	virtual ~moveAbstraction() {}
	virtual uint32_t getMoveHash(int who, Move *) = 0;
	virtual bool compareMoveHash(uint32_t, uint32_t) = 0;
	virtual Move *abstractMove(int who, GameState *, Move *) = 0; // unabstracts an abstract move and makes it applyable
	virtual Move *translateMoveApply(GameState *, Move *) = 0; // unabstracts an abstract move and makes it applyable
	virtual Move *translateMoveUndo(GameState *, Move *) = 0; // unabstracts an abstract move and makes it undoable
};

class returnValue {
 public:
	returnValue(Move *mm = 0, returnValue *nn = 0) { m = mm; next = nn; }
	virtual ~returnValue() { delete m; while (next) { returnValue *t = next; next = next->next; t->next = 0; delete t; } }
	virtual returnValue *clone(GameState *g) const { return new returnValue(m?m->clone(g):0, next?next->clone(g):0); }
	virtual void Print(int v = 1) const { if (next) next->Print(v); if (m) m->Print(v); }
	virtual double getValue(int who) const { return 0; }
	virtual void appendMove(Move *mm, GameState *g = 0) { mm->next = m; m = mm; }
	Move *m;
	returnValue *next;
};

// defines the game specific information
class gameHashState : public State {
 public:
	gameHashState() :State() {}
	virtual ~gameHashState() {}
	virtual bool equals(State *val) = 0;
	virtual int type() { return 0; }
};

// defines the algorithm specific information that needs to be
// saved in the hash table

class algorithmState : public State {
 public:
  algorithmState() :State() {}
  virtual ~algorithmState() {}
  virtual bool equals(State *val) = 0;
  virtual void Print(int val = 0) const {}
  virtual unsigned long hash_key() { return 0; }
};

class HashState : public State {
 public:
	HashState();
	virtual ~HashState() { delete ret; delete as; delete ghs; }
	virtual unsigned long hash_key();
	bool equals(State *val);
	int type() { return 0; }
	returnValue *ret; // contains list of moves & evaluations
	algorithmState *as; // contains other algorithm specific info
	gameHashState *ghs; // contains a game state (probably compressed)
	void Print(int val = 0) const;
};

} // namespace hearts

#endif
