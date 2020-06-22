#include "Algorithm.h"
#include "GameState.h"
#include "Player.h"
#include <time.h>
#include <assert.h>
#include "Timer.h"

namespace hearts {

static void my_nothing_error(const char *fmt, ...) {}

//#define LOG if (1) printf
//#define LOG /*if (VERBOSE)*/ printf
#define LOG my_nothing_error
//const bool logNodes = true;

Algorithm::Algorithm()
{
	useDEPTHLIMIT = false;
	useNODELIMIT = false;
	useTIMELIMIT = false;
	useTHREADS = false;
	useAFTERSTATES = false;
	SEARCHNODELIMIT = uINF;
	SEARCHTIMELIMIT = kMaxTimeLimit;
	SEARCHDEPTHLIMIT = uINF;
	BFLIMIT = uINF;
	USEHASH = false;
	VERIFYHASH = false;
	VERBOSE = false;
	SAVE = false;
	PRUNING = true;
	randomize = false;
	ht = 0;
	searchDepth = 0;
	prevBest = 0;
	returnValueList = 0;
	totalNodesExpanded = 0;
	rand.srand(time((time_t*)0));
}

Algorithm::Algorithm(const Algorithm& a)
{
//	printf("Called algorithm copy constructor\n");
	who = 0; prevBest = 0;
	returnValueList = 0;
	iterDepth = a.iterDepth;
	USEHASH = a.USEHASH;
	VERIFYHASH = a.VERIFYHASH;
	if (USEHASH)
		ht = new HashTable(49979693);//452930477);//1257787);//131071
	else
		ht = 0;
	searchDepth = a.searchDepth;
	totalMoves = a.totalMoves; // the depth when we start our search
	nodesExpanded = a.nodesExpanded;
	totalNodesExpanded = a.totalNodesExpanded;
	useDEPTHLIMIT = a.useDEPTHLIMIT;
	useNODELIMIT = a.useNODELIMIT;
	useTIMELIMIT = a.useTIMELIMIT;
	useTHREADS = a.useTHREADS;
	useAFTERSTATES = a.useAFTERSTATES;
	SEARCHDEPTHLIMIT = a.SEARCHDEPTHLIMIT;
	startDepth = a.startDepth;
	SEARCHTIMELIMIT = a.SEARCHTIMELIMIT;
	SEARCHNODELIMIT = a.SEARCHNODELIMIT;
	BFLIMIT = a.BFLIMIT;
	VERBOSE = a.VERBOSE;
	PRUNING = a.PRUNING;
	t1 = a.t1;
	t2 = a.t2;
}


Algorithm::~Algorithm()
{
	delete ht;
	delete prevBest;
	while (returnValueList)
	{
		returnValue *tmp = returnValueList;
		returnValueList = returnValueList->next;
		tmp->next = 0;
		delete tmp;
	}
}

returnValue *Algorithm::Play(GameState *g, Player *p)
{
	int val;
	returnValue *currMove=0, *oldMove=0;
	who = p;
	std::vector<int> iterationNodes;
	
	int cp=-1;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++) {
		if (g->getPlayer(x) == p)
			cp = x;
	}
	//assert(cp != -1);
	// if I'm not part of the game, I can play on behalf of that player?
	if (cp == -1)
		cp = g->getNextPlayerNum();

	val = setupNextIteration(g, p);
	LOG("Starting at depth %d, %d\n", iterDepth, val);
	printf("depth %2d: ", iterDepth);
	while (!searchExpired(g))
	{
		Timer t;
		t.StartTimer();
		currMove = DispatchSearch(0, cp, g);
		t.EndTimer();
		printf("%6.1f seconds elapsed %10lu nodes expanded\t", t.GetElapsedTime(), getNodesExpanded());
		if (currMove)
		{
			LOG("Done\n");
			//printf("%1.2f}\t  ", currMove->getValue(g->getPlayerNum(who)));
			currMove->Print(1);
			if (searchExpired(g))
			{
				delete currMove;
				break;
			}
			if (oldMove != 0)
			{
				delete oldMove;
			}
			oldMove = currMove;
			currMove = 0;
		}
		val = setupNextIteration(g, p);
		printf("depth %2d: ", val);
		fflush(stdout);
		if ((unsigned int)iterDepth > p->getMaxDepth())
		{
			LOG("Stopping because of max depth\n");
			break;
		}
		// lets stop if we expect our next iteration
		// to put us over the top
		if (useNODELIMIT && getNodesExpanded()*4 > SEARCHNODELIMIT)
		{
			printf("Special: Stopping because nodes is rapidly approaching search limit\n");
			break;
		}
		iterationNodes.push_back(getNodesExpanded());
		int lastArrayIndex = iterationNodes.size()-1;
		if ((iterationNodes.size() > 2) &&
			((iterationNodes[lastArrayIndex] - iterationNodes[lastArrayIndex-1]) ==
			 (iterationNodes[lastArrayIndex-1] - iterationNodes[lastArrayIndex-2])))
		{
			printf("Special: Stopping because not making any progress\n");
			break;
		}
	}

