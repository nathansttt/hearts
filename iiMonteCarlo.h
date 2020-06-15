#include "Algorithm.h"
#include "algorithmStates.h"
#include <vector>

#ifdef __MAC__
#include <CoreServices/CoreServices.h>
//#include <Multiprocessing.h>
#endif

#ifndef iiMonteCarlo_h
#define iiMonteCarlo_h

class threadModel {
	public:
		Algorithm *alg;
		GameState *gs;
#ifdef __MAC__
		MPQueueID returnQ;
#endif
};

enum decisionRule {
	kMaxWeighted,
	kMaxAverage,
	kMaxAvgVar,
	kMaxMinScore
};

class iiMonteCarlo : public Algorithm {
public:
	iiMonteCarlo(Algorithm *a, int numModels, int numChoices = -1);
	iiMonteCarlo(Player *_player, int numModels);
	~iiMonteCarlo();
	Algorithm *clone() const { return new iiMonteCarlo(*this); }
	returnValue *Play(GameState *g, Player *p);
	returnValue *Analyze(GameState *g, Player *p);
	returnValue *DispatchSearch(unsigned int depth, int cp, GameState *g);
	Algorithm *getAlgorithm() { return algorithm; }
	int getNumModels() { return numModels; }
	void setNumModels(int val) { numModels = val; }
	const char *getName();
	void setDecisionRule(decisionRule r) { dr = r; }
private:
	const char *getDecisionName();
	Move *Combine(GameState *g, std::vector<returnValue *> &v, int who, std::vector<double> &probs);
	returnValue *CombinedAnalyze(GameState *g, std::vector<returnValue *> &v, int who, std::vector<double> &probs);
	void doModels(GameState *g, Player *p, std::vector<returnValue *> &v, std::vector<double> &probs);
	void doThreadedModels(GameState *g, Player *p, std::vector<returnValue *> &v, std::vector<double> &probs);
	void GetGameStates(GameState *g, Player *p, std::vector<GameState *> &states, std::vector<double> &probs);
	void NormalizeProbs(std::vector<double> &pr);
	int numModels, numChoices;
	Algorithm *algorithm;
	Player *player;
	decisionRule dr;
};

#ifdef __MAC__
OSStatus doThreadedModel(void *data);
#else
void *doThreadedModel(void *data);
#endif

#endif
