#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mt_random.h"
//#include "random.h"
#include "GameState.h"
#include "iiGameState.h"
#include "Game.h"

#ifndef CARDGAMESTATE_H
#define CARDGAMESTATE_H

//#define min(x, y) (((x)>(y))?(y):(x))
//#define max(x, y) (((x)<(y))?(y):(x))

class CardPlayer;
class CardGameState;

typedef int16_t card;
typedef uint8_t byte;

enum {
	kCardPartitionState = 0xFEED,
	kCardState = 0xBEEF
};

void PrintCard(card c, FILE *f = stdout);
uint64_t getFullDeck();

class Deck {
public:
	Deck() { reset(); }
	inline void reset()
	{ cards = 0;
	counts[0] = counts[1] = counts[2] = counts[3] = -1; }
	inline void Shuffle() { r.srand(time((time_t*)0)); fill(); }
	inline void Shuffle(int SEED) { r.srand(SEED); fill(); }
	inline void fill()
	{ cards = getFullDeck(); 
	counts[0] = counts[1] = counts[2] = counts[3] = -1; }

	inline bool has(int suit, int rank) const
	{ return has(getcard(suit, rank)); }

	inline bool has(int which) const
	{ return (cards>>(which))&1; }

	inline void set(int suit, int rank)
	{ return set(getcard(suit, rank)); }
	inline void set(int which)
	{ cards = cards|(((uint64_t)1)<<which);
          counts[getsuit(which)] = -1; }

	inline void clear(int which)
	{ cards = cards&(~(((uint64_t)1)<<which)); counts[getsuit(which)] = -1; }
	inline void clear(int suit, int rank)
	{ clear(getcard(suit, rank)); }

	inline bool hasSuit(int suit) const
	{ return ((cards>>(16*suit))&(0xFFFF)); }

	inline bool empty() const
	{ return (cards == 0); }

	inline uint16_t getSuit(int suit) const
	{ return (uint16_t)((cards>>(16*suit))&(0xFFFF)); }
	void setSuit(int suit, uint16_t val)
	{
		cards = (cards&(~((uint64_t)0xFFFF<<(16*suit))))|((uint64_t)val<<(16*suit));
		counts[suit] = -1;
	}
	void addSuit(int suit, uint16_t val)
	{
		cards = (cards|((uint64_t)val<<(16*suit)));
		counts[suit] = -1;
	}	
	inline uint64_t getHand() const
	{ return cards; }
	inline void removeHand(Deck *h)
	{ cards = cards & (~h->cards); counts[0] = counts[1] = counts[2] = counts[3] = -1; }
	inline void setHand(const Deck *h)
	{ cards = h->cards; counts[0] = counts[1] = counts[2] = counts[3] = -1; }
	inline void setHand(uint64_t h)
	{ cards = h; counts[0] = counts[1] = counts[2] = counts[3] = -1; }
	inline void addHand(const Deck *h)
	{ cards |= h->cards; counts[0] = counts[1] = counts[2] = counts[3] = -1; }
	inline void addHand(uint64_t h)
	{ cards |= h; counts[0] = counts[1] = counts[2] = counts[3] = -1; }

	inline static card getcard(int suit, int rank)
	{ return (suit<<4)+rank; }

	inline static uint16_t getsuit(card c)
	{ return ((c>>4)&0x3); }

	inline static uint16_t getrank(card c)
	{ return (c&0xF); }

	inline static bool higher(card a, card b)
	{ return (a<b); }

	inline static bool lower(card a, card b)
	{ return (a>b); }

	card Deal();
	card getRandomCard();
	card getRandomCard(int suit);

	uint32_t suitCount(int which) const;
	uint32_t count() const;
	card suitHigh(int which) const;
	card suitLow(int which) const;
	int numCardsHigher(card c) const;
	void print() const;
private:
	mutable int16_t counts[4];
	mt_random r;
	uint64_t cards;
};


class Trick {
public:
	Trick(int t = -1) { reset(0, t);}
	virtual ~Trick() {}
	virtual int Winner() const;
	virtual int WinningCard() const;
	virtual int bestRankWin() const;
	//virtual int LosingCard();
	//virtual int Points(); // not being used - eval is proper place to overload
	void Display();
	void Print() { Display(); }
	void print() { Display(); }
	void AddCard(card c, int who);
	card RemoveCard();
	bool Done() { return (curr>=np); }
	void reset(int numP, int t = -1);
	int np;
	int curr;
	card play[MAXPLAYERS];
	int player[MAXPLAYERS];
private:
	int trump; // suit which is trump. -1 for no trump
	int points;
};

typedef enum {
	kLeadCard,
	kFollowCard,
	kSloughCard,
	kDiscardCard
} cardContext;

