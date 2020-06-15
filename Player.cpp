#include "Player.h"
//#include "algorithmStates.h"

//HashTable *Player::historyTable = 0;

Player::Player(Algorithm *alg)
{
	algorithm = alg;
	minimumScore = 0;
	history = false;
//	randomize = false;
	historyTable = 0;
}

Player::~Player()
{
	if (history)
	{
		saveHistory();
		delete historyTable;
		historyTable = 0;
	}
}

void Player::setGameState(GameState *gs)
{
	g = gs;
}


Move *Player::Play()
{
	if (algorithm)
	{
		returnValue *v;
		algorithm->resetCounters(g);
		v = algorithm->Play(g, this);
		if ((history) && (historyTable))
		{
			saveHistory();
			//historyTable->Clear();
		}
		//printf("(%d) Search value returned: ", g->getPlayerNum(this));
		//v->Print(0);
		//printf("\n");
		Move *m = v->m;
		v->m = 0;
		delete v;
		return m;
	}
	// if we haven't been supplied with an algorithm...
	// do greedy move selection according to the cutoff eval
//	returnValue *v = getGreedyBestMove();
//	Move *m = v->m;
//	v->m = 0;
//	delete v;

	// no, we should use epsilon greedy for the above
	// instead, we just return the best move-ordered move

	Move holder;
	Move *m = g->getMoves();
	if (m->next != 0)
	{
		while (m)
		{
			Move *tmp = m->next;
			m->next = 0;
			m->dist = rankMove(m, g);
			holder.insert(m);
			m = tmp;
		}
		m = holder.next;
		holder.next = 0;
	}
	g->freeMove(m->next);
	m->next = 0;
	return m;
}

returnValue *Player::getGreedyBestMove()
{
	int me = g->getPlayerNum(this);
	Move *m = g->getMoves();
	Move *best = 0;
	double bestScore = 0;
	while (m)
	{
		double value;
		g->ApplyMove(m);

		if (g->Done() || (g->getNextPlayerNum() != me))
			value = cutoffEval(me);
		else {
			returnValue *v = getGreedyBestMove();
			value = v->getValue(me);
			delete v;
		}
		
		g->UndoMove(m);
		if ((value > bestScore) || (best == 0))
		{
			delete best;
			bestScore = value;
			best = m;
			m = m->next;
			best->next = 0;
		}
		else {
			Move *tmp = m;
			m = m->next;
			tmp->next = 0;
			g->freeMove(tmp);
			//delete tmp;
		}
	}
	if (best == 0)
		printf("No Best Move!!!!\n");
	return 0;//new minimaxval(bestScore, best);
}


const char *Player::getName()
{
	if (algorithm)
		return algorithm->getName();
	return "no algorithm";
}

void Player::setUseHistory(bool use)
{
	history = use;
	if ((history) && (!historyTable))
	{
		historyTable = new HashTable(10037);
		//loadHistory();
	}
}
/*
void Player::setRandomizeMoves(bool use)
{
	randomize = use;
}
*/
uint64_t Player::bestMove(const Move *m, GameState *gs)
{
	if ((!history) || (!algorithm))
		return 0;
//	if (randomize)
//		return random();
	uint64_t newval;

	int depth = algorithm->getCurrentSearchDepthLimit()-
				algorithm->getCurrentSearchDepth();
	historyState *hs = new historyState(gs->getMoveHash(m));
	historyState *lookup;
	if ((lookup = (historyState*)historyTable->IsIn(hs))!=0)
	{
		lookup->value += ((uint64_t)1<<depth);
		newval = lookup->value;
		delete hs;
	}
	else {
		historyTable->Add(hs);
		newval = hs->value = ((uint64_t)1<<depth);
	}
	return newval;
}

int Player::rankMove(Move *m, GameState *gs, bool optional)
{
	if (!history)
		return 0;
	historyState *hs = new historyState(gs->getMoveHash(m));
	historyState *lookup;
	if ((lookup = (historyState*)historyTable->IsIn(hs))!=0)
	{
		delete hs;
		return lookup->value;
	}
	delete hs;
	return 0;
}

void Player::loadHistory()
{
	return;
	FILE *f = fopen("ranks.moves", "r");
	if (f)
	{
		while (!feof(f))
		{
			long index;
			uint64_t value;
			if (fscanf(f, "%lu\t%llu\n", &index, &value) == EOF)
				break;

			historyState *hs = new historyState(index);
			hs->value = value;
			historyTable->Add(hs);
		}
		fclose(f);
	}

}

// do we want to merge this with the current file or just save?

// for now, just save.
void Player::saveHistory()
{
	historyState *hs;
	if (!historyTable)
		return;

	FILE *f = fopen("ranks.moves", "w");
	if (!f)
	{
		fprintf(stderr, "ERROR, could not save results!\n");
		return;
	}

	historyTable->iterReset();
	while (!historyTable->iterDone())
	{
		hs = (historyState*)historyTable->iterNext();
		fprintf(f, "%lu\t%llu\n", hs->hash, hs->value);
	}
	fflush(f);
	fclose(f);
	//exit(0);
}

double Player::cutoffEval(unsigned int who)
{
		return score(who);
}

void Player::cutoffEvals(std::vector<double> &values)
{
	values.resize(0);
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
		values.push_back(cutoffEval(x));
}
