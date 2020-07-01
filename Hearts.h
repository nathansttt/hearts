//#include "LimitedEvalPlayer.h"
#include "CardGameState.h"
//#include "BPPlayer.h"
//#include "RRPlayer.h"
//#include "PerceptronPlayer.h"
//#include "mathUtil.h"
#include "UCT.h"
#include "CardProbabilityData.h"

#ifndef HEARTS_H
#define HEARTS_H

namespace hearts {

enum tPassDir { kLeftDir=1, kRightDir=-1, kAcrossDir=2, kHold=0 };

enum {
	kQueenPenalty = 0x0001,          //  13 for QS
	kJackBonus = 0x0002,             // -10 for JD
	kNoTrickBonus = 0x0004,          //  -5 for no tricks
	kEndScoreBonus = 0x0008,         // ???
	kShootingNeedsJack = 0x0010,     // must have jack to shoot
	kLead2Clubs = 0x0020,            // 2C leads
	kLeadClubs = 0x0040,             // just clubs leads
	kNoHeartsFirstTrick = 0x0080,    // no hearts first trick
	kNoQueenFirstTrick = 0x0100,     // no queen first trick
	kQueenBreaksHearts = 0x0200,     // queen breaks hearts
	kDoPassCards = 0x0400,           // pass
	kMustBreakHearts = 0x0800,        // must break hearts
	kHeartsArentPoints = 0x1000,        // hearts are 1 point each
	kNoShooting = 0x2000        // hearts are 1 point each
};

enum suit {
	SPADES = 0,
	DIAMONDS = 1,
	CLUBS = 2,
	HEARTS = 3
};

enum rank {
	ACE = 0,
	KING = 1,
	QUEEN = 2,
	JACK = 3,
	TEN = 4,
	NINE = 5,
	EIGHT = 6,
	SEVEN = 7,
	SIX = 8,
	FIVE = 9,
	FOUR = 10,
	THREE = 11,
	TWO = 12
};

enum tScoreUtility {
 kMinimizeOwnScore,
 kMaximizeMarginLocal,
 kMaximizeMarginGlobal,
 kMaximizeLeaderLocal,
 kMaximizeLeaderGlobal
};

class HeartsGameState : public CardGameState
{
public:
	HeartsGameState(std::vector<std::vector<card> > &theCards, int seed = 1)
	:CardGameState(theCards, -1, seed, HEARTS, 13) { rules = 0; passDir = kHold; }
	HeartsGameState(int seed = 1)
	:CardGameState(-1, seed, HEARTS, 13) { rules = 0; passDir = kHold; }
	CardGameState *create();
	virtual void Print(int val = 1) const;
	virtual void ApplyMove(Move *move);
	virtual void UndoMove(Move *move);
	HashState *getHashState(HashState*);
	void Reset(int SEED = -1);
	void DealCards();
	Move *getMoves();
	Move *getAllMoves();
	virtual Move *getRandomMove();// { return GameState::getRandomMove(); }
	bool IsLegalMove(Move *m);
	void setPassDir(int dir);
	int getPassDir() { return passDir; }
	bool donePassing() { return ((!(rules&kDoPassCards)) || (passDir == kHold) || (numCardsPassed == getNumPlayers()*3)); }
	int Pass();
	int score(const Trick *t) const;
	double score(int who) const;
	int Winner() const;
	void setFirstPlayer(int first);
	virtual void waitEndTrick();
	iiGameState *getiiGameState(bool consistent, int who, Player *playerModel);

	void MeasureProperties();

