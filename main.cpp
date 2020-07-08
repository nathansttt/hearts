#include "Player.h"
#include "Game.h"
#include "Hearts.h"
#include "iiMonteCarlo.h"
#include "statistics.h"

using namespace hearts;

Player *GetPlayer(int playerType)
{
	//int sims[3] = {1000, 10000, 100000};
	//int worlds[3] = {20, 30, 50};

	double C = 0.4;
	switch (playerType)
	{
	case 0:
		{
			int sims = 10000;
			int worlds = 30;
			SimpleHeartsPlayer *p; iiMonteCarlo *a; UCT *b;
			p = new SafeSimpleHeartsPlayer(a = new iiMonteCarlo(b = new UCT(sims/worlds, C), worlds));
			p->setModelLevel(2);
			b->setPlayoutModule(new HeartsPlayout());
			a->setUseThreads(true);
			b->setEpsilonPlayout(0.1);
			return p;
		}
	case 1:
		{
			int sims = 10000;
			int worlds = 30;
			SimpleHeartsPlayer *p; iiMonteCarlo *a; UCT *b;
			p = new SafeSimpleHeartsPlayer(a = new iiMonteCarlo(b = new UCT(sims/worlds, C), worlds));
			p->setModelLevel(2);
			b->setPlayoutModule(new HeartsPlayout());
			a->setUseThreads(true);
			return p;
		}
	default:
		{
			int sims = 1000;
			int worlds = 20;
			SimpleHeartsPlayer *p; iiMonteCarlo *a; UCT *b;
			p = new SafeSimpleHeartsPlayer(a = new iiMonteCarlo(b = new UCT(sims/worlds, C), worlds));
			p->setModelLevel(2);
			b->setPlayoutModule(new HeartsPlayout());
			a->setUseThreads(true);
			return p;
		}

	}
	assert(false);
	return 0;
}

void PlayGame(statistics &s)
{
	srandom(time(NULL));
	HeartsGameState *g;
	g = new HeartsGameState(random());
	HeartsCardGame game(g);
	//game.setMaxPoints(1);
	int order[14][4] =
	{{0, 0, 0, 1},
	 {0, 0, 1, 0},
	 {0, 1, 0, 0},
	 {1, 0, 0, 0},
	 {0, 0, 1, 1},
	 {0, 1, 0, 1},
	 {0, 1, 1, 0},
	 {1, 0, 0, 1},
	 {1, 0, 1, 0},
	 {1, 1, 0, 0},
	 {1, 1, 1, 0},
	 {1, 1, 0, 1},
	 {1, 0, 1, 1},
	 {0, 1, 1, 1}
	};
	
	int rules=kQueenPenalty;
	if (0)
		rules |= kNoTrickBonus;
	if (0)
		rules |= kJackBonus;
	if (1)
	{
		rules |= kMustBreakHearts;
		if (1)
			rules |= kQueenBreaksHearts;
	}
	if (1)
		rules |= kDoPassCards;
	if (1)
		rules |= kNoQueenFirstTrick|kNoHeartsFirstTrick;
	
	rules |= kLeadClubs; //break;
	//rules |= kLead2Clubs; break;

	g->setRules(rules);

	int org = random()%14;
	for (int x = 0; x < 4; x++)
	{
		int player = order[org][x];
		SimpleHeartsPlayer *p; iiMonteCarlo *a; UCT *b;
		game.addPlayer(GetPlayer(order[org][x]));
	}

	g->setPassDir(0);	
	game.Play();

	s.collect(&game, g);

	// clean up memory
	for (int x = 0; x < 4; x++)
		delete g->getPlayer(x)->getAlgorithm();
	g->deletePlayers();
	delete g;
	g = 0;
}

int main(int argc, char **argv)
{
	statistics s;
	for (int x = 0; x < 100; x++)
	{
		PlayGame(s);
		s.save();
	}
}
