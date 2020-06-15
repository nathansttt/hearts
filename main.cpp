#include "Player.h"
#include "Game.h"
#include "Hearts.h"
#include "iiMonteCarlo.h"

int main(int argc, char **argv)
{
	int sims[3] = {1000, 10000, 100000};
	int worlds[3] = {20, 30, 50};
	double C = 0.4;
	srandom(time(NULL));
	HeartsGameState *g;
	g = new HeartsGameState(random());
	HeartsCardGame game(g);
		
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

	int p1, p2, p3, p4;
	p1 = p2 = p3 = p4 = 0;
	p4 = 2;
	
	g->setRules(rules);
		
	SimpleHeartsPlayer *p; iiMonteCarlo *a; UCT *b;
	game.addPlayer(p = new SafeSimpleHeartsPlayer(a = new iiMonteCarlo(b = new UCT(sims[p1]/worlds[p1], C), worlds[p1])));
	p->setModelLevel(2);
	b->setPlayoutModule(new HeartsPlayout());
	a->setUseThreads(true);
	game.addPlayer(p = new SafeSimpleHeartsPlayer(a = new iiMonteCarlo(b = new UCT(sims[p2]/worlds[p2], C), worlds[p1])));
	p->setModelLevel(2);
	b->setPlayoutModule(new HeartsPlayout());
	a->setUseThreads(true);
	game.addPlayer(p = new SafeSimpleHeartsPlayer(a = new iiMonteCarlo(b = new UCT(sims[p3]/worlds[p3], C), worlds[p1])));
	p->setModelLevel(2);
	b->setPlayoutModule(new HeartsPlayout());
	a->setUseThreads(true);
	game.addPlayer(p = new SafeSimpleHeartsPlayer(a = new iiMonteCarlo(b = new UCT(sims[p4]/worlds[p4], C), worlds[p1])));
	p->setModelLevel(2);
	b->setPlayoutModule(new HeartsPlayout());
	a->setUseThreads(true);
	g->setPassDir(0);
		
	game.Play();
}