	if ((oldMove == 0) || (oldMove->m == 0))
	{
		Move *m = getMoves(g, g->getPlayer(cp));
		delete m->next;
		m->next = 0;
		delete oldMove;
		oldMove = new returnValue(m);
	}

	LOG("%ld nodes expanded\n", getNodesExpanded());
	return oldMove;
}

returnValue *Algorithm::Analyze(GameState *g, Player *p)
{
	who = p;
	returnValue *currMove=0, *oldMove=0;
	int val;
	
	int cp=0, np = g->getNumPlayers();
	for (int x = 0; x < np; x++) {
		if (g->getPlayer(x) == p)
			cp = x;
	}

	val = setupNextIteration(g, p);
	if (searchExpired(g))
		LOG("Hey, search already expired!\n");
	LOG("Starting at depth %d, %d\n", iterDepth, val);
	while (!searchExpired(g))
	{
		LOG("Searching %s depth %d, %d\n", getName(), iterDepth, val);
		currMove = analyzeHelper(0, cp, g);
		//currMove = analyzeHelper(0, np, cp, op, g, 0, 0);
		if (currMove)
		{
			//currMove->Print(1);
			if (searchExpired(g))
			{
				if (oldMove == 0)
					LOG("Search Expired, and we have no other moves!\n");
				delete currMove;
				break;
			}
			if (oldMove != 0)
			{
				delete oldMove;
			}
			oldMove = currMove;
			currMove = 0;
		}
		if ((unsigned int)iterDepth > p->getMaxDepth())
			break;
		if (getNodesExpanded()*3 > SEARCHNODELIMIT)
			break;
		val = setupNextIteration(g, p);
	}
	if ((oldMove == 0) || (oldMove->m == 0))
	{
		Move *m = getMoves(g, g->getPlayer(cp));
		delete m->next;
		m->next = 0;
		delete oldMove;
		oldMove = new returnValue(m);
	}
	return oldMove;
}

returnValue *Algorithm::analyzeHelper(unsigned int depth, unsigned int cp, GameState *g)
{
	returnValue *tmp, *answer=0;

	Move *m, *t;

	t = m = getMoves(g, g->getPlayer(cp));

	while (t) {
		m = t; 
		ApplyMove(g, m);
		tmp = DispatchSearch(depth+1, g->getNextPlayerNum(), g);
		UndoMove(g, m);
		if (tmp)
		{
			t = t->next; m->next = tmp->m; tmp->m = m;
			tmp->next = answer;
			answer = tmp;
			tmp = 0;
		}
		else {
			t = t->next;
			m->next = 0;
			delete m;
		}
	}

	return answer;
}

void Algorithm::ApplyMove(GameState *g, Move *m)
{
	nodesExpanded++;
	totalNodesExpanded++;
	searchDepth++;
	g->ApplyMove(m);
}

void Algorithm::UndoMove(GameState *g, Move *m)
{
	searchDepth--;
	g->UndoMove(m);
}

void Algorithm::setUseHashTable(bool use)
{
	USEHASH = use;
	if ((ht == 0) && (use))
		ht = new HashTable(49979693);//HashTable(1257787);//10037//131071
}

void Algorithm::setVerifyHashTable(bool verify)
{
	VERIFYHASH = verify;
}

bool Algorithm::searchExpired(GameState *g)
{
	if (g->Done())
	{
		LOG("Stopped because game's over\n");
		return true;
	}
	if ((useAFTERSTATES && (g->getPreviousPlayer() == who)) || (!useAFTERSTATES))
	{
		if ((useDEPTHLIMIT) && (getCurrentSearchDepth() > SEARCHDEPTHLIMIT))
		{
			LOG("Stopped with current search depth %d\n", getCurrentSearchDepth());
			return true;
		}
	}
	if ((iterDepth != -1) && (getCurrentSearchDepth() >= (unsigned int)iterDepth))
	{
		LOG("Stopped with current search depth %d iter = %d\n", getCurrentSearchDepth(),iterDepth);
		return true;
	}
	if ((useDEPTHLIMIT) && ((unsigned)iterDepth > SEARCHDEPTHLIMIT))
	{
		LOG("Stopped because iterDepth (%d) is bigger than limit (%d)\n", iterDepth, SEARCHDEPTHLIMIT);
		return true;
	}
	if (timeExpired())
	{
		LOG("Time's up...we quit.\n");
		return true;
	}
	if ((useNODELIMIT) && (nodesExpanded >= SEARCHNODELIMIT))
	{
		LOG("Stopped with %ld nodes expanded\n", nodesExpanded);
		return true;
	}
	return false;
}

Move *Algorithm::getRandomMove(GameState *g)
{
	return g->getRandomMove();
}

