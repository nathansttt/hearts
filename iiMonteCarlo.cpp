#include "Player.h"
#include "iiMonteCarlo.h"

#include <deque>
#include <pthread.h>
#ifndef __MAC__
#else
#include <CoreServices/CoreServices.h>
#endif
#include <assert.h>
#include "iiGameState.h"
#include "fpUtil.h"

#ifdef __MAC__
//#include "Diagnostics.h"
#endif

namespace hearts {

#define MAXMOVES 52
#define _PRINT_ 0
//#define min(x, y) ((x)<(y)?(x):(y))

#if GCC_VERSION < 4
//extern "C" int isnan(double);
#endif

iiMonteCarlo::iiMonteCarlo(Algorithm *a, int _numModels, int _numChoices)
{
	dr = kMaxWeighted;
	this->numModels = _numModels;
	if (_numChoices == -1)
		numChoices = numModels;
	else
		numChoices = _numChoices;
	algorithm = a;
	player = 0;
}

iiMonteCarlo::iiMonteCarlo(Player *_player, int _numModels)
{
	dr = kMaxWeighted;
	this->numModels = _numModels;
	algorithm = 0;
	player = _player;
}

iiMonteCarlo::~iiMonteCarlo()
{
//	delete algorithm;
	delete player;
}

const char *iiMonteCarlo::getName()
{
	static char name[1024];
	if (algorithm)
		sprintf(name, "MC_D-%s_M-%d__%s", getDecisionName(), numModels, algorithm->getName());
	else
		sprintf(name, "MC_D-%s_m-%d__%s", getDecisionName(), numModels, player->getName());

	return name;
}

const char *iiMonteCarlo::getDecisionName()
{
	switch (dr)
	{
		case kMaxWeighted: return "MaxWgt";
		case kMaxAverage: return "MaxAvg";
		case kMaxAvgVar: return "MaxAvgVar";
		case kMaxMinScore: return "MaxMin";
	}
	return "?";
}

returnValue *iiMonteCarlo::Play(GameState *g, Player *p)
{
	std::vector<returnValue *> v;
	std::vector<double> probs;
	Move *best;

	// 1. procure and analyze each model

	if (usingThreads() && (algorithm) && (algorithm->getSearchTimeLimit() == kMaxTimeLimit))
		doThreadedModels(g, p, v, probs);
	else
		doModels(g, p, v, probs);

	// 2. combine the results - only the algorithm knows how to do this.
	// 3. get the move with the highest expected results
	//	printf("Analyzing results\n");
	best = Combine(g, v, g->getPlayerNum(p), probs);
	//	best->Print(1);
	
	// 4. clean up memory
	for (int x = 0; x < numModels; x++)
		delete v[x];
	//delete [] v;

//	printf("Returning result\n");
	// 5. return result
	return new returnValue(best);
}

void iiMonteCarlo::doModels(GameState *g, Player *p, std::vector<returnValue *> &v, std::vector<double> &probs)
{
	//returnValue **v;
	v.resize(numModels);
	//v = new returnValue *[numModels];

//	if (strstr(p->getName(), "UCT250") != 0)
//	{
//		printf("%s not using advanced modeling\n", p->getName());
//		((iiSheepsheadState*)iiState)->advancedModeling = false;
//	}

	std::vector<GameState *> toAnalyze;
	GetGameStates(g, p, toAnalyze, probs);
	assert((int)toAnalyze.size() == numModels);
	
	for (int x = 0; x < numModels; x++)
	{
		v[x] = 0;
		if (!toAnalyze[x])
			continue;
		assert(toAnalyze[x] != 0);
#if _PRINT_
		printf("Getting model %d (prob: %f) for player %d\n", x, probs[x], g->getPlayerNum(p));
		toAnalyze[x]->Print(1);
//		printf("Searching model %d for %d\n", x, toAnalyze->getPlayerNum(toAnalyze->getNextPlayer()));
#endif
		algorithm->resetCounters(toAnalyze[x]);
		toAnalyze[x]->copyMoveList(g);
		v[x] = algorithm->Analyze(toAnalyze[x], toAnalyze[x]->getNextPlayer());
		g->copyMoveList(toAnalyze[x]);
		assert(v[x] != 0);
#if _PRINT_
		printf("Results:\n");
		for (returnValue *tmp = v[x]; tmp; tmp = tmp->next)
		{
			printf("(%1.3f)\t", tmp->getValue(g->getPlayerNum(p)));
			tmp->m->Print(1);
			//tmp->Print(0);
			printf("\n");
		}
#endif
//		toAnalyze[x]->deletePlayers();
		delete toAnalyze[x];
	}
	//return v;
}

//pthread_mutex_t mutex;
//pthread_mutex_init(&mutex, 0);
//pthread_mutex_destroy(&mutex);
//pthread_mutex_lock(&mutex);
//pthread_mutex_unlock(&mutex);

void iiMonteCarlo::doThreadedModels(GameState *g, Player *p, std::vector<returnValue*> &v, std::vector<double> &probs)
{
	iiGameState *iiState;
	Algorithm **algs;
	threadModel **tm;
	//returnValue **v;
	GameState **gameStates;
#ifndef __MAC__
	pthread_t *threads;
	threads = new pthread_t[numModels];
#else
	MPQueueID returnQ;
	MPTaskID *threads;
	threads = new MPTaskID[numModels];
	MPCreateQueue(&returnQ);
#endif
	
	v.resize(numModels);
	//v = new returnValue *[numModels];
	algs = new Algorithm *[numModels];
	tm = new threadModel *[numModels];
	gameStates = new GameState *[numModels];
	std::vector<int> modelQ;
	for (int x = 0; x < numModels; x++)
		modelQ.push_back(x);
	
	iiState = g->getiiGameState(true, g->getPlayerNum(p), player);

	if (!algorithm)
	{
		fprintf(stderr, "Error, algorithm is null, can't do models!\n");
		exit(0);
	}
	
	double probSum = 0;
	for (int x = 0; x < numModels; x++)
	{
		threads[x] = 0;
		v[x] = 0;
		double prob;
		gameStates[x] = iiState->getGameState(prob);
		probs.push_back(prob);
		probSum += prob;
		algs[x] = algorithm->clone();
		algs[x]->resetCounters(gameStates[x]);
		tm[x] = new threadModel();

		tm[x]->alg = algs[x];
		tm[x]->gs = gameStates[x];
		//tm[x]->v = &v[x];
#ifdef __MAC__
		tm[x]->returnQ = returnQ;
#endif
	}
	for (int x = 0; x < numModels; x++)
		probs[x]/=probSum;
	
	int numCPU = 1;
#ifdef __APPLE__
//	numCPU = MPProcessors();
#endif
	int numRunning = 0;
	std::deque<int> running;
	while ((modelQ.size() > 0) || (numRunning > 0))
	{
		while ((numRunning < numCPU) && (modelQ.size() > 0))
		{
			int next = modelQ.back();
			modelQ.pop_back();
			running.push_back(next);
			numRunning++;
#if _PRINT_
			printf("Starting up %d, %d now running\n", next, numRunning);
#endif
			
#ifndef __MAC__
			pthread_create(&threads[next], NULL, doThreadedModel, (void**)tm[next]);
#else
			MPCreateTask(doThreadedModel, (void*)tm[next], 512*1024, 0, NULL, NULL, 0, &threads[next]); 
#endif
		}

		int res;
		int waitFor = running.front();
		running.pop_front();
#ifndef __MAC__
		res = pthread_join(threads[waitFor], (void **)&v[waitFor]);
#else
		res = MPWaitOnQueue(returnQ, (void **)&v[waitFor], 0, 0, kDurationForever);
		if (v[waitFor] == 0)
		{ printf("Got NIL return\n");  }
#endif
#if _PRINT_
		printf("Got result from %d\n", waitFor);
		v[waitFor]->Print();
#endif
		numRunning--;
		if (res != 0)
		{
			printf("Error %d joining with %d\n", res, waitFor);
			exit(1);
		}
	}
	for (int x = 0; x < numModels; x++)
	{
		//printf("deleting alg %d: %p\n", x, algs[x]);	fflush(stdout);
		delete algs[x];
		algs[x] = 0;
		//printf("deleting tm %d\n", x);	fflush(stdout);
		delete tm[x];
		tm[x] = 0;
		//printf("deleting gamestate players\n");	fflush(stdout);
		//gameStates[x]->deletePlayers();
		//printf("deleting gamestate %d\n", x);	fflush(stdout);
		delete gameStates[x];
		gameStates[x] = 0;
	}
#ifdef __MAC__
	MPDeleteQueue(returnQ);
#endif
	delete [] tm;
	delete [] algs;
	delete [] gameStates;
	delete [] threads;
	delete iiState;
	//return v;
}

#ifdef __MAC__
OSStatus doThreadedModel(void *data)
#else
void *doThreadedModel(void *data)
#endif
{
	threadModel *m = (threadModel *)data;
	returnValue *val = m->alg->Analyze(m->gs, m->gs->getNextPlayer());
	//printf("Writing to: %p (%p)\n", (m->v), m);
#ifndef __MAC__
	pthread_exit((void *)val);
	return 0;
#else
	MPNotifyQueue(m->returnQ, (void *)val, 0, 0);
	MPExit(0);
	return 0;
#endif
}
// this function is required of other algorithms for the sake of monte-carlo
// experiments. 
// but if we write it...will it allow recursive monte-carlo experiments(?)

returnValue *iiMonteCarlo::Analyze(GameState *g, Player *p)
{
	std::vector<returnValue *> v;
	std::vector<double> probs;
	returnValue *best;

	// 1. procure and analyze each model

	if (usingThreads() && (algorithm->getSearchTimeLimit() == kMaxTimeLimit))
		doThreadedModels(g, p, v, probs);
	else
		doModels(g, p, v, probs);

	// 2. combine the results - only the algorithm knows how to do this.
	// 3. get the move with the highest expected results
	//	printf("Analyzing results\n");
	best = CombinedAnalyze(g, v, g->getPlayerNum(p), probs);
	//	best->Print(1);
	
	// 4. clean up memory
	for (int x = 0; x < numModels; x++)
		delete v[x];
	//delete [] v;

	return best;
}

returnValue *iiMonteCarlo::DispatchSearch(unsigned int depth, int cp, GameState *g)
{
	return 0;
}

//this code is currently not used and is probably buggy!
returnValue *iiMonteCarlo::CombinedAnalyze(GameState *g, std::vector<returnValue *> &v, int whichPlayer, std::vector<double> &probs)
{
	assert(false);
	Move *val[MAXMOVES];
	double valSum[MAXMOVES];
	int valCnt[MAXMOVES];
	int maxIndex = 0;
	double sum = 0;
	for (unsigned int x = 0; x < probs.size(); x++)
		sum += probs[x];
	if (!fequal(sum, 0))
	{
		for (unsigned int x = 0; x < probs.size(); x++)
			probs[x]/=sum;
#if _PRINT_
		printf("Normalizing probabilities!\n");
#endif
	}
	sum = 0;
	for (int x = 0; x < MAXMOVES; x++)
	{
		valSum[x] = 0;
		valCnt[x] = 0;
		val[x] = 0;
	}
	
	for (int x = 0; x < numModels; x++)
	{
#if _PRINT_
//		printf("Trying results from %d\n", x);
#endif
		returnValue *iter = v[x];
		// each of these is a list of possible moves...
		while (iter)
		{
			// find value in list so far, and add in current result...
			for (int y = 0; y < MAXMOVES; y++)
			{
				if (val[y] == 0)
				{
#if _PRINT_
					//printf("Putting in move loc %d\n", y);
#endif
					if (iter->m == 0)
						printf("Hey; we didn't get a move with our return value!\n");
					val[y] = iter->m->clone(g);
					if (val[y] == 0)
						printf("Hey; clone returned 0!\n");
					valCnt[y] = 1;
					valSum[y] = probs[x]*iter->getValue(whichPlayer);
					if (isnan(iter->getValue(whichPlayer)))
					{ printf("We got NAN!\n"); exit(1); }
					break;
				}
				if (val[y]->equals(iter->m))
				{
					valCnt[y]++;
					valSum[y] += probs[x]*iter->getValue(whichPlayer);
					if (isnan(iter->getValue(whichPlayer)))
					{ printf("We got NAN!\n"); iter->Print(0); exit(1); }
					break;
				}
			}
			iter = iter->next;
		}
	}
	returnValue *answer = 0;
//	for (int x = 0; x < MAXMOVES; x++)
//	{
//		if (val[x])
//			sum+=valSum[x]/valCnt[x];
//	}
	// now choose which move to make
	for (int x = 0; x < MAXMOVES; x++)
	{
		if (val[x])
		{
#if _PRINT_
			printf("===***creating combined value %1.2f for \n", valSum[x]/*/(double)valCnt[x]*/);
			val[x]->Print(0);
#endif
			returnValue *tmp = new minimaxval(1-(double)valSum[x]/*/(double)valCnt[x]*/, val[x]->clone(g));
			tmp->next = answer;
			answer = tmp;
		}
	}

#if _PRINT_
	printf("index\tcount\tsum\tmove\n");
	for (int x = 0; x < MAXMOVES; x++)
	{
		if (val[x])
		{
			printf("%d\t%d\t%1.2f\t", x, valCnt[x], valSum[x]);
			val[x]->Print(0);
			printf("\n");
		}
	}
#endif
	for (int x = 0; x < MAXMOVES; x++)
	{
		if (val[x] && (x != maxIndex))
			delete val[x];
	}
	return answer;
}

Move *iiMonteCarlo::Combine(GameState *g, std::vector<returnValue *> &v, int whichPlayer, std::vector<double> &probs)
{
	Move *val[MAXMOVES];
	double valSum[MAXMOVES];
	double valWeighted[MAXMOVES];
	double valMin[MAXMOVES];
	double S[MAXMOVES];
	double mean[MAXMOVES];
	int valCnt[MAXMOVES];
	int maxIndex = 0;

	double sum = 0;
	for (unsigned int x = 0; x < probs.size(); x++)
		sum += probs[x];
	if (!fequal(sum, 0))
	{
		for (unsigned int x = 0; x < probs.size(); x++)
			probs[x]/=sum;
#if _PRINT_
		printf("Normalizing probabilities!\n");
#endif
	}
	
	for (int x = 0; x < MAXMOVES; x++)
	{
		mean[x] = 0;
		S[x] = 0;
		valWeighted[x] = 0;
		valSum[x] = 0;
		valMin[x] = 0;
		valCnt[x] = 0;
		val[x] = 0;
	}
	
	for (int x = 0; x < numModels; x++)
	{
#if _PRINT_
//		printf("Trying results from %d\n", x);
#endif
		returnValue *iter = v[x];
		if (!iter)
			break;
		// each of these is a list of possible moves...
		while (iter)
		{
			// find value in list so far, and add in current result...
			for (int y = 0; y < MAXMOVES; y++)
			{
				if (val[y] == 0)
				{
#if _PRINT_
//					printf("%d Putting in move loc %d (%1.2f)\n", whichPlayer, y, iter->getValue(whichPlayer));
#endif
					if (iter->m == 0)
						printf("Hey; we didn't get a move with our return value!\n");
					val[y] = iter->m->clone(g);
					if (val[y] == 0)
						printf("Hey; clone returned 0!\n");
					valCnt[y] = 1;
					valSum[y] = iter->getValue(whichPlayer);
					valMin[y] = iter->getValue(whichPlayer);
					valWeighted[y] = probs[x]*iter->getValue(whichPlayer);
					
					double delta = iter->getValue(whichPlayer) - mean[y];
					mean[y] = mean[y] + delta/valCnt[y];
					S[y] = S[y] + delta*(iter->getValue(whichPlayer) - mean[y]);
						
					if (isnan(iter->getValue(whichPlayer)))
					{ printf("We got NAN!\n"); exit(1); }
						
					break;
				}
				if (val[y]->equals(iter->m))
				{
					valCnt[y]++;
					valSum[y] += iter->getValue(whichPlayer);
					valMin[y] = std::min(valMin[y], iter->getValue(whichPlayer));
					valWeighted[y] += probs[x]*iter->getValue(whichPlayer);

					double delta = iter->getValue(whichPlayer) - mean[y];
					mean[y] = mean[y] + delta/valCnt[y];
					S[y] = S[y] + delta*(iter->getValue(whichPlayer) - mean[y]);
					
					if (isnan(iter->getValue(whichPlayer)))
					{ printf("We got NAN!\n"); iter->Print(0); exit(1); }
					break;
				}
			}
			iter = iter->next;
		}
	}
	mt_random r;
	//r.srand(time(0));
	
	// make best move
	int ties = 1;
	// now choose which move to make
#if _PRINT_
	if (dr == kMaxWeighted)
		printf("Using kMaxWeighted\n");
#endif
	for (int x = 1; x < MAXMOVES; x++)
	{
		if (val[x])
		{
			if (((dr == kMaxWeighted) && (valWeighted[x] > valWeighted[maxIndex])) ||
				((dr == kMaxAverage) && (mean[x] > mean[maxIndex])) ||
				((dr == kMaxAvgVar) && (mean[x]-sqrt(S[x]/(valCnt[x]-1)) > mean[maxIndex]-sqrt(S[maxIndex]/(valCnt[maxIndex]-1)))) ||
				((dr == kMaxMinScore) && (valMin[x] > valMin[maxIndex])))
			{
				if (dr == kMaxWeighted)
					assert(valWeighted[x] > valWeighted[maxIndex]);
				ties = 1;
				maxIndex = x;
			}
//			else if (valSum[x]/valCnt[x] == valSum[maxIndex]/valCnt[maxIndex])
//				//else if (valMin[x] == valMin[maxIndex])
//			{
//				ties++;
//				if (r.rand_double() < (double)1/ties)
//					maxIndex = x;
//			}
		}
	}
		
#if _PRINT_
	//ai_debug("-=-=->\tindex\tcount\tsum\tmove");
	printf("index\tcount\tsum\tmin\tavg\tavg-s\tstd\twgt\n");
	for (int x = 0; x < MAXMOVES; x++)
	{
		if (val[x])
		{
			//char msg[255];
			//sprintf(msg, "-=-=->\t%d\t%d\t%1.2f\t", x, valCnt[x], valSum[x]);
			//ai_debug(msg);
			printf("%d\t%d\t%1.3f\t%1.3f\t%1.3f\t%1.3f\t%1.3f\t%1.4f", x, valCnt[x], valSum[x], valMin[x],
				   valSum[x]/valCnt[x], mean[x]-sqrt(S[x]/(valCnt[x]-1)), sqrt(S[x]/(valCnt[x]-1)), valWeighted[x]);
			val[x]->Print(0);//variance = S/(n - 1)
			if (x == maxIndex)
				printf("*");
			printf("\n");
		}
	}
#endif

	for (int x = 0; x < MAXMOVES; x++)
	{
		if (val[x] && (x != maxIndex))
			delete val[x];
	}
	//printf("Using %d for answer!\n", maxIndex);
	return val[maxIndex];
}

void iiMonteCarlo::GetGameStates(GameState *g, Player *p,
								 std::vector<GameState *> &states,
								 std::vector<double> &probs)
{
	probs.resize(0);
	states.resize(0);
	std::vector<GameState *> candidates;
	std::vector<double> pr;

	iiGameState *iiState;
	iiState = g->getiiGameState(true, g->getPlayerNum(p), player);
	iiState->getGameStates(numChoices, candidates, pr);

	NormalizeProbs(pr);
	while ((int)states.size() < numModels)
	{
		double val = rand.rand_double();
		for (unsigned int x = 0; x < pr.size(); x++)
		{
			if (fless(val, pr[x]))
			{
				if (candidates[x] == 0)
					continue;
				states.push_back(candidates[x]);
				probs.push_back(1.0/(double)numModels);
				candidates[x] = 0;
				break;
			}
		}
	}
	while (candidates.size() > 0)
	{
		GameState *gg = candidates.back();
		if (gg)
		{
			//gg->deletePlayers();
			delete gg;
		}
		candidates.pop_back();
	}
	delete iiState;
}

void iiMonteCarlo::NormalizeProbs(std::vector<double> &pr)
{
	double sum = 0, minval = 1, maxval = 0;
	for (unsigned int x = 0; x < pr.size(); x++)
	{
		if (fless(pr[x], minval))
			minval = pr[x];
		if (fgreater(pr[x], maxval))
			maxval = pr[x];
	}
	maxval -= minval;
//	// scale down to 0
	for (unsigned int x = 0; x < pr.size(); x++)
	{
		if (!fequal(maxval, 0))
		{
			pr[x] -= minval;
			pr[x] = 7.0*pr[x]/maxval+1.0;
		}
		sum += pr[x];
	}
	for (unsigned int x = 0; x < pr.size(); x++)
	{
		pr[x]/=sum;
		if (x != 0)
			pr[x] += pr[x-1];
	}
}

} // namespace hearts
