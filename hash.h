#include <stdlib.h>

#ifndef _HASH_H
#define _HASH_H

extern int creationCounter;

class State {
 public:
	State();
	virtual ~State();
	virtual unsigned long hash_key() = 0;
	virtual bool equals(State *val) = 0;
	virtual int type() { return 0; }
	virtual void Print(int val = 0) const { }
  // for debugging
  int nodeNum;
};

struct HNode {
  HNode(State *val, HNode *nxt = 0);
  ~HNode();
  State *key;
  HNode *next;
  int hits;
};

class HashTable {
 public:
  HashTable(int size);
  ~HashTable();
  void Add(State *val);
  void Remove(State *val);
  State *IsIn(State *val) const;
	int getNumElts() const { return elts; }
  void PrintStats() const;
  void Clear();
  void iterReset();
  bool iterDone();
  State *iterNext();
 private:
  unsigned long hash(State *val);
  HNode **table;
  int tsize;
  int elts;
  int iterIndex;
  HNode *iterNode;
};

#endif