class CardMove : public Move {
public:
	CardMove(Move *n = 0) :Move(n) { }
	CardMove(card cd, int who, Move *n=0)
	:Move(n) { c = cd; player = who; }
	void init(card cd, cardContext ctxt, int who, Move *n=0) { c = cd; context = ctxt, player = who; next = n;}
	void Print(int ex, FILE *f = stdout);

	Move *clone(GameState *g) const;
	Move *clone() const;
	bool equals(Move *m) { if (m) return (((CardMove*)m)->c == c); return false; }
	uint32_t getHash() { return c; }
	//static void PrintCard(card c, FILE *f = stdout);
	card c;
	cardContext context;
};

class CardGame : public Game {
public:
	CardGame(GameState *gs) :Game(gs) { started = false; }
	virtual int Play();
	virtual void doOnePlay();
	virtual void addPlayer(Player *p)
	{ started = false; Game::addPlayer(p); }
	virtual int getMaxPoints() { return 1; }
protected:
	bool started;
};

class CardGameState : public GameState {
public:
	CardGameState(std::vector<std::vector<card> > &theCards, int trump=-1, int seed = 1, int spec = -1, int nc = -1);
	CardGameState(int trump=-1, int seed = 1, int spec = -1, int nc = -1);
	virtual ~CardGameState();
	virtual CardGameState *create();
	virtual void Reset(int NEWSEED = -1);
	virtual void SetInitialCards(std::vector<std::vector<card> > &cards);
	
	virtual void DealCards();
	virtual Move *getMoves();
	virtual Move *getAllMoves();
	virtual Move *getRandomMove();

	virtual void Print(int val = 1) const;
	Player *getNextPlayer() const;
	int getNextPlayerNum() const;
	int getPreviousPlayerNum() const;
	Player *getPreviousPlayer() const;

	virtual void setRules(int value) { rules = value; }
	virtual int getRules() { return rules; }
	virtual void setFirstPlayer(int first) { firstPlayer = first; currPlr = first; }

	using GameState::score;
	virtual int trickScore(const Trick *_t) const { return 0; }
	const Trick *getCurrTrick() const { return &t[currTrick]; }
	inline int getCurrTrickNum() { return currTrick; }
	const Trick *getTrick(int which) { if (which <= currTrick) return &t[which]; return 0; }
	virtual void ApplyMove(Move *move);
	virtual void UndoMove(Move *move);
	virtual bool Done() const;

	card getRelativeRank(card c);
	virtual uint32_t getMoveHash(const Move *m);
	Move *getMoveFromHash(uint32_t hash);
	virtual HashState *getHashState(HashState *mem = 0);
	//virtual partition *getPartition(int me, double value);
	//virtual HashState *getPartitionHashState(GameState *g, int me);
	virtual uint64_t HashCurrentState(int who) const { return cards[who].getHand(); }

	Deck allplayed; // cards from all players
	Deck cards[MAXPLAYERS];
	Deck played[MAXPLAYERS];
	Deck taken[MAXPLAYERS];
	Deck original[MAXPLAYERS];

	int special; // suit that can't be played until broken

	virtual iiGameState *getiiGameState(bool consistent, int who, Player *playerModel = 0);
	void TakeCard(int who, card c);
	void UntakeCard(int who, card c);
//protected:
//	friend class iiCardState; 
	int numCards;
	Trick *t;
	// private:
	int trump;
	int currPlr, firstPlayer;
	int currTrick;
	int rules;
	int SEED;
	Deck d;
private:
	Move *allocateMoreMoves(int n);
};

//#define tSIZE 2756

class CardPlayer : public Player {
public:
	CardPlayer(Algorithm *alg);
	virtual ~CardPlayer();
	virtual int getNextDepth(int curr);
	virtual unsigned int getMaxDepth();
private:
//	uint64_t ranks[tSIZE];
};

class cardHashState : public gameHashState {
public:
	cardHashState() { }
	~cardHashState() { }
	//{ delete [] cards; delete [] pts; delete [] numx; delete [] numCards; }
	unsigned long hash_key();
	bool equals(State *val);
	int type() { return kCardState; }
	void Print(int val = 0) const;
	
	
	uint64_t cards[MAXPLAYERS];
	unsigned short pts[MAXPLAYERS];
	unsigned int numx[MAXPLAYERS];
	uint64_t numCards;
	int nump, nextp;
	//  double a, b;
};
/*

class cardPartition : public partition {
public:
 	cardPartition(GameState *g, double score, double d);
	//cardPartition();
	void Print(int val = 1) const;
	double getValue(int who) const;
	returnValue *clone() const;

	void Union(partition *val, GameState *g);
	//unsigned long hash_key();
	//bool equals(State *val);
	//int type() { return kCardPartitionState; }
	HashState *getHashState(GameState *, int me);
	void appendMove(Move *mm, GameState *g);
private:
	//void makePartition(CardGameState *g, byte counts[4][4]);
	//uint64_t cardcount; // 4 bits x 4 suits x 4 players = 64 bits
	// next = 4 bits, points = 4 bits
	//byte info; // next, number points for root to win
	byte cards[4][4]; // player/suit
	byte cardcount[4][4];
	byte cardxs[4][4];
	bool won;
	int score;
};

class cardPartitionHashState : public gameHashState {
public:
	cardPartitionHashState(GameState *g, double score, const byte countlimit[4][4] = 0);
	~cardPartitionHashState() {}
	unsigned long hash_key();
	bool equals(State *val);
	int type() { return kCardPartitionState; }
	void Print(int val = 0) const;
	double won;
private:
	unsigned short cards[4][4]; // player/suit
	byte cardcount[4][4];
	byte cardxs[4][4];
	int score;
};

*/

