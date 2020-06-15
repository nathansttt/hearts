#include "Game.h"

const bool logMoves = true;

Game::Game(GameState *gs)
{
	verbose = true;
	if (gs == 0)
		fprintf(stderr, "Error, gamestate passed into Game class is NULL\n");
	moveLimit = uINF;
	men = new Player*[MAXPLAYERS];
	for (unsigned int x = 0; x < MAXPLAYERS; x++)
	{
		men[x] = 0;
		gs->gameScore[x] = 0.0;
	}
	numPlayers = 0;
	theGame = gs;
	gs->setGame(this);
	
	FILE *f = fopen("moves.out", "a+");
	if (f) {
		fprintf(f, "*** new game started ***\n");
		fclose(f);
	}
	f = fopen("nodes.out", "a+");
	if (f) {
		fprintf(f, "*** new game started ***\n");
		fclose(f);
	}
}

Game::~Game()
{
	delete[] men;
	while (gameMoves.size() > 0)
	{
		delete gameMoves.back();
		gameMoves.pop_back();
	}
}

void Game::Reset()
{
	while (gameMoves.size() > 0)
	{
		delete gameMoves.back();
		gameMoves.pop_back();
	}
	for (int x = 0; x < numPlayers; x++)
	{
		theGame->gameScore[x] = 0;
	}
}

bool Game::Done()
{
	return theGame->Done();
}

int Game::Winner() const
{
	int best = 0;
	for (int x = 0; x < numPlayers; x++)
	{
		if (theGame->gameScore[x] > theGame->gameScore[best])
			best = x;
	}
//	return theGame->Winner();
	return best;
}

bool Game::PlayerWon(int who) const
{
	for (int x = 0; x < numPlayers; x++)
	{
		if (x == who)
			continue;
		if (theGame->gameScore[x] >= theGame->gameScore[who])
			return false;
	}
	return true;
}

int Game::Rank(int who)
{
	int better = 0;
	for (int x = 0; x < numPlayers; x++)
	{
		if (x == who)
			continue;
		if (theGame->gameScore[x] > theGame->gameScore[who])
			better++;
	}
	return better;
}


void Game::setMoveLimit(unsigned int val)
{
	moveLimit = val;
}

void Game::addPlayer(Player *p)
{
	men[numPlayers] = p;
	numPlayers++;
	theGame->addPlayer(p);
	p->setGameState(theGame);
	
	FILE *f = fopen("moves.out", "a+");
	if (f) {
		fprintf(f, "*** Players %d added (type = %s) ***\n", numPlayers, p->getName());
		fclose(f);
	}
	f = fopen("nodes.out", "a+");
	if (f) {
		fprintf(f, "*** Players %d added (type = %s) ***\n", numPlayers, p->getName());
		fclose(f);
	}
}

void Game::Print()
{
//	for (int x = 0; x < numPlayers; x++)
//		printf("Player %d score %1.0f\n", x+1, theGame->gameScore[x]);
	theGame->Print();
}

int Game::Play()
{
	unsigned int moves = 0;
	while (!Done())
	{
		moves++;
		doOnePlay();
		Print();
		if (moves >= moveLimit)
			break;
//		getchar();
	}
//	printf("Total Game Score:\n");
	if (moves >= moveLimit)
	{
		for (int x = 0; x < numPlayers; x++)
		{
			addScore(x, theGame->score(x));
			//		printf("Player %d:\t%1.0f\n", x+1, theGame->gameScore[x]);
		}
	}
//	for (int x = 0; x < numPlayers; x++)
//		printf("Player %d score %1.0f\n", x+1, theGame->gameScore[x]);
	return 0;
}

void Game::addScore(int who, double value)
{
	printf("Player %d got %1.0f\n", who, value);
	theGame->gameScore[who] += value;
	men[who]->addScore(value);
}

void Game::doOnePlay()
{
	Player *p;
	Move *m;
	int who;

	if (Done())
		return;
	
	p = theGame->getNextPlayer();
	who = theGame->getPlayerNum(p);
	if (p == 0)
	{
		printf("Next Player is null!\n");
		return;
	}
	//printf("Game State: ");
	//theGame->Print();
//	m = p->getMoves();
//	m->Print(1);
//	delete m;
	m = p->Play();
	if (m == 0)
	{
		printf("Next Move is null!!\n");
		m = theGame->getMoves();
		theGame->freeMove(m->next);
		m->next = 0;
	}

	theGame->ApplyMove(m);
	for (int x = 0; x < numPlayers; x++)
		theGame->getPlayer(x)->Played(who, m);

	//printf("%d's (team %d) move: ", who, theGame->getTeam(who));
	//m->Print(0);
	if (logMoves)
	{
		FILE *f = fopen("moves.out", "a+");
		if (f)
		{
			fprintf(f, "%d:%s\t", who, p->getName());
			m->Print(1, f);
			fclose(f);
		}
	}
	if (verbose)
	{
		printf("%d (%s) plays ", who, p->getName());
		m->Print(0);
		printf("\n");
	}

	//	printf("eval is ");
//	for (int x = 0; x < numPlayers; x++)
//			printf("%1.2f ", p->cutoffEval(x));
//	printf("\n");
	
	theGame->freeMove(m->next);
	m->next = 0;
	gameMoves.push_back(m);
//	//delete m;
//	m = 0;
	if (Done())
	{
		for (int x = 0; x < numPlayers; x++)
		{
			addScore(x, theGame->score(x));
			//		printf("Player %d:\t%1.0f\n", x+1, theGame->gameScore[x]);
		}
	}
}

double Game::score(int who)
{
	return theGame->gameScore[who];
}