	int passDir;
	int numCardsPassed;
	std::vector<card> passes[MAXPLAYERS];
private:
	mt_random rand;
};

class HeartsCardGame : public CardGame {
public:
	HeartsCardGame(GameState *gs) :CardGame(gs) { mp = 100; }
	virtual int Play();
	virtual void doOnePlay();
	virtual int getMaxPoints() { return mp; }
	virtual void setMaxPoints(int maxp) { mp = maxp; }
	int Winner() const;
	bool Winner(int who);
	int Rank(int who);
private:
	int mp;
};

class HeartsPlayer {
public:
	virtual ~HeartsPlayer() {}
	virtual void selectPassCards(int dir, card &a, card &b, card &c) = 0;
};

class HeartsDucker : public CardPlayer, HeartsPlayer {
public:
	HeartsDucker() :CardPlayer(0) {}
	const char *getName() { return "HeartsDucker"; }
	virtual Move *Play();
	virtual void selectPassCards(int dir, card &a, card &b, card &c);
	virtual Player *clone() const { return new HeartsDucker(); }
	virtual double score(unsigned int who) { return g->score(who); }
};

class HeartsShooter : public CardPlayer, HeartsPlayer {
public:
	HeartsShooter() :CardPlayer(0) {}
	const char *getName() { return "HeartsShooter"; }
	virtual Move *Play();
	virtual void selectPassCards(int dir, card &a, card &b, card &c);
	virtual Player *clone() const { return new HeartsDucker(); }
	virtual double score(unsigned int who) { return g->score(who); }
};

class HeartsPlayout : public UCTModule {
public:
	maxnval *DoRandomPlayout(GameState *g, Player *p, double epsilon);
	Move *DoMinPlay(CardGameState *cgs, bool split, double epsilon);
	const char *GetModuleName() { return "HPlayout"; }
private:
	mt_random rand;
};

	class HeartsPlayoutCheckShoot : public UCTModule {
public:
	maxnval *DoRandomPlayout(GameState *g, Player *p, double epsilon);
	Move *DoMinPlay(CardGameState *cgs, bool split, double epsilon);
	Move *DoMaxPlay(CardGameState *cgs, int me, double epsilon);
	const char *GetModuleName() { return "HCheckPlayout"; }
private:
	mt_random rand;
};

class SimpleHeartsPlayer : public CardPlayer, HeartsPlayer, public UCTModule {
public:
	SimpleHeartsPlayer(Algorithm *alg);//, tScoreUtility u);
	void selectPassCards(int dir, card &a, card &b, card &c);
	void setModelLevel(int val) { modelLevel = val; }
	virtual const char *getName();
	virtual const char *GetModuleName() { return "Simple"; }
	virtual Player *clone() const;
	iiGameState *getiiModel();

	double score(unsigned int who);
	virtual double cutoffEval(unsigned int who = uINF);
	maxnval *DoRandomPlayout(GameState *g, Player *p, double epsilon);
protected:
	mt_random rand;
private:
	Move *DoMinPlay(CardGameState *cgs, double epsilon);
	Move *DoShootPlay(CardGameState *cgs);
	bool canShoot(CardGameState *cgs);
	int modelLevel;
};

class GlobalHeartsPlayer : public SimpleHeartsPlayer {
public:
	GlobalHeartsPlayer(Algorithm *alg) :SimpleHeartsPlayer(alg) {}
	Player *clone() const;
	virtual double cutoffEval(unsigned int who = uINF);
	virtual const char *getName();
	virtual const char *GetModuleName() { return "GlobalHearts1"; }
};

class GlobalHeartsPlayer2 : public SimpleHeartsPlayer {
public:
	GlobalHeartsPlayer2(Algorithm *alg) :SimpleHeartsPlayer(alg) {}
	Player *clone() const {	return new GlobalHeartsPlayer2(algorithm); }
	virtual double cutoffEval(unsigned int who = uINF);
	virtual const char *getName();
	virtual const char *GetModuleName() { return "GlobalHearts2"; }
};

class GlobalHeartsPlayer3 : public SimpleHeartsPlayer {
public:
	GlobalHeartsPlayer3(Algorithm *alg) :SimpleHeartsPlayer(alg) {}
	Player *clone() const {	return new GlobalHeartsPlayer3(algorithm); }
	virtual double cutoffEval(unsigned int who = uINF);
	virtual const char *getName();
	virtual const char *GetModuleName() { return "GlobalHearts3"; }
};


class SafeSimpleHeartsPlayer : public SimpleHeartsPlayer {
public:
	SafeSimpleHeartsPlayer(Algorithm *alg) :SimpleHeartsPlayer(alg) {}
	Player *clone() const;
	virtual double cutoffEval(unsigned int who = uINF);
	virtual const char *getName();
	virtual const char *GetModuleName() { return "SafeSimple"; }
};

class HeartsCardPlayer : public CardPlayer, HeartsPlayer {
public:
	HeartsCardPlayer(Algorithm *alg) :CardPlayer(alg) { }
	Player *clone() const { return new HeartsCardPlayer(algorithm); }