class iiCardState : public iiGameState {
public:
	iiCardState() { t = 0; }
	void init(int trump, int spec, int nc, int numPlayers, int seed);
	virtual ~iiCardState();
	virtual GameState *getGameState(double &prob);
	void Played(Move *);
	void Unplayed(Move *);
	void hasCard(card c, int who);
	void hasCards(uint64_t c, int who);

	static uint64_t getNumCombos(std::vector<int> &counts, Deck &d);
	static uint64_t getNumCombos(std::vector<int> &counts);
	static void GetConstellation(std::vector<int> &counts, uint64_t arrangement, std::vector<std::vector<card> > &cards, Deck &d);
	
	CardGameState *originalGame; // this is only used so we can clone the new game state
	Player *master; // only used to clone new players for the game
	int rules, firstPlayer;
	int currPlr;
	static uint64_t getIndexFromCards(std::vector<card> &cards, int available, int thisPlayer);
	static void getCards(uint64_t val, std::vector<card> &cards, int available, int thisPlayer);
	static uint64_t choose(int n, int m);

//protected:
	//	friend class CardGameState;
	Trick *t;

	Deck d;
	int trump;
	int special;
	int numCards;
	int numPlayers;
//	int me;

	int currTrick;
	int totalMoves;

	Deck cards[MAXPLAYERS];
	Deck taken[MAXPLAYERS];
	Deck original[MAXPLAYERS];
	Deck played[MAXPLAYERS];
	Deck allplayed;
	int SEED;
	mt_random rand;
};

class cardMoveAbstraction : public moveAbstraction
{
public:
	virtual ~cardMoveAbstraction() {}
	virtual uint32_t getMoveHash(int who, Move *m) { return ((CardMove*)m)->c; }
	virtual bool compareMoveHash(uint32_t a, uint32_t b) { return a == b; }
	virtual Move *abstractMove(int who, GameState *g, Move *m) { return m->clone(g); }
	virtual Move *translateMoveApply(GameState *g, Move *m) { return m->clone(g); }
	virtual Move *translateMoveUndo(GameState *g, Move *m) { return m->clone(g); }
};

class hiddenCardMoveAbstraction : public moveAbstraction
{
public:
	virtual ~hiddenCardMoveAbstraction() {}
	virtual uint32_t getMoveHash(int who, Move *m)
	{
		if (who == m->player)
			return ((CardMove*)m)->c;
		return 10;
	}
	virtual bool compareMoveHash(uint32_t a, uint32_t b)
	{ if ((a == 10) || (b == 10)) return true; return a == b; }
	virtual Move *abstractMove(int who, GameState *g, Move *m) { return m->clone(g); }
	virtual Move *translateMoveApply(GameState *g, Move *m) { return m->clone(g); }
	virtual Move *translateMoveUndo(GameState *g, Move *m) { return m->clone(g); }
private:
};

#endif

/*
 these were formerly in the CardPlayer class
 
 //virtual Move *getMoves();
	//virtual Player *clone() const { return new CardPlayer(algorithm); }
	//the higher number the more we like it
	//virtual int rankMove(Move *m, GameState *g, bool optional=false) { return 0; }
	//virtual int rankMove(int c, const Trick *t, GameState *g) { return 0; }
	//virtual uint64_t bestMove(const Move *, GameState *g);
	
 //	virtual double score(unsigned int who) { printf("Wrong!\n"); return 0; }
 //	virtual double scoreUpperBound(unsigned int who = uINF) { return INF; }
 //	virtual double scoreLowerBound(unsigned int who = uINF) { return NINF; }
 //	virtual double scoreMinDelta() { return 1; }
 //	virtual double scoreMaxDelta() { return 1; }
 //	virtual double cutoffEval(unsigned int who = uINF) { printf("Wrong!!!\n"); return score(who); }
 //virtual void addHashTable(State *s, returnValue *r);
 //void addPartition2Hash(partition *p, int cp, double b);
 //void addPartition2Hash(returnValue *p, int cp, double b);

 */