Move *Algorithm::getMoves(GameState *g, Player *p)
{
	Move holder;
	Move *t, *m = g->getMoves();
	unsigned int count = 0;
	if (m->next != 0)
	{
		while (m)
		{
			assert(m->player == g->getPlayerNum(p));
			Move *tmp = m->next;
			m->next = 0;
			if (randomize)
				m->dist = rand.rand_long();
			else
				m->dist = p->rankMove(m, g);
			holder.insert(m);
			count++;
			m = tmp;
		}
		m = holder.next;
		holder.next = 0;
	}
	t = m;
	if (count <= BFLIMIT)
		return m;
	for (unsigned int x = 0; t && (x < BFLIMIT-1) && (t->next); x++, t = t->next)
	{
		//LOG("Keeping d=%d\n", t->dist);
	}
	while (t->next)
	{
		Move *tmp = t->next;
		t->next = tmp->next;
		tmp->next = 0;
		//LOG("Dropping d=%d\n", tmp->dist);
		g->freeMove(tmp);
		//delete tmp;
		/*
		delete t->next;
		t->next = 0;*/
	}
//	LOG("Length is %d\n", m->length());
	return m;
}

int Algorithm::setupNextIteration(GameState *g, Player *p)
{
	if ((iterDepth != -1) && (!useTHREADS))
	{
		if (1)
		{
			logNodes();
		}
	}

	searchDepth = 0;
	iterDepth = p->getNextDepth(iterDepth);
	clearHashTable(g);
	return iterDepth;
}

void Algorithm::logNodes()
{
	FILE *t = fopen("nodes.out", "a+");
	if (t)
	{
		fprintf(t, "%s\t%d\t%ld\n", getName(), iterDepth, nodesExpanded);
		fclose(t);
	}	
}

int Algorithm::getIterationSearchLimit()
{
	return iterDepth;
}

void Algorithm::setSearchDepthLimit(int val)
{
	useDEPTHLIMIT = true;
	SEARCHDEPTHLIMIT = val;
}

void Algorithm::setSearchTimeLimit(double val)
{
	useTIMELIMIT = true;
	SEARCHTIMELIMIT = val;
}

void Algorithm::setSearchNodeLimit(unsigned long val)
{
	useNODELIMIT = true;
	SEARCHNODELIMIT = val;
}

void Algorithm::setBranchingFactorLimit(unsigned long val)
{
	BFLIMIT = val;
}

void Algorithm::clearHashTable(GameState *g)
{
	if (ht)
	{
		//ht->PrintStats();
		ht->iterReset();
		while (!ht->iterDone())
		{
			HashState *hs = (HashState*)ht->iterNext();
			if ((hs->ret) && (hs->ret->m))
			{
				delete hs->ret->m;
				//g->freeMove(hs->ret->m);
				hs->ret->m = 0;
			}
		}
		ht->Clear();
	}
}

const HashState *Algorithm::checkHashTable(HashState *s)
{
	const HashState *st;

	if (s == 0)
		return 0;

	if (ht == 0)
		return 0;

	if ((st = (HashState*)ht->IsIn(s)) != 0)
		return st;//st->ret->clone();

	return 0;
}

void Algorithm::addHashTable(HashState *s, returnValue *r)
{
	//HashState *st=0;
	if ((s == 0) || (ht == 0))
	{
		// dangerous because these points still exist in calling function...
		delete s;
		s = 0;
		delete r;
		r = 0;
		return;
	}
	/*
		st = ht->IsIn(s); // eliminate this check to speed things up
		if (st)
		{
		printf("Trying to add something already in there!!!\n");
		exit(0);
		}
	*/
	if (s->ret != 0)
	{
		printf("Trying to add a return value where I shouldn't...\n");
		exit(0);
	}
	s->ret = r;
	ht->Add(s);
}

HashState *Algorithm::getHashState(GameState *g, HashState *mem)
{
	if (getCurrentSearchDepth() > (unsigned int)iterDepth-2*g->getNumPlayers())
		return 0;
	if (getCurrentSearchDepth() < 2*g->getNumPlayers())
		return 0;
	if (ht && ht->getNumElts() > 2000000)
		return 0;
	return g->getHashState(mem);
}

void Algorithm::resetCounters(GameState *g)
{
	nodesExpanded = 0;
	//totalMoves = g->getDepth();
	searchDepth = 0;
	iterDepth = -1;
	delete prevBest;
	prevBest = 0;
	clearHashTable(g);
	// this should probably be before we clear the hash table, but not right now.
	t1 = ((double)clock()/CLOCKS_PER_SEC);
}

bool Algorithm::timeExpired()
{
	return (useTIMELIMIT)&&(timeElapsed() > SEARCHTIMELIMIT);
}

void Algorithm::setReturnValueType(returnValue *v)
{
	delete returnValueList;
	returnValueList = v;
	v->next = 0;
}

returnValue *Algorithm::getNewReturnValue(GameState *g)
{
	if (returnValueList->next == 0)
		return returnValueList->clone(g);
	
	returnValue *t = returnValueList;
	returnValueList = returnValueList->next;
	t->next = 0;
	t->m = 0;
	return t;
}

void Algorithm::freeReturnValue(returnValue *v)
{
	if (v)
	{
		returnValue *trav;
		for (trav = v; trav->next; trav = trav->next)
		{ }
		trav->next = returnValueList;
		returnValueList = v;
	}
}

} // namespace hearts