	//		double score();
	double score(unsigned int who);
	double scoreUpperBound(unsigned int who = uINF);
	double scoreLowerBound(unsigned int who = uINF);
	double scoreMinDelta() { return 1; }
	double scoreMaxDelta() { return 16; }
	double cutoffEval(unsigned int who = uINF);
	double handEval(unsigned int who = uINF);
	//int handscore();
	double getMaxsum(GameState *g);

	virtual void selectPassCards(int dir, card &a, card &b, card &c);
	int rankMove(Move *m, GameState *g, bool optional=false);
private:
};

class iiHeartsState : public iiCardState {
public:
	iiHeartsState() :iiCardState() { advancedModeling = false; }
	void init(int tmp, int spec, int nc, int nPlayers, int seed)
	{
		iiCardState::init(tmp, spec, nc, nPlayers, seed);
	}
	~iiHeartsState() {}
	virtual GameState *getGameState(double &prob);
	virtual const char *GetName() { if (advancedModeling) return "OM-3"; return "OM-0"; }
	bool advancedModeling;
	int passDir;
	int numCardsPassed;
	std::vector<card> passes[MAXPLAYERS];
	static cardProbData cpd;
private:
	double GetTrickOdds(int player, Trick &t, int which, Deck &d);
	double GetProbability(int player, std::vector<card> &newCards, Trick *t, Deck &d);
	double GetPassProbability(std::vector<card> &passes, std::vector<card> &newCards);
	double GetCardProbability(int who, std::vector<card> passes[],
							  card c, std::vector<double> &priors);
};

class advancedIIHeartsState : public iiHeartsState {
public:
	advancedIIHeartsState();
	~advancedIIHeartsState();
	virtual GameState *getGameState(double &prob);
	virtual const char *GetName() { return "OM-2"; }
	static cardProbData cpd;
private:
	double GetTrickOdds(int player, Trick &t, int which, Deck &d);
	double GetProbability(int player, std::vector<card> &newCards, Trick *t, Deck &d);
	double GetPassProbability(std::vector<card> &passes, std::vector<card> &newCards);
	double GetCardProbability(int who, std::vector<card> passes[],
							  card c, std::vector<double> &priors);
};

class simpleIIHeartsState : public iiHeartsState {
public:
	simpleIIHeartsState();
	~simpleIIHeartsState();
	virtual GameState *getGameState(double &prob);
	virtual const char *GetName() { return "OM-1"; }
};

//class ModelingHeartsPlayer : public LimitedEvalPlayer, HeartsPlayer {
//public:
//	ModelingHeartsPlayer(Algorithm *alg) :LimitedEvalPlayer(alg) {}
//	Player *clone() const;
//	int rankMove(Move *m, GameState *g, bool optional=false);
//	
//	int getNumScores();
//	int getCurrentScoreIndex();
//	void getScoreAndEval(int which, int *values, int *evals);
//	bool isRawScorePossible(int *vals);
//	double score(unsigned int who); // actual game score
//	int getNextDepth(int curr);
//	unsigned int getMaxDepth();
//	virtual void selectPassCards(int dir, card &a, card &b, card &c);
//private:
//};

} // namespace hearts

#endif
