#include "Hearts.h"
//#include "mathUtil.h"
#include "fpUtil.h"
#include "HeartsGameHistories.h"

namespace hearts {

	inline bool clamp(double val, double minv, double maxv)
	{
	    return (val > maxv)?maxv:(val<minv?minv:val);
	}
	
uint64_t getFullDeck()
{
	return ((uint64_t)0x1FFF1FFF<<32)+0x1FFF1FFF;
}

void PrintCard(card c, FILE *f)
{
	const char *s[] = {"S", "D", "C", "H"};
	const char *r[] = {" A", " K", " Q", " J", "10", " 9", " 8",
		" 7", " 6", " 5", " 4", " 3", " 2", "", "", ""};

	if (c == -1) {
#ifndef __NO_ANSI__
		if (f == stdout)
			fprintf(f, "%c[%d;%dm", 27, 3, 35);
#endif
		fprintf(f, " GO");
#ifndef __NO_ANSI__
		if (f == stdout)
			fprintf(f, "%c[%d;%dm", 27, 0, 0);
#endif
		return;
	}

#ifndef __NO_ANSI__
	if (f != stdout)
#endif
		fprintf(f, "%s%s", r[Deck::getrank(c)], s[Deck::getsuit(c)]);
#ifndef __NO_ANSI__
	else {
		fprintf(f, "%c[%d;%dm", 27, (Deck::getsuit(c)/2), 34-((Deck::getsuit(c))%2)*3);
		fprintf(f, "%s%s", r[Deck::getrank(c)], s[Deck::getsuit(c)]);
		fprintf(f, "%c[%d;%dm", 27, 0, 0);
	}
#endif
}

extern char *theName;

int HeartsCardGame::Play()
{
//	GameHistories gh;
//	gh.SetSavePath("/Users/nathanst/Desktop/");
//	gh.SetUserName("MrData");
//	gh.StartNewGame();
	int dir[4] = { kLeftDir, kRightDir, kAcrossDir, kHold };
	int numGames = 0;
	bool done = false;
	while (!done)
	{
		theGame->Reset();
		theGame->Print(1);
		//((CardGameState*)theGame)->DealCards();
		//printf("---Setting %d to go first!---\n", (numGames+4)%numPlayers);
		((HeartsGameState*)theGame)->setPassDir(dir[numGames%4]);
		//((HeartsGameState*)theGame)->doCardPass();
		((CardGameState*)theGame)->setFirstPlayer((numGames+4)%numPlayers);
		started = true;
		// this plays an entire game...
		Game::Play();
#if 1 //_PRINT_
		for (int x = 0; x < numPlayers; x++)
			printf("Player %d score %1.0f [%s]\n", x+1, theGame->gameScore[x], theGame->getPlayer(x)->getName());
#endif
		for (int x = 0; x < numPlayers; x++)
		{
			if (theGame->gameScore[x] >= getMaxPoints())
			{
				done = true;
			}
		}
		numGames++;
		//gh.AddToLastGame((HeartsGameState*)theGame);
	}
	return 0;
}

void HeartsCardGame::doOnePlay()
{
	static int numplays = 0;

	if ((numplays%numPlayers == 0) && (numplays != 0) && (numplays%52 != 0))
		((HeartsGameState*)theGame)->waitEndTrick();
	CardGame::doOnePlay();
	numplays++;
	if (numplays%52 == 0)
		((HeartsGameState*)theGame)->waitEndTrick();
}

int HeartsCardGame::Winner() const
{
	int best = 0;
	for (int x = 0; x < numPlayers; x++)
	{
		if (theGame->gameScore[x] < theGame->gameScore[best])
			best = x;
	}
	return best;
}

bool HeartsCardGame::Winner(int who)
{
	for (int x = 0; x < numPlayers; x++)
	{
		if (x == who)
			continue;
		if (theGame->gameScore[x] < theGame->gameScore[who])
			return false;
	}
	return true;
}

int HeartsCardGame::Rank(int who)
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


void HeartsGameState::Reset(int SD)
{
	for (unsigned int x = 0; x < MAXPLAYERS; x++)
		passes[x].resize(0);
	numCardsPassed = 0;
	currPlr = 0;
	CardGameState::Reset(SD);
}

CardGameState *HeartsGameState::create()
{
	return new HeartsGameState();
}

void HeartsGameState::Print(int val) const
{
	if (passDir != kHold)
	{
		printf("%d cards on the table to be passed\n", numCardsPassed);
		printf("Passing cards:\n");
		for (unsigned int y = 0; y < getNumPlayers(); y++)
		{
			printf("%d: ", y);
			for (unsigned int x = 0; x < passes[y].size(); x++)
			{
				PrintCard(passes[y][x]);
				printf(" ");
			}
			printf("\n");
		}
	}
	else {
		printf("Passdir is HOLD! (%d)\n", passDir);
	}
	CardGameState::Print(val);
}

void HeartsGameState::ApplyMove(Move *move)
{
//	PrintCard(((CardMove*)move)->c);
//	printf(" applied %d\n", move->player);
//	printf("passdir is %d\n", passDir);
	int next = getNextPlayerNum();
	if ((passDir == kHold) || (numCardsPassed == (int)getNumPlayers()*3))//(passes[next].size() == 3)
	{
		CardGameState::ApplyMove(move);
	}
	else {
		CardMove *cm = (CardMove*)move;
		// handle pass!
		assert(cm->player == next);
		passes[next].push_back(cm->c);
		assert(passes[next].size() <= 3);
		numCardsPassed++;
		UntakeCard(next, cm->c);
		
		if (passes[next].size() == 3)
			currPlr = (currPlr+1)%getNumPlayers();
		if (numCardsPassed != (int)getNumPlayers()*3)
			return;
		int numP = getNumPlayers();
		
		for (int x = 0; x < numP; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				TakeCard((x+numP+passDir)%numP, passes[x][y]);
			}
		}
		if ((rules&kLead2Clubs) || (rules&kLeadClubs))
		{
			for (int x = 0; x < numP; x++)
			{
				if (cards[x].has(CLUBS, TWO))
				{
					currPlr = x;
					break;
				}
			}
		}
		else {
			//printf("Last player was %d; setting next to %d\n", next, firstPlayer);
			currPlr = firstPlayer; //(next+1)%getNumPlayers();
		}
	}
}

void HeartsGameState::UndoMove(Move *move)
{
//	PrintCard(((CardMove*)move)->c);
//	printf(" undone %d\n", move->player);
//	printf("passdir is %d\n", passDir);
	if ((passDir == kHold) || (getTrick(0)->curr != 0))
	{
		CardGameState::UndoMove(move);
	}
	else {
		int numP = getNumPlayers();
		// actually undo giving the new cards -- if everyone is done passing		
		if (numCardsPassed == (int)getNumPlayers()*3)
		{
			for (int x = 0; x < numP; x++)
			{
				for (int y = 0; y < 3; y++)
				{
					UntakeCard((x+numP+passDir)%numP, passes[x][y]);
				}
			}
		}

		CardMove *cm = (CardMove*)move;
		// actually give the new cards -- if everyone is done passing
		currPlr = move->player;
		assert(passes[currPlr].back() == cm->c);
		TakeCard(move->player, cm->c);
		passes[currPlr].pop_back();
		numCardsPassed--;
//		if (passes[currPlr].size() == 0)
//			currPlr = (currPlr-1+getNumPlayers())%getNumPlayers();
	}
//	printf("=====================UNDO MOVE!=======================\n");
//	CardMove *cm = (CardMove*)move;
//	printf("%d ", cm->player);
//	PrintCard(cm->c);
//	printf("---\n");
//	Print(1);
//	printf("<<<<<<<<<<<<<<<<<<<<<UNDO MOVE!>>>>>>>>>>>>>>>>>>>>>>>\n");
}

void HeartsGameState::DealCards()
{
	
#ifdef _PRINT_
	printf("Initializing tricks...\n");
#endif
	
	
#ifdef _PRINT_
	printf("Dealing Cards!\n");
#endif
	if (numCards != -1)
	{
		for (int c = TWO; c >= ACE; c--)
			for (int s = SPADES; s < HEARTS; s++)
			{
				if (d.count() == numCards*getNumPlayers())
				{
					c = ACE;
					s = HEARTS;
					break;
				}
				d.clear(Deck::getcard(s, c));
				allplayed.set(Deck::getcard(s, c));
			}
	}
	
	if (numCards == -1) {
		numCards = 0;
		while (d.count() >= numPlayers) {
			numCards++;
			for (unsigned int x = 0; x < numPlayers; x++) {
				card c;
				c = d.Deal();
				TakeCard(x, c);
#ifdef _PRINT_
				printf("%d gets : %d", x+1, c);
				PrintCard(c);
				printf("\n");
#endif
			}
		}
	}
	else {
		for (int x = 0; x < numCards; x++)
			for (unsigned int y = 0; y < numPlayers; y++)
			{
				card c = d.Deal();
				TakeCard(y, c);
			}
	}
		while (d.count() > 0)
			allplayed.set(d.Deal());
		
#ifdef _PRINT_
		Print();
#endif
		SEED+=81;
}

bool HeartsGameState::IsLegalMove(Move *m)
{
	bool result = false;
	Move *n = getAllMoves();
	for (Move *tt = n; tt; tt = tt->next)
		if (m->equals(tt))
			result = true;
	freeMove(n);
	return result;
}

Move *HeartsGameState::getAllMoves()
{
	int me = getNextPlayerNum();
	const Trick *ct = getCurrTrick();
	CardMove *ret=0;
	
	// passing cards - pass anything
	if ((rules&kDoPassCards) && (passDir != kHold) && (numCardsPassed < (int)getNumPlayers()*3))
	{
		for (int x = 0; x < 64; x++)
		{
			if (cards[me].has(x))
			{
				CardMove *cm = (CardMove*)getNewMove();
				cm->init(x, kSloughCard, me, ret);
				ret = cm;
			}
		}
		if (ret == 0) // first pass was so high that there aren't 2 other cards to pass
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(cards[me].getRandomCard(), kSloughCard, me, ret);
			ret = cm;
		}
		return ret;
	}

	// two of clubs leads rule
 	if (rules&kLead2Clubs && (currTrick == 0) && (ct->curr == 0))
	{
		if (cards[me].has(Deck::getcard(CLUBS, TWO)))
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(Deck::getcard(CLUBS, TWO), kLeadCard, me, 0);
			return cm;
			//return new CardMove(Deck::getcard(CLUBS, TWO), me);
		}
		else {
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(Deck::getcard(CLUBS, THREE), kLeadCard, me, 0);
			return cm;
		}
	}
	
	// following in suit
	if (((ct->curr != 0) && (cards[me].hasSuit(Deck::getsuit(ct->play[0])))) ||
		((currTrick == 0) && (ct->curr == 0) && (rules&kLeadClubs)))
	{
		int ledSuit;
		int specialCard;
		
		if ((ct->curr == 0) && (rules&kLeadClubs))
			ledSuit = CLUBS;
		else
			ledSuit = Deck::getsuit(ct->play[0]);
		
		// we can't skip generation of special cards (QS/JD)
		if ((ledSuit == SPADES) && (rules&kQueenPenalty))
			specialCard = cards[me].has(Deck::getcard(SPADES, QUEEN))?
			Deck::getcard(SPADES, QUEEN):-1;
		else if ((ledSuit == DIAMONDS) && (rules&kJackBonus))
			specialCard = cards[me].has(Deck::getcard(DIAMONDS, JACK))?
			Deck::getcard(DIAMONDS, JACK):-1;
		else
			specialCard = -1;
		
		uint32_t theSuit = cards[me].getSuit(ledSuit);
		uint32_t allSuit = allplayed.getSuit(ledSuit);
		int curr = Deck::getcard(ledSuit, 0);
		while (theSuit)
		{
			if (theSuit&1)
			{
				CardMove *cm = (CardMove*)getNewMove();
				cm->init(curr, kFollowCard, me, ret);
				ret = cm;//new CardMove(curr, me, ret);
			}
			curr++;
			theSuit>>=1;
			allSuit>>=1;
		}
		
		if (ret)
		{
			return ret;
		}
	}
	
	// playing any card except special suit (unless broken)
	for (int y = 0; y < 4; y++)
	{
		if ((ct->curr != 0) || /* (ct->curr == 0) - now implicit */
			(y != HEARTS) ||
			(!(rules&kMustBreakHearts)) ||
			((y == HEARTS) && ((allplayed.getSuit(HEARTS)) ||
							   ((rules&kQueenBreaksHearts) &&
								(allplayed.has(Deck::getcard(SPADES, QUEEN)))))))
		{
			if ((currTrick == 0) && (y == HEARTS) && (rules&kNoHeartsFirstTrick))
				continue;
			int specialCard;
			// we can't skip generation of special cards (QS/JD)
			if ((y == SPADES) && (rules&kQueenPenalty))
				specialCard = cards[me].has(Deck::getcard(SPADES, QUEEN))?
				Deck::getcard(SPADES, QUEEN):-1;
			else if ((y == DIAMONDS) && (rules&kJackBonus))
				specialCard = cards[me].has(Deck::getcard(DIAMONDS, JACK))?
				Deck::getcard(DIAMONDS, JACK):-1;
			else
				specialCard = -1;
			
			uint32_t theSuit = cards[me].getSuit(y);
			uint32_t allSuit = allplayed.getSuit(y);
			int curr = Deck::getcard(y, 0);
			while (theSuit)
			{
				if (theSuit&1)
				{
					if ((currTrick != 0) || (!(rules&kNoQueenFirstTrick)) ||
						(curr != Deck::getcard(SPADES, QUEEN)))
					{
						CardMove *cm = (CardMove*)getNewMove();
						cm->init(curr, (ct->curr==0)?kLeadCard:kSloughCard, me, ret);
						ret = cm;//new CardMove(curr, me, ret);
						//ret = new CardMove(curr, me, ret);
					}
				}
				curr++;
				theSuit>>=1;
				allSuit>>=1;			
			}
		}
	}
	if (ret)
	{
		return ret;
	}
	
	uint32_t theSuit = cards[me].getSuit(HEARTS);
	uint32_t allSuit = allplayed.getSuit(HEARTS);
	int curr = Deck::getcard(HEARTS, 0);
	while (theSuit)
	{
		if (theSuit&1)
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(curr, (ct->curr==0)?kLeadCard:kSloughCard, me, ret);
			ret = cm;//new CardMove(curr, me, ret);
			//ret = new CardMove(curr, me, ret);
		}
		curr++;
		theSuit>>=1;
		allSuit>>=1;			
	}
	
	return ret;
}

Move *HeartsGameState::getMoves()
{
	int me = getNextPlayerNum();
	const Trick *ct = getCurrTrick();
	CardMove *ret=0;

	// passing cards - pass anything
	if ((rules&kDoPassCards) && (passDir != kHold) && (numCardsPassed < (int)getNumPlayers()*3))
	{
//		printf("Only generating pass moves!\n");
		int start = 0;
		if (passes[me].size() > 0)
		{
			start = passes[me].back()+1;
		}
		for (int x = start; x < 64; x++)
		{
			if (cards[me].has(x))
			{
				CardMove *cm = (CardMove*)getNewMove();
				cm->init(x, kSloughCard, me, ret);
				ret = cm;
			}
		}
		//printf("%d Found %d moves\n", me, ret->length());
		if (passes[me].size() == 0) // remove first two moves
		{
			assert(ret->next->next != 0);
//			printf("Excluding: ");
//			PrintCard(ret->c);
//			PrintCard(((CardMove*)ret->next)->c);
//			printf("\n");
			Move *m = ret;
			ret = (CardMove*)ret->next->next;
			m->next->next = 0;
			freeMove(m);
		}
		else if (passes[me].size() == 1) // remove first move
		{
			assert(ret->next != 0);
//			printf("Excluding: ");
//			PrintCard(ret->c);
//			printf("\n");
			Move *m = ret;
			ret = (CardMove*)ret->next;
			m->next = 0;
			freeMove(m);
		}
		assert(ret != 0);
//		if (ret == 0) // first pass was so high that there aren't 2 other cards to pass
//		{
//			CardMove *cm = (CardMove*)getNewMove();
//			for (int x = 0; x < 64; x++)
//				if (cards[me].has(x)
//			cm->init(cards[me].getRandomCard(), me, ret);
//			ret = cm;
//		}
		return ret;
	}
	
	// two of clubs leads rule
 	if (rules&kLead2Clubs && (currTrick == 0) && (ct->curr == 0))
	{
		if (cards[me].has(Deck::getcard(CLUBS, TWO)))
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(Deck::getcard(CLUBS, TWO), kLeadCard, me, 0);
			return cm;
			//return new CardMove(Deck::getcard(CLUBS, TWO), me);
		}
		else {
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(Deck::getcard(CLUBS, THREE), kLeadCard, me, 0);
			return cm;
		}
	}

	// following in suit
	if (((ct->curr != 0) && (cards[me].hasSuit(Deck::getsuit(ct->play[0])))) ||
		((currTrick == 0) && (ct->curr == 0) && (rules&kLeadClubs)))
	{
		int ledSuit;
		int specialCard;

		if ((ct->curr == 0) && (rules&kLeadClubs))
			ledSuit = CLUBS;
		else
			ledSuit = Deck::getsuit(ct->play[0]);

		// we can't skip generation of special cards (QS/JD)
		if ((ledSuit == SPADES) && (rules&kQueenPenalty))
			specialCard = cards[me].has(Deck::getcard(SPADES, QUEEN))?
			                    Deck::getcard(SPADES, QUEEN):-1;
		else if ((ledSuit == DIAMONDS) && (rules&kJackBonus))
			specialCard = cards[me].has(Deck::getcard(DIAMONDS, JACK))?
                          Deck::getcard(DIAMONDS, JACK):-1;
		else
			specialCard = -1;

		uint32_t theSuit = cards[me].getSuit(ledSuit);
		uint32_t allSuit = allplayed.getSuit(ledSuit);
		int curr = Deck::getcard(ledSuit, 0);
		while (theSuit)
		{
			if (theSuit&1)
			{
				CardMove *cm = (CardMove*)getNewMove();
				cm->init(curr, kFollowCard, me, ret);
				ret = cm;//new CardMove(curr, me, ret);
				while (((theSuit&1) || (allSuit&1)) && (curr != specialCard)
							 && (curr != specialCard -1))
				{ theSuit>>=1; allSuit>>=1; curr++; }
			}
			curr++;
			theSuit>>=1;
			allSuit>>=1;
		}

		if (ret)
		{
			return ret;
		}
	}

	// playing any card except special suit (unless broken)
	for (int y = 0; y < 4; y++)
	{
		if ((ct->curr != 0) || /* (ct->curr == 0) - now implicit */
				(y != HEARTS) ||
				(!(rules&kMustBreakHearts)) ||
				((y == HEARTS) && ((allplayed.getSuit(HEARTS)) ||
								   ((rules&kQueenBreaksHearts) &&
									(allplayed.has(Deck::getcard(SPADES, QUEEN)))))))
		{
			if ((currTrick == 0) && (y == HEARTS) && (rules&kNoHeartsFirstTrick))
				continue;
			int specialCard;
			// we can't skip generation of special cards (QS/JD)
			if ((y == SPADES) && (rules&kQueenPenalty))
				specialCard = cards[me].has(Deck::getcard(SPADES, QUEEN))?
				                            Deck::getcard(SPADES, QUEEN):-1;
			else if ((y == DIAMONDS) && (rules&kJackBonus))
				specialCard = cards[me].has(Deck::getcard(DIAMONDS, JACK))?
				                            Deck::getcard(DIAMONDS, JACK):-1;
			else
				specialCard = -1;

			uint32_t theSuit = cards[me].getSuit(y);
			uint32_t allSuit = allplayed.getSuit(y);
			int curr = Deck::getcard(y, 0);
			while (theSuit)
			{
				if (theSuit&1)
				{
					if ((currTrick != 0) || (!(rules&kNoQueenFirstTrick)) ||
							(curr != Deck::getcard(SPADES, QUEEN)))
					{
						CardMove *cm = (CardMove*)getNewMove();
						cm->init(curr, (ct->curr==0)?kLeadCard:kSloughCard, me, ret);
						ret = cm;//new CardMove(curr, me, ret);
						//ret = new CardMove(curr, me, ret);
					}
					while (((theSuit&1) || (allSuit&1)) && (curr != specialCard)
								 && (curr != specialCard -1))
					{ theSuit>>=1; allSuit>>=1; curr++; }
				}
				curr++;
				theSuit>>=1;
				allSuit>>=1;			
			}
		}
	}
	if (ret)
	{
		return ret;
	}

	uint32_t theSuit = cards[me].getSuit(HEARTS);
	uint32_t allSuit = allplayed.getSuit(HEARTS);
	int curr = Deck::getcard(HEARTS, 0);
	while (theSuit)
	{
		if (theSuit&1)
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(curr, (ct->curr==0)?kLeadCard:kSloughCard, me, ret);
			ret = cm;//new CardMove(curr, me, ret);
			//ret = new CardMove(curr, me, ret);
			while ((theSuit&1) || (allSuit&1))
			{ theSuit>>=1; allSuit>>=1; curr++; }
		}
		curr++;
		theSuit>>=1;
		allSuit>>=1;			
	}
	
	return ret;
}

Move *HeartsGameState::getRandomMove()
{
	if (rules != kQueenPenalty)
		return GameState::getRandomMove();
	
	int me = getNextPlayerNum();
	//Player *who = getNextPlayer();
	const Trick *tr = getCurrTrick();
	//CardMove m;
	Move *ret=0;
	
	// following
	if (tr->curr != 0)
	{
		cardContext stage;
		int myPlay;
		if (cards[me].hasSuit(Deck::getsuit(tr->play[0])))
		{
			myPlay = cards[me].getRandomCard(Deck::getsuit(tr->play[0]));
			stage = kFollowCard;
		}
		else {
			myPlay = cards[me].getRandomCard();
			stage = kSloughCard;
		}
		CardMove *cm = (CardMove*)getNewMove();
		assert(myPlay != -1);
		cm->init(myPlay, stage, me, ret);
		return cm;
	}
	
	// leading -- special is broken or all we have
	if ((allplayed.getSuit(special)) ||
		(cards[me].suitCount(special) == cards[me].count()))
	{
		int myPlay = cards[me].getRandomCard();
		CardMove *cm = (CardMove*)getNewMove();
		cm->init(myPlay, kLeadCard, me, ret);
		assert(myPlay != -1);
		return cm;
	}
	else {
		int myPlay;
		while ((myPlay = cards[me].getRandomCard()) == special)
		{ }
		CardMove *cm = (CardMove*)getNewMove();
		cm->init(myPlay, kLeadCard, me, ret);
		assert(myPlay != -1);
		return cm;
	}
	assert(false);
	return 0;
}

// slow!! this can be **much** faster...?
double HeartsGameState::score(int who) const
{
	// if shooting is allowed and hearts are points...
	if ((!(rules&kNoShooting)) && (!(rules&kHeartsArentPoints)))
	{
		for (unsigned int x = 0; x < numPlayers; x++)
		{
			if ((taken[x].suitCount(HEARTS) == allplayed.suitCount(HEARTS)) &&
					((!(rules&kQueenPenalty)) || taken[x].has(Deck::getcard(SPADES, QUEEN))) &&
					((!(rules&kShootingNeedsJack)) || taken[who].has(Deck::getcard(DIAMONDS, JACK))))
			{
				//printf("Player %d shot!\n", x);
				if ((int)x == who)
				{
					if ((rules&kJackBonus) && (taken[x].has(Deck::getcard(DIAMONDS, JACK))))
						return -10;
					return 0;
					//return 0+(((rules&kJackBonus)&&(taken[x].has(Deck::getcard(DIAMONDS, JACK))))?(+10):(0));
				}
				else {
					int score = 13+allplayed.suitCount(HEARTS);
					if ((rules&kJackBonus) && (taken[x].has(Deck::getcard(DIAMONDS, JACK))))
						return score-10;
					return score;
					
					//return -13-allplayed.suitCount(HEARTS)+(((rules&kJackBonus)&&(taken[who].has(Deck::getcard(DIAMONDS, JACK))))?(10):(0));
				}
			}
		}
	}
	int scores = 0;
	if (!(rules&kHeartsArentPoints))
		scores += taken[who].suitCount(HEARTS);
	if (rules&kQueenPenalty)
		scores += 13*taken[who].has(Deck::getcard(SPADES, QUEEN));
	if (rules&kJackBonus)
		scores -= 10*taken[who].has(Deck::getcard(DIAMONDS, JACK));
	// only incorporate this into the score when half the cards have been played out
	if ((rules&kNoTrickBonus) && (allplayed.count() == 52))
		scores -= 5*(taken[who].empty());
	return scores;
}

int HeartsGameState::score(const Trick *ct) const
{
	int points=0;
	for (int x = 0; x < ct->curr; x++)
	{
		if (!(rules&kHeartsArentPoints))
			if (Deck::getsuit(ct->play[x]) == HEARTS)
				points++;
		if ((ct->play[x] == Deck::getcard(SPADES, QUEEN)) && (rules&kQueenPenalty))
			points+=13;
		if ((ct->play[x] == Deck::getcard(DIAMONDS, JACK)) && (rules&kJackBonus))
			points-=10;
	}
	return points;
}

/*
maxnval *HeartsGameState::checkEndGame()
{
	// only run check on trick boundaries
	if (t[currTrick].curr != 0)
		return 0;

	bool getRest = true;
	bool noPoints = false;

	// start simple - if current player has 1 suit and no one else does,
	// he gets the rest of the points
	for (int x = 0; x < 4; x++) // suits
	{
		if (cards[currPlr].getSuit(x))
		{
			for (int z = 0; z < getNumPlayers(); z++)
			{
				if ((z != currPlr) && (cards[z].getSuit(x)))
				{
					getRest = false;
					break;
				}
			}
		}
	}

	if ((allplayed.getSuit(HEARTS) == 0x1FFF) && allplayed.has(Deck::getcard(SPADES, QUEEN)))
		noPoints = true;
	
    if (getRest || noPoints)
    {
		maxnval *v = new maxnval();
		for (int x = 0; x < getNumPlayers(); x++)
		{
		v->eval[x] = -score(x);
		v->eval[currPlr] -= cards[x].suitCount(3)+13*cards[x].has(2);
		}
		//printf("$$$$$$$$$$$$$ Found End Game Saver $$$$$$$$$$$$$\n";
		return v;
    }
	
	return 0;
}
*/
void HeartsGameState::setPassDir(int dir)
{
	passDir = dir;
	if (!(rules&kDoPassCards))
		passDir = kHold;
	if (passDir != kHold)
		currPlr = 0;
}

//void HeartsGameState::doCardPass()
//{
//	int numP = getNumPlayers();
//	if (passDir == kHold)
//		return;
//	//Print(1);
//	for (int x = 0; x < numP; x++)
//	{
//		((HeartsCardPlayer*)getPlayer(x))->selectPassCards(passDir, passes[x][0], passes[x][1], passes[x][2]);
//		for (int y = 0; y < 3; y++)
//		{
//			//printf("%d Untaking %d\n", x, passes[x][y]);
//			UntakeCard(x, passes[x][y]);
//		}
//	}
//	for (int x = 0; x < numP; x++)
//	{
//		if (strcmp(getPlayer((x+numP+passDir)%numP)->getName(), "human") == 0)
//			printf("Passed: ");
//		if (strcmp(getPlayer(x)->getName(), "human") == 0)
//			printf("Passing: ");
//		for (int y = 0; y < 3; y++)
//		{
//			if (strcmp(getPlayer((x+numP+passDir)%numP)->getName(), "human") == 0)
//			{
//				PrintCard(passes[x][y]);
//				printf(" and ");
//			}
//			if (strcmp(getPlayer(x)->getName(), "human") == 0)
//			{
//				PrintCard(passes[x][y]);
//				printf(" and ");
//			}
//			//printf("Giving %d %d\n", (x+numP+passDir)%numP, passes[x][y]);
//			TakeCard((x+numP+passDir)%numP, passes[x][y]);
//		}
//		if (strcmp(getPlayer((x+numP+passDir)%numP)->getName(), "human") == 0)
//			printf("\n");
//		if (strcmp(getPlayer(x)->getName(), "human") == 0)
//			printf("\n");
//	}
//	waitEndTrick();
//}

int HeartsGameState::Winner() const
{
	int takenPoints[MAXPLAYERS];
	int best = 0;
	for (unsigned int x = 0; x < MAXPLAYERS; x++)
		takenPoints[x] = 0;
	for (int x = 0; x < numCards; x++)
		takenPoints[t[x].Winner()] += score(&t[x]);
	for (unsigned int x = 1; x < getNumPlayers(); x++)
		if (takenPoints[x] < takenPoints[best])
			best = x;
	return best;
}

void HeartsGameState::setFirstPlayer(int first)
{
	if ((rules&kLead2Clubs) || (rules&kLeadClubs))
	{
		for (unsigned int x = 0; x < getNumPlayers(); x++)
		{
			if (cards[x].has(Deck::getcard(CLUBS, TWO)))
			{
				CardGameState::setFirstPlayer(x);
				if (passDir != kHold)
				{
					currPlr = 0;
				}
				return;
			}
		}
		for (unsigned int x = 0; x < getNumPlayers(); x++)
		{
			if (cards[x].has(Deck::getcard(CLUBS, THREE)))
			{
				CardGameState::setFirstPlayer(x);
				if (passDir != kHold)
				{
					currPlr = 0;
				}
				return;
			}
		}
	}
	else {
		firstPlayer = first;
//		if ((rules&kDoPassCards) && (getPassDir() != kHold))
//			currPlr = 0;
//		else
		currPlr = first;
		//CardGameState::setFirstPlayer(first);
	}
	if (passDir != kHold)
	{
		currPlr = 0;
	}
}

void HeartsGameState::waitEndTrick()
{
}

iiGameState *HeartsGameState::getiiGameState(bool consistent, int who, Player *playerModel)
{
	//printf("Getting iiGameSate for %d; curr = %d\n", who, currPlr);
	if (playerModel == 0)
		playerModel = (Player*)getPlayer(who);
	iiHeartsState *cs = (iiHeartsState*)playerModel->getiiModel();
	if (cs == 0)
		cs = new iiHeartsState();
	assert(cs != 0);
	cs->init(trump, special, numCards, numPlayers, SEED++);

	cs->master = playerModel;
	cs->originalGame = this;
	cs->rules = rules;
	cs->numCardsPassed = numCardsPassed;
	cs->firstPlayer = firstPlayer;
	
	if (t[0].curr != 0)
	{
		// set first player the same as the old game
		// then when we play out the game, the players will match!
		cs->currPlr = t[0].player[0];
	}
	else {
		cs->currPlr = currPlr;
	}

	if (consistent)
		cs->hasCards(original[who].getHand(), who);
	else
	{
		int num = original[who].count(); // only give us half of our cards
		Deck da(original[who]);
		for (int x = 0; x < num-1; x++)
			cs->hasCard(da.Deal(), who);
	}
	for (int x = 0; x < numCards+1; x++)
	{
		for (int y = 0; y < t[x].curr; y++)
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(t[x].play[y], (y==0)?kLeadCard:
					 ((Deck::getsuit(t[x].play[0])==t[x].play[y])?kFollowCard:kSloughCard),
					 t[x].player[y]);
			cs->Played(cm);
			freeMove(cm);
		}
	}
	
	cs->passDir = passDir;
	if ((rules&kDoPassCards) && (passDir != kHold))
	{
		int numP = getNumPlayers();
		//printf("Handling Pass!\n");
		if (numCardsPassed == (int)getNumPlayers()*3) // done passing
		{
			for (int y = 0; y < 3; y++)
			{
				//printf("%d knows %d has %d\n", who, (who+numP+passDir)%numP, passes[who][y]);
				cs->hasCard(passes[who][y], (who+numP+passDir)%numP);
				cs->passes[who].push_back(passes[who][y]);
				cs->passes[(who+numP-passDir)%numP].push_back(passes[(who+numP-passDir)%numP][y]);
			}
		}
		else {
			//printf("%d has %d cards in the pass pile\n", who, (int)passes[who].size());
			// just add our cards to the pass info
			for (unsigned int y = 0; y < passes[who].size(); y++)
			{
				cs->passes[who].push_back(passes[who][y]);
				//printf("Putting into passes [%d]: ", who);
				//PrintCard(passes[who][y]);
				//printf("\n");
			}
		}
	}
	else {

	}
	return cs;

	// wrong place to do this!!!
//	if (cards[who].has(SPADES, QUEEN))
//		return ret;
//	if (allplayed.has(SPADES, QUEEN))
//		return ret;
//	const Trick *t = getCurrTrick();
//	for (int x = 0; x < t->curr; x++)
//		if (t->play[x] == Deck::getcard(SPADES, QUEEN))
//			return ret;
//	// assign the queen of spades with equal prob. to those who haven't
//	// lead spades and might still have spades
//	if (rand.rand_double() > 0.10) // 10% chance of going to anyone legal
//	{
//		bool lead[4] = {false, false, false, false};
//		bool out[4] =  {false, false, false, false};
//		int numOut = 0;
//		int numLead = 0;
//		for (int x = 0; x < getCurrTrickNum(); x++)
//		{
//			const Trick *t = getTrick(x);
//			if (t->curr == 0) break;
//			if (Deck::getsuit(t->play[0]) == SPADES)
//			{
//				lead[t->player[0]] = true;
//				for (int y = 1; y < t->curr; y++)
//				{
//					if (Deck::getsuit(t->play[y]) != SPADES)
//					{
//						out[t->player[y]] = true;
//					}
//				}
//			}
//		}
//		for (int x = 0; x < 4; x++)
//		{
//			if (x == who)
//				continue;
//			if (out[x])
//				numOut++;
//			else if (lead[x])
//				numLead++;
//		}
//		int cnt = 4-numOut-numLead;
//		if (cnt != 0) // if everyone lead, we're ignorant
//		{
//			int assign = rand.ranged_long(0, cnt);
//			for (int x = 0; x < 4; x++)
//			{
//				if ((!out[x]) && (!lead[x]) && (x != who))
//				{
//					if (assign == 0)
//					{
//						ret->hasCard(Deck::getcard(SPADES, QUEEN), x);
//						printf("Assigning queen to %d\n", x);
//						break;
//					}
//					assign--;
//				}
//			}
//		}
//	}
//	return ret;
}

void HeartsGameState::MeasureProperties()
{
	SimpleHeartsPlayer shp(0);
	Move *t = getMoves();
	double bias = 0;
	int count = 0;
	double theMin = 26;
	double theMax = 0;
	for (Move *m = t; m; m = m->next)
	{
		count++;
		ApplyMove(m);
		maxnval *v = shp.DoRandomPlayout(this, &shp, 1.0);
		UndoMove(m);
		//printf("Move: %1.4f\n", v->getValue(getNextPlayerNum()));
		bias+=v->getValue(getNextPlayerNum());
		if (v->getValue(getNextPlayerNum()) < theMin)
			theMin = v->getValue(getNextPlayerNum());
		if (v->getValue(getNextPlayerNum()) > theMax)
			theMax = v->getValue(getNextPlayerNum());
		delete v;
	}
	FILE *f = fopen("/Users/nathanst/Desktop/bias_corr.txt", "a+");
	
//	printf("BIAS:\t%f\n", bias/count);
	fprintf(f, "%1.4f\t", bias/count);
	if (theMax-theMin > 8)
	{
//		printf("CORRELATION: -1\n");
		fprintf(f, "-1\n");
	}
	else {
//		printf("CORRELATION: 1\n");
		fprintf(f, "1\n");
	}
	fclose(f);
	freeMove(t);
}

SimpleHeartsPlayer::SimpleHeartsPlayer(Algorithm *alg)
:CardPlayer(alg)
{
	modelLevel = 0;
}

const char *SimpleHeartsPlayer::getName()
{
	static char name[255];
	iiGameState *igs = getiiModel();
	sprintf(name, "Simple(%s,%s)", igs->GetName(), algorithm->getName());
	delete igs;
	return name;
}


iiGameState *SimpleHeartsPlayer::getiiModel()
{
	if (modelLevel == 2)
		return new advancedIIHeartsState();
	else if (modelLevel == 1)
		return new simpleIIHeartsState();
	else if (modelLevel == 3)
	{
		iiHeartsState *i = new iiHeartsState();
		i->advancedModeling = true;
		return i;
	}
	return new iiHeartsState();
}


Player *SimpleHeartsPlayer::clone() const
{
	return new SimpleHeartsPlayer(algorithm);
	//	SimpleHeartsPlayer *shp = new SimpleHeartsPlayer(algorithm, localPolicy);
//	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
//	{
//		shp->setScoreModel(getScoreModel(x), x);
//	}
//	return shp;
}


void SimpleHeartsPlayer::selectPassCards(int dir, card &a, card &b, card &c)
{
	CardGameState *cgs = (CardGameState *)g;
	int me = g->getPlayerNum(this);
	int drop[3] = {-1, -1, -1};
	int curr = 0;
	
	// ugly hack
	if (strcmp(g->getPlayer(me)->getName(), "human") == 0)
	{
		g->Print(0);
		printf("Select cards to pass %s:\n", (dir==kLeftDir)?"left":
			   ((dir==kRightDir)?"right":"across"));
		int which = 0;
		int choices[13];
		for (int x = 0; x < 64; x++)
			if (cgs->cards[me].has(x))
			{
				printf(" (%d)", which);
				PrintCard(x);
				choices[which] = x;
				which++;
			}
		//cgs->cards[me].Print();
		printf("\n\nEnter card numbers (eg. 1 5 6): \n");
		int aa, bb, cc;
		scanf("%d %d %d", &aa, &bb, &cc);
		a = choices[aa];
		b = choices[bb];
		c = choices[cc];
		return;
	}
	
	
	// need to check shoot potential first
	
	// get rid of high spades
	if (cgs->cards[me].suitCount(SPADES) < 4)
	{
		if (cgs->cards[me].has(Deck::getcard(SPADES, ACE)))
		{ drop[curr] = Deck::getcard(SPADES, ACE); curr++; }
		if (cgs->cards[me].has(Deck::getcard(SPADES, KING)))
		{ drop[curr] = Deck::getcard(SPADES, KING); curr++; }
		if (cgs->cards[me].has(Deck::getcard(SPADES, QUEEN)))
		{ drop[curr] = Deck::getcard(SPADES, QUEEN); curr++; }
	}
    
	// diamond suit with high cards not backed by low cards
	if (((cgs->cards[me].getSuit(DIAMONDS)&0x1FC0) == 0) && (curr < 3))
	{
		for (int x = Deck::getcard(DIAMONDS, ACE); x <= Deck::getcard(DIAMONDS, TWO); x++)
		{
			if (cgs->cards[me].has(x) && (curr < 3))
			{
				drop[curr] = x; curr++;
			}
		}
	}
	// club suit with high cards not backed by low cards
	if (((cgs->cards[me].getSuit(CLUBS)&0x1FC0) == 0) && (curr < 3))
	{
		for (int x = Deck::getcard(CLUBS, ACE); x <= Deck::getcard(CLUBS, TWO); x++)
		{
			if (cgs->cards[me].has(x) && (curr < 3))
			{
				drop[curr] = x; curr++;
			}
		}
	}
	if (curr < 3)
	{
		bool stop = false;
		int cnt = cgs->cards[me].suitCount(HEARTS);
		for (int x = Deck::getcard(HEARTS, ACE); (x <= Deck::getcard(HEARTS, TWO))&&(cnt>1); x++)
		{
			if (cgs->cards[me].has(x))
			{
				cnt--;
				if (!stop)
					stop = true;
				else if (curr < 3) {
					drop[curr] = x; curr++;
				}
			}
			else
				cnt--;
		}
	}
	for (int x = Deck::getcard(SPADES, ACE); x <= Deck::getcard(SPADES, TWO); x++)
	{
		if (curr >= 3)
			break;
		for (int y = 1; y < 3; y++)
		{
			if (curr >= 3)
				break;
			if (cgs->cards[me].has(Deck::getcard(y, x)) && 
				(drop[0] != Deck::getcard(y, x)) && (drop[1] != Deck::getcard(y, x)))
			{
				drop[curr] = Deck::getcard(y, x);
				curr++;
			}
		}
	}
	for (int x = Deck::getcard(CLUBS, ACE); x <= Deck::getcard(CLUBS, TWO); x++)
	{
		if (curr >= 3)
			break;
		for (int y = 1; y < 3; y++)
		{
			if (curr >= 3)
				break;
			if (cgs->cards[me].has(Deck::getcard(y, x)) && 
				(drop[0] != Deck::getcard(y, x)) && (drop[1] != Deck::getcard(y, x)))
			{
				drop[curr] = Deck::getcard(y, x);
				curr++;
			}
		}
	}
	for (int x = Deck::getcard(DIAMONDS, ACE); x <= Deck::getcard(DIAMONDS, TWO); x++)
	{
		if (curr >= 3)
			break;
		for (int y = 1; y < 3; y++)
		{
			if (curr >= 3)
				break;
			if (cgs->cards[me].has(Deck::getcard(y, x)) && 
				(drop[0] != Deck::getcard(y, x)) && (drop[1] != Deck::getcard(y, x)))
			{
				drop[curr] = Deck::getcard(y, x);
				curr++;
			}
		}
	}
	a = drop[0];
	b = drop[1];
	c = drop[2];
}

//tScoreUtility SimpleHeartsPlayer::getScoreModel(int who) const
//{
//	if (who == -1)
//		return localPolicy;
//	if ((int)scorePolicy.size() <= who)
//		return localPolicy;
//	return scorePolicy[who];
//}
//
//void SimpleHeartsPlayer::setScoreModel(tScoreUtility u, int who)
//{
//	if (who == -1)
//		localPolicy = u;
//	else {
//		if ((int)scorePolicy.size() <= who)
//			scorePolicy.resize(who, localPolicy);
//		scorePolicy[who] = u;
//	}
//}

double SimpleHeartsPlayer::score(unsigned int who)
{
	return g->score(who);
}

//double SimpleHeartsPlayer::getBestOtherScore(std::vector<int> &scores, int who)
//{
//	int best = 0;
//	if (who == 0)
//		best = 1;
//	for (unsigned int x = best+1; x < scores.size(); x++)
//	{
//		if ((int)x == who)
//			continue;
//		if (scores[x] < scores[best])
//			best = x;
//	}
//	return scores[best];
//}
//
//double SimpleHeartsPlayer::getBestOtherScore(std::vector<int> &scores,
//											 std::vector<int> &gscores, int who)
//{
//	int best = 0;
//	if (who == 0)
//		best = 1;
//	for (unsigned int x = best+1; x < scores.size(); x++)
//	{
//		if ((int)x == who)
//			continue;
//		if (scores[x]+gscores[x] < scores[best]+gscores[best])
//			best = x;
//	}
//	return scores[best]+gscores[best];
//}
//
//double SimpleHeartsPlayer::getWorstOtherScore(std::vector<int> &scores, int who)
//{
//	int best = 0;
//	if (who == 0)
//		best = 1;
//	for (unsigned int x = best+1; x < scores.size(); x++)
//	{
//		if ((int)x == who)
//			continue;
//		if (scores[x] > scores[best])
//			best = x;
//	}
//	return scores[best];
//}
//
//double SimpleHeartsPlayer::getWorstOtherScore(std::vector<int> &scores,
//											 std::vector<int> &gscores, int who)
//{
//	int best = 0;
//	if (who == 0)
//		best = 1;
//	for (unsigned int x = best+1; x < scores.size(); x++)
//	{
//		if ((int)x == who)
//			continue;
//		if (scores[x]+gscores[x] > scores[best]+gscores[best])
//			best = x;
//	}
//	return scores[best]+gscores[best];
//}

double SimpleHeartsPlayer::cutoffEval(unsigned int who)
{
//	if (simple)
//		return (26.0-g->score(who))/3.0;
	double sum = 0;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
		sum+=(26-g->score(x));
	return (26-g->score(who))/sum;
}

maxnval *SimpleHeartsPlayer::DoRandomPlayout(GameState *gs, Player *p, double epsilon)
{
	//bool shootDanger = false;
	std::vector<Move *> moves;
	CardGameState *cgs = (CardGameState *)gs;
	// first pass, don't think about shooting...
	while (!cgs->Done())
	{
//		if (canShoot(cgs))
//		{
//			moves.push_back(DoShootPlay(cgs));
//			shootDanger = true;
//		}
//		else if (shootDanger == true)
//		{
//			moves.push_back(cgs->getRandomMove());
//		}
//		else
		moves.push_back(DoMinPlay(cgs, epsilon));
		gs->ApplyMove(moves.back());
	}
	maxnval *v = new maxnval();
	double sum = 0;
	for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
		sum+=(26-cgs->score(x));
	for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
		v->eval[x] = (26-cgs->score(x));///sum;

	while (moves.size() > 0)
	{
		gs->UndoMove(moves.back());
		gs->freeMove(moves.back());
		moves.pop_back();
	}
	return v;
}

Move *SimpleHeartsPlayer::DoMinPlay(CardGameState *cgs, double epsilon)
{
	if (0||(rand.rand_double() < epsilon)) // x% chance of a rand move
	{
		return cgs->getRandomMove();
	}
		
	const Trick *trick = cgs->getCurrTrick();
	card winningCard = trick->WinningCard();
//	int winner = trick->Winner();
	mt_random rand;
	
	Move *m = cgs->getMoves();
	Move *best = m;
	if (winningCard == -1) // leading, play random
	{
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (rand.rand_double() <= 1/count)
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	else if (Deck::getsuit(winningCard) == Deck::getsuit(((CardMove*)best)->c)) // follow randomly
	{
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (rand.rand_double() <= 1/count)
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	else { // sloughing -- high to low
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (Deck::getcard(SPADES, QUEEN) == ((CardMove*)t)->c)
			{
				best = t;
				break;
			}

			if (rand.rand_double() <= 1/count)
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	assert(0);
	return 0;
}

bool SimpleHeartsPlayer::canShoot(CardGameState *cgs)
{
	int me = g->getNextPlayerNum();
	if (cgs->taken[me].getSuit(HEARTS) == 0) // taken hearts already
		return false;
	if (cgs->allplayed.getSuit(HEARTS) != cgs->taken[me].getSuit(HEARTS))
		return false;
	if (cgs->allplayed.has(SPADES, QUEEN) && (!cgs->taken[me].has(SPADES, QUEEN)))
		return false;
	if (cgs->taken[me].suitCount(HEARTS) < 6) // taken at least 6 hearts
		return false;
	return true;
}

Move *SimpleHeartsPlayer::DoShootPlay(CardGameState *cgs)
{
	int me = g->getNextPlayerNum();
	const Trick *trick = cgs->getCurrTrick();
	card winningCard = trick->WinningCard();
	
	Move *m = cgs->getMoves();
	Move *best = m;
	
	for (Move *t = m->next; t; t = t->next)
	{
		card choice = ((CardMove*)t)->c;
		card bestCard = ((CardMove*)best)->c;
		if ((winningCard == -1) && (choice <= 2))
		{
			best = t;
			break;
		}
		
		// first go for longest suit
		if (cgs->original[me].suitCount(Deck::getsuit(choice)) >
			cgs->original[me].suitCount(Deck::getsuit(bestCard)))
			best = t;
		else if (cgs->original[me].suitCount(Deck::getsuit(choice)) ==
				 cgs->original[me].suitCount(Deck::getsuit(bestCard)))
		{
			if (Deck::getrank(choice) < // then go for highest card
				Deck::getrank(bestCard))
				best = t;
		}
	}
	best = best->clone(cgs);
	cgs->freeMove(m);
	return best;
}	

double GlobalHeartsPlayer::cutoffEval(unsigned int who)
{
	double sum = 0;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
		sum+=g->getGame()->score(x)+g->score(x);
	if (sum == 0)
		return 1;
	return 1-(g->getGame()->score(who)+g->score(who))/sum;
}

Player *GlobalHeartsPlayer::clone() const
{
	return new GlobalHeartsPlayer(algorithm);
}

const char *GlobalHeartsPlayer::getName()
{
	static char name[255];
	iiGameState *igs = getiiModel();
	sprintf(name, "GlobalHearts(%s,%s)", igs->GetName(), algorithm->getName());
	delete igs;
	return name;
}

double GlobalHeartsPlayer2::cutoffEval(unsigned int who)
{
//	double minVal = 100;
//	bool useParanoid = true;
//	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
//		if (g->getGame()->score(x) < g->getGame()->score(who))
//			useParanoid = false;
//	if (useParanoid)
//	{
//		if (who = g->getPlayerNum(this))
//			return 1.0-(double)g->score(who)/26.0;
//		return (double)g->score(who)/26.0;
//	}
	double sum = 0;
	int lowest = 0;
	if (who == 0)
		lowest = 1;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
	{
		if (x != who)
		{
			sum+=g->score(x);
			if (g->getGame()->score(x) < g->getGame()->score(lowest))
				lowest = x;
		}
	}
//	sum = 5*(26-g->score(who))/(sum);
//	return sum;// + g->score(lowest)/26.0; // bonus for hurting winning player
	return (26-g->score(who));
}
//double sum = 0;
//for (unsigned int x = 0; x < g->getNumPlayers(); x++)
//sum+=(26-g->score(x));
//return (26-g->score(who))/sum;

const char *GlobalHeartsPlayer2::getName()
{
	static char name[255];
	iiGameState *igs = getiiModel();
	sprintf(name, "GlobalHearts2(%s,%s)", igs->GetName(), algorithm->getName());
	delete igs;
	return name;
}


double GlobalHeartsPlayer3::cutoffEval(unsigned int who)
{
	double sum = 0;
	int lowest = 0;
//	if (who == 0)
//		lowest = 1;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
	{
//		if (x != who)
//		{
//			sum+=g->score(x);
			if (g->getGame()->score(x)+g->score(x) < g->getGame()->score(lowest)+g->score(lowest))
				lowest = x;
//		}
	}
	//	sum = 5*(26-g->score(who))/(sum);
	//	return sum;// + g->score(lowest)/26.0; // bonus for hurting winning player
//	if (lowest == who)
//		return 1+(26-g->score(who));
	return 26.0-g->score(who)+random()%3;
}
//double sum = 0;
//for (unsigned int x = 0; x < g->getNumPlayers(); x++)
//sum+=(26-g->score(x));
//return (26-g->score(who))/sum;

const char *GlobalHeartsPlayer3::getName()
{
	static char name[255];
	iiGameState *igs = getiiModel();
	sprintf(name, "GlobalHearts3(%s,%s)", igs->GetName(), algorithm->getName());
	delete igs;
	return name;
}

double SafeSimpleHeartsPlayer::cutoffEval(unsigned int who)
{
	double s1, s2;
	//if (rand.rand_double() < 0.999)
	{
		double sum = 0;
		for (unsigned int x = 0; x < g->getNumPlayers(); x++)
			sum+=(26-g->score(x));
		s1 = (26-g->score(who))/sum;
	}
	//else
	{
		// paranoid
		unsigned int me = g->getPlayerNum(this);
		if (who == me)
			s2 = (26-g->score(me))/26;
		else
			s2 = (g->score(me))/26;
	}
	//printf("%f\t%f\n", s1, s2);
	return s1*0.500 + s2*0.500;
	assert(false);
	return 0;
}

Player *SafeSimpleHeartsPlayer::clone() const
{
	return new SafeSimpleHeartsPlayer(algorithm);
}

const char *SafeSimpleHeartsPlayer::getName()
{
	static char name[255];
	iiGameState *igs = getiiModel();
	sprintf(name, "SafeSimple(0.90,%s,%s)", igs->GetName(), algorithm->getName());
	delete igs;
	return name;
}

maxnval *HeartsPlayout::DoRandomPlayout(GameState *gs, Player *p, double epsilon)
{
	//bool shootDanger = false;
	std::vector<Move *> moves;
	CardGameState *cgs = (CardGameState *)gs;
	int havePoints = 0;
	for (unsigned int x = 0; x < cgs->getNumPlayers(); x++)
		if (cgs->taken[x].getSuit(HEARTS) || cgs->taken[x].has(SPADES, QUEEN))
			havePoints++;
	// first pass, don't think about shooting...
	while (!cgs->Done())
	{
		moves.push_back(DoMinPlay(cgs, (havePoints > 1), epsilon));
		gs->ApplyMove(moves.back());
	}
	maxnval *v = new maxnval();
	double sum = 0;
	for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
		sum+=(26-cgs->score(x));
	for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
		v->eval[x] = (26-cgs->score(x))/sum;
	
	while (moves.size() > 0)
	{
		gs->UndoMove(moves.back());
		gs->freeMove(moves.back());
		moves.pop_back();
	}
	return v;
}

Move *HeartsPlayout::DoMinPlay(CardGameState *cgs, bool split, double epsilon)
{
	if (rand.rand_double() < epsilon) // x% chance of a rand move
	{
		return cgs->getRandomMove();
	}
	
	const Trick *trick = cgs->getCurrTrick();
	card winningCard = trick->WinningCard();

	//	int winner = trick->Winner();
	if (trick->curr == 0) // trick just taken!
	{
		int havePoints = 0;
		for (unsigned int x = 0; x < cgs->getNumPlayers(); x++)
			if (cgs->taken[x].getSuit(HEARTS) || cgs->taken[x].has(SPADES, QUEEN))
				havePoints++;
		if (havePoints > 1)
			split = true;
	}
	
	Move *m = cgs->getMoves();
	Move *best = m;
	if (winningCard == -1) // leading, play random
	{
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (rand.rand_double() <= 1/count)
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	else if (Deck::getsuit(winningCard) == Deck::getsuit(((CardMove*)best)->c)) // follow randomly
	{
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (rand.rand_double() <= 1/count)
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	else { // sloughing -- high to low
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (Deck::getcard(SPADES, QUEEN) == ((CardMove*)t)->c)
			{
				best = t;
				break;
			}
			
			if ((rand.rand_double() <= 1/count) || (Deck::getsuit(((CardMove*)t)->c) == HEARTS))
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	assert(0);
	return 0;
}


maxnval *HeartsPlayoutCheckShoot::DoRandomPlayout(GameState *gs, Player *p, double epsilon)
{
	//bool shootDanger = false;
	std::vector<Move *> moves;
	CardGameState *cgs = (CardGameState *)gs;
	int havePoints = 0;
	for (unsigned int x = 0; x < cgs->getNumPlayers(); x++)
		if (cgs->taken[x].getSuit(HEARTS) || cgs->taken[x].has(SPADES, QUEEN))
			havePoints++;
	// first pass, don't think about shooting...
	while (!cgs->Done())
	{
		moves.push_back(DoMinPlay(cgs, (havePoints > 1), epsilon));
		gs->ApplyMove(moves.back());
	}
	maxnval *v = new maxnval();
	double sum = 0;
	for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
		sum+=(26-cgs->score(x));
	for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
		v->eval[x] = (26-cgs->score(x))/sum;
	
	bool playoutWasShoot = false;
	int me = gs->getPlayerNum(p);
	if (cgs->taken[me].getSuit(HEARTS)==0x1FFF && cgs->taken[me].has(SPADES, QUEEN))
		playoutWasShoot = true;
	
	while (moves.size() > 0)
	{
		gs->UndoMove(moves.back());
		gs->freeMove(moves.back());
		moves.pop_back();
	}
	
	// Did we shoot - do max play instead
	if (playoutWasShoot)
	{
		printf("Re-doing shoot.\n");
		while (!cgs->Done())
		{
			moves.push_back(DoMaxPlay(cgs, me, epsilon));
			gs->ApplyMove(moves.back());
		}
		maxnval *v = new maxnval();
		double sum = 0;
		for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
			sum+=(26-cgs->score(x));
		for (unsigned int x = 0; x < gs->getNumPlayers(); x++)
			v->eval[x] = (26-cgs->score(x))/sum;
		
		while (moves.size() > 0)
		{
			gs->UndoMove(moves.back());
			gs->freeMove(moves.back());
			moves.pop_back();
		}

	}

	return v;
}

Move *HeartsPlayoutCheckShoot::DoMinPlay(CardGameState *cgs, bool split, double epsilon)
{
	if (rand.rand_double() < epsilon) // x% chance of a rand move
	{
		return cgs->getRandomMove();
	}
	
	const Trick *trick = cgs->getCurrTrick();
	card winningCard = trick->WinningCard();

	//	int winner = trick->Winner();
	if (trick->curr == 0) // trick just taken!
	{
		int havePoints = 0;
		for (unsigned int x = 0; x < cgs->getNumPlayers(); x++)
			if (cgs->taken[x].getSuit(HEARTS) || cgs->taken[x].has(SPADES, QUEEN))
				havePoints++;
		if (havePoints > 1)
			split = true;
	}
	
	Move *m = cgs->getMoves();
	Move *best = m;
	if (winningCard == -1) // leading, play random
	{
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (rand.rand_double() <= 1/count)
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	else if (Deck::getsuit(winningCard) == Deck::getsuit(((CardMove*)best)->c)) // follow randomly
	{
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (rand.rand_double() <= 1/count)
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	else { // sloughing -- high to low
		double count = 0;
		for (Move *t = m->next; t; t = t->next)
		{
			count += 1;
			if (Deck::getcard(SPADES, QUEEN) == ((CardMove*)t)->c)
			{
				best = t;
				break;
			}
			
			if ((rand.rand_double() <= 1/count) || (Deck::getsuit(((CardMove*)t)->c) == HEARTS))
				best = t;
		}
		best = best->clone(cgs);
		cgs->freeMove(m);
		return best;
	}
	assert(0);
	return 0;
}

Move *HeartsPlayoutCheckShoot::DoMaxPlay(CardGameState *cgs, int me, double epsilon)
{
	return cgs->getRandomMove();
}

void HeartsCardPlayer::selectPassCards(int dir, card &a, card &b, card &c)
{
	CardGameState *cgs = (CardGameState *)g;
	int me = g->getPlayerNum(this);
	int drop[3] = {-1, -1, -1};
	int curr = 0;

	// ugly hack
	if (strcmp(g->getPlayer(me)->getName(), "human") == 0)
	{
		printf("Select cards to pass %s:\n", (dir==kLeftDir)?"left":
					 ((dir==kRightDir)?"right":"across"));
		int which = 0;
		int choices[13];
		for (int x = 0; x < 64; x++)
			if (cgs->cards[me].has(x))
			{
				printf(" (%d)", which);
				PrintCard(x);
				choices[which] = x;
				which++;
			}
		//cgs->cards[me].Print();
		printf("\n\nEnter card numbers (eg. 1 5 6): \n");
		int aa, bb, cc;
		scanf("%d %d %d", &aa, &bb, &cc);
		a = choices[aa];
		b = choices[bb];
		c = choices[cc];
		return;
	}


	// need to check shoot potential first

	// get rid of high spades
	if (cgs->cards[me].suitCount(SPADES) < 4)
	{
		if (cgs->cards[me].has(Deck::getcard(SPADES, ACE)))
		{ drop[curr] = Deck::getcard(SPADES, ACE); curr++; }
		if (cgs->cards[me].has(Deck::getcard(SPADES, KING)))
		{ drop[curr] = Deck::getcard(SPADES, KING); curr++; }
		if (cgs->cards[me].has(Deck::getcard(SPADES, QUEEN)))
		{ drop[curr] = Deck::getcard(SPADES, QUEEN); curr++; }
	}
    
	// diamond suit with high cards not backed by low cards
	if (((cgs->cards[me].getSuit(DIAMONDS)&0x1FC0) == 0) && (curr < 3))
	{
		for (int x = Deck::getcard(DIAMONDS, ACE); x <= Deck::getcard(DIAMONDS, TWO); x++)
		{
			if (cgs->cards[me].has(x) && (curr < 3))
			{
				drop[curr] = x; curr++;
			}
		}
	}
	// club suit with high cards not backed by low cards
	if (((cgs->cards[me].getSuit(CLUBS)&0x1FC0) == 0) && (curr < 3))
	{
		for (int x = Deck::getcard(CLUBS, ACE); x <= Deck::getcard(CLUBS, TWO); x++)
		{
			if (cgs->cards[me].has(x) && (curr < 3))
			{
				drop[curr] = x; curr++;
			}
		}
	}
	if (curr < 3)
	{
		bool stop = false;
		int cnt = cgs->cards[me].suitCount(HEARTS);
		for (int x = Deck::getcard(HEARTS, ACE); (x <= Deck::getcard(HEARTS, TWO))&&(cnt>1); x++)
		{
			if (cgs->cards[me].has(x))
			{
				cnt--;
				if (!stop)
					stop = true;
				else if (curr < 3) {
					drop[curr] = x; curr++;
				}
			}
			else
				cnt--;
		}
	}
	for (int x = Deck::getcard(SPADES, ACE); x <= Deck::getcard(SPADES, TWO); x++)
	{
		if (curr >= 3)
			break;
		for (int y = 1; y < 3; y++)
		{
			if (curr >= 3)
				break;
			if (cgs->cards[me].has(Deck::getcard(y, x)) && 
					(drop[0] != Deck::getcard(y, x)) && (drop[1] != Deck::getcard(y, x)))
			{
				drop[curr] = Deck::getcard(y, x);
				curr++;
			}
		}
	}
	for (int x = Deck::getcard(CLUBS, ACE); x <= Deck::getcard(CLUBS, TWO); x++)
	{
		if (curr >= 3)
			break;
		for (int y = 1; y < 3; y++)
		{
			if (curr >= 3)
				break;
			if (cgs->cards[me].has(Deck::getcard(y, x)) && 
					(drop[0] != Deck::getcard(y, x)) && (drop[1] != Deck::getcard(y, x)))
			{
				drop[curr] = Deck::getcard(y, x);
				curr++;
			}
		}
	}
	for (int x = Deck::getcard(DIAMONDS, ACE); x <= Deck::getcard(DIAMONDS, TWO); x++)
	{
		if (curr >= 3)
			break;
		for (int y = 1; y < 3; y++)
		{
			if (curr >= 3)
				break;
			if (cgs->cards[me].has(Deck::getcard(y, x)) && 
					(drop[0] != Deck::getcard(y, x)) && (drop[1] != Deck::getcard(y, x)))
			{
				drop[curr] = Deck::getcard(y, x);
				curr++;
			}
		}
	}
	a = drop[0];
	b = drop[1];
	c = drop[2];
}

double HeartsCardPlayer::cutoffEval(unsigned int who)
{
	CardGameState *cgs = (CardGameState *)g;
	int numP = g->getNumPlayers();
	if (who == uINF)
		who = g->getPlayerNum(this);
	int owner = g->getPlayerNum(this);
	int split = 0;

	double eval[MAXPLAYERS];
	double unplayedPoints = 26-cgs->allplayed.has(Deck::getcard(SPADES, QUEEN))*13-
	                        cgs->allplayed.suitCount(HEARTS);
	for (int x = 0; x < numP; x++)
		eval[x] = 0.0;
	double sum = 0.00001;
	double shorts = 0;

	if (unplayedPoints == 0)
	{
		for (int x = 0; x < numP; x++)
			sum+=(eval[x] = 26-score(x));
		return eval[who]/sum;
	}
	
	// card score -- uses eval as temp sum, so we do this first.
	int cardeval[MAXPLAYERS] = {0, 0, 0, 0, 0, 0};
	for (int x = 0; x < numP; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			int hnd = cgs->cards[x].getSuit(y);
			if ((hnd == 0) && (x == owner) && (cgs->allplayed.suitCount(x) < 8))
				shorts++;
			int cnt = 0;
			while (hnd)
			{
				if (hnd&0x1)
				{
					cardeval[x] += cnt;
				}
				hnd >>= 1;
				cnt++;
			}
		}
	}
	for (int x = 0; x < numP; x++)
		eval[x] = cardeval[x];
	sum = 0.000001;
	for (int x = 0; x < numP; x++)
		sum += eval[x];
	for (int x = 0; x < numP; x++)
		eval[x]=unplayedPoints*(eval[x]/sum); // weight unplayed points less than real points

	// handle scores
	for (int x = 0; x < numP; x++)
	{
		double tmp;
		tmp = 
			(x)+15;
		if (tmp > 0)
			split++;
		eval[x] += 45+26-unplayedPoints-tmp;
		if ((x != owner) && (cgs->taken[owner].has(Deck::getcard(SPADES, QUEEN))))
			eval[x]+=3; // anti-queen bonus
	}
	// handle QofS -- if I originally had it
	if (cgs->original[owner].has(Deck::getcard(SPADES, QUEEN)))
	{
		if (cgs->cards[owner].getSuit(SPADES))
		{
			for (int x = 0; ;x++)
			{
				const Trick *t = cgs->getTrick(x);
				if (!t)
					break;
				if (t->WinningCard() == 2)
				{
					if (cgs->cards[owner].getSuit(SPADES))
						eval[owner]/=4;
					else {
						for (int y = x+1; ;y++)
						{
							t = cgs->getTrick(y);
							if (t == 0)
								break;
							for (int z = 0; z < t->curr; z++)
							{
								if ((t->player[z] == owner) && (Deck::getsuit(t->play[z]) == SPADES))
								{
									eval[owner]/=4;
									break;
								}
							}
						}
					}
					break;
				}
			}
		}
	}
 
	if (split > 1)
	{ eval[owner] += 3; }
	if (g->getNextPlayerNum() != owner)
	{ eval[owner] += 1; }
	//if (cgs->allplayed.count()/numP < 6)
	eval[owner]+=shorts;

	sum = 0.000001;
	for (int x = 0; x < numP; x++)
		sum += eval[x];

	return eval[who]/sum;
}
//WinningCard

double HeartsCardPlayer::handEval(unsigned int who)
{
	return 0;
}

double HeartsCardPlayer::score(unsigned int who)
{
	if (who == uINF)
		who = g->getPlayerNum(this);

	return g->score(who);
}


/*
	long64 HeartsCardPlayer::bestMove(const Move *m, GameState *g, bool lookup) // for killer heuristic
	{
	// we work on learning, but use our static ordering
	return CardPlayer::bestMove(m, g, lookup); // lookup=true means we don't adjust values

	//	const CardMove *cm = (CardMove *)m;
	//	CardGameState *cgs = (CardGameState*)g;
	//	return rankMove(cm->c, cgs->getCurrTrick(), g);
	}
*/
// the higher number the more we like it
int HeartsCardPlayer::rankMove(Move *m, GameState *gs, bool optional)
{
	if (history)
		return Player::rankMove(m, gs, optional);
	if (optional)
		return kIgnoreMove;
	//printf("Using our rank\n";
	const Trick *t = ((CardGameState*)gs)->getCurrTrick();
	int c = ((CardMove*)m)->c;

	// if we are leading
	if (t->curr == 0)
	{
		// avoid the A-Q of Spades
		if (c <= Deck::getcard(SPADES, QUEEN))
			return 2-c;
		if (c < Deck::getcard(CLUBS, TWO))
			return 39-(c%13);
		// hearts we prefer to lead low, and a low heart might be better than
		// another lead
		return 30+(c%13);
	}
	// if we are following an Ace or King of Spades
	if ((t->curr > 0) && (t->play[0] < Deck::getcard(SPADES, QUEEN)))
	{
		// first, drop the queen
		if (c == Deck::getcard(SPADES, QUEEN))
			return 100;
		// if in suit, spades are best from high to low.
		if (Deck::getsuit(c) == SPADES)
			return 26-c;
		// otherwise play from high to low in other suits
		return 13-(c%13);
	}
	// following in spades
	if ((t->curr > 0) && (Deck::getsuit(t->play[0]) == 0))
	{
		if (c == Deck::getcard(SPADES, QUEEN))
			return 0;
		return 13-c;
	}
	// we are sloughing
	if ((t->curr > 0) && (Deck::getsuit(c) != Deck::getsuit(t->play[0])))
	{
		// drop the queen
		if (c == Deck::getcard(SPADES, QUEEN))
			return 100;
		return 13-(c%13); // play a high card
	}
	return 13-(c%13);
}

double HeartsCardPlayer::getMaxsum(GameState *)
{
	return 100;//26*(g->getNumPlayers()-1);
}

double HeartsCardPlayer::scoreLowerBound(unsigned int who)
{
	//	return 0;
	CardGameState *cgs = (CardGameState *)g;
	if (who == uINF)
		who = g->getPlayerNum(this);
	int pointsOut = 0, othersPoints = 0;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
	{
		int thisPlayer = cgs->taken[x].suitCount(HEARTS)+
		                 13*cgs->taken[x].has(Deck::getcard(SPADES, QUEEN));
		pointsOut += thisPlayer;
		if (x != who)
			othersPoints += thisPlayer;
	}
	return g->getNumPlayers()*othersPoints;
}

double HeartsCardPlayer::scoreUpperBound(unsigned int who)
{
	CardGameState *cgs = (CardGameState *)g;
	if (who == uINF)
		who = g->getPlayerNum(this);
	int pointsOut = 0, othersPoints = 0;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
	{
		int thisPlayer = cgs->taken[x].suitCount(HEARTS)+
		                 13*cgs->taken[x].has(Deck::getcard(SPADES, QUEEN));
		pointsOut += thisPlayer;
		if (x != who)
			othersPoints += thisPlayer;
	}
	pointsOut = 26-pointsOut;

	return g->getNumPlayers()*(othersPoints+pointsOut);
}

HashState *HeartsGameState::getHashState(HashState *)
{
	if (getCurrTrick()->curr != 0)
		return 0;
	HashState *hs = new HashState();
	cardHashState *cHS = new cardHashState();
	CardPlayer *p[6];
	unsigned long long hands[6];

	hs->ghs = cHS;
	cHS->nextp = getNextPlayerNum();
	cHS->nump = getNumPlayers();

	for (int w = 0; w < cHS->nump; w++)
	{
		p[w] = (CardPlayer*)getPlayer(w);
		cHS->cards[w] = 0;//p->cards.getHand();
		cHS->pts[w] = (int)p[w]->score(w);
		cHS->numCards = (cHS->numCards<<16)|
			((cards[w].suitCount(0)&0xF)<<0)|
			((cards[w].suitCount(1)&0xF)<<4)|
			((cards[w].suitCount(2)&0xF)<<8)|
			((cards[w].suitCount(3)&0xF)<<12);
		//cHS->numCards[w] = 0;
		//for (int x = 0; x < 4; x++)
		//cHS->numCards[w] = ((cHS->numCards[w])<<4)+cards[w].suitCount(x);
		hands[w] = cards[w].getHand();
	}

	for (int x = 0; x <= Deck::getcard(SPADES, QUEEN); x++)
	{
		for (int w = 0; w < cHS->nump; w++)
		{
			if (hands[w]&1)
			{
				cHS->cards[w]++;
			}
			cHS->cards[w] = cHS->cards[w]<<1;
			hands[w] = hands[w]>>1;
		}
	}
	for (int w = 0; w < cHS->nump; w++)
		cHS->cards[w] = cHS->cards[w]<<1;

	for (int x = Deck::getcard(SPADES, JACK); x < Deck::getcard(HEARTS, TWO); x++)
	{
		if (x%16==0)
		{
			for (int w = 0; w < cHS->nump; w++)
				cHS->cards[w] <<= 1;
		}
		for (int w = 0; w < cHS->nump; w++)
		{
			if (hands[w]&1)
			{
				cHS->cards[w]++;
				for (int z = 0; z < cHS->nump; z++)
					cHS->cards[z] = cHS->cards[z]<<1;
			}
			hands[w] = hands[w]>>1;
		}
	}

	return hs;
}

//int ModelingHeartsPlayer::getNumScores()
//{
//	int range = 0;
//	bool queen = false;
//	for (int x = 0; x < (int)g->getNumPlayers(); x++)
//	{
//		range += ((CardGameState*)g)->original[x].suitCount(HEARTS);
//		queen = (queen)||(((CardGameState*)g)->original[x].has(Deck::getcard(SPADES, QUEEN)));
//	}
//	switch (g->getNumPlayers())
//	{
//		case 2:
//			return (range+1)*(queen?g->getNumPlayers():1); break;
//		case 3:
//			return (range+1)*(range+2)*(queen?(g->getNumPlayers()):(1))/2;
//		case 4:
//			return (range*range*range+6*range*range+11*range)*(queen?g->getNumPlayers():1)/6+1;
//	}
//	return 0;
//}
//
//void ModelingHeartsPlayer::getScoreAndEval(int which, int *values, int *evals)
//{
////	HeartsGameState *sgs = (HeartsGameState *)g;
//	int nump = g->getNumPlayers();
//	int range = 0;
//	bool queen = false;
//	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
//	{
//		range += ((CardGameState*)g)->original[x].suitCount(HEARTS);
//		queen = queen||((CardGameState*)g)->original[x].has(Deck::getcard(SPADES, QUEEN));
//	}
//	
//	if (nump == 3)
//	{
//		// assume 3 players for the moment...
//		int first, second, third;
//		first = 0;
//		second = queen?(which/3):which;
//		while (second >= range+1-first)
//		{ second -= (range+1-first); first++; }
//		
//		third = range-first-second;
//		values[0] = first;
//		values[1] = second;
//		values[2] = third;
//		if (queen)
//			values[which%3]+=13;
//	}
//	else if (nump == 4)
//	{
//		// assume 3 players for the moment...
//		int first, second, third, fourth;
//		int rng = range;
//		int whoQueen = which%4;
//		if (queen)
//			which/=4;
//		int whch = which;
//		fourth = 0;
//		while (whch >= (rng+1)*(rng+2)/2)
//		{ fourth++; whch -= (rng+1)*(rng+2)/2; rng--; }
//		
//		first = 0;
//		second = whch;
//		while (second >= rng+1-first)
//		{ second -= (rng+1-first); first++; }
//		third = rng-first-second;
//		
//		values[0] = fourth;
//		values[1] = first;
//		values[2] = second;
//		values[3] = third;
//		if (queen)
//			values[whoQueen]+=13;
//		printf("%d\t(%2d, %2d, %2d, %2d)\n", which, values[0], values[1], values[2], values[3]);
//	}
//	
//	int maxs[nump];
//	for (int x = 0; x < nump; x++)
//	{
//		maxs[x] = -100;
//		evals[x] = (values[x] + (int)g->getGame()->score(x));
//	}
//	for (int x = 0; x < nump; x++)
//		for (int y = 0; y < nump; y++)
//			if  (x != y)
//				maxs[x] = std::max(maxs[x], evals[x]-evals[y]);
//	for (int x = 0; x < nump; x++)
//		evals[x] = -maxs[x];
///*
//	if (getName()[0] == 'm')
//	{
//		for (int x = 0; x < nump; x++)
//			if ((g->getPlayer(x)->getName()[0] == 'x') ||
//					(g->getPlayer(x)->getName()[1] == 'x'))
//				evals[x] = 26-values[x];
//	}
//	// not modeling maximizing player
//	else if (getName()[0] == 'x')
//	{
//		for (int x = 0; x < nump; x++)
//			evals[x] = 26-values[x];
//	}
//	*/
//}
//
//bool ModelingHeartsPlayer::isRawScorePossible(int *vals)
//{
//	return true;
//}
//
//int ModelingHeartsPlayer::getCurrentScoreIndex()
//{
//	int range = 1;
//	int queen = -1;
//	for (int x = 0; x < (int)g->getNumPlayers(); x++)
//	{
//		range += ((CardGameState*)g)->original[x].suitCount(HEARTS);
//		if (((CardGameState*)g)->taken[x].has(Deck::getcard(SPADES, QUEEN)))
//			queen = x;
//	}
//	int a = ((CardGameState*)g)->taken[0].suitCount(HEARTS);
//	int b = ((CardGameState*)g)->taken[1].suitCount(HEARTS);
//	int index = a*(2*range+1-a)/2 + b;
//	if (queen != -1)
//		index = index*g->getNumPlayers()+queen;
//	//printf("(%d, %d, %d) = %d\n", a, b, range-a-b-1, index);
//	return index;
//}
//
//double ModelingHeartsPlayer::score(unsigned int who)
//{
//	if (who == uINF)
//		who = g->getPlayerNum(this);
//	
//	double answer = ((CardGameState*)g)->score(who);
//	return answer;
//}
//
//unsigned int ModelingHeartsPlayer::getMaxDepth()
//{
//	unsigned int total = 0;
//	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
//		total += ((CardGameState*)g)->cards[x].count();
//	return total;
//}
//
//int ModelingHeartsPlayer::getNextDepth(int curr)
//{
//	if (curr == -1)
//		return getMaxDepth();
//	else
//		return getMaxDepth()+1;
//	
//	int np = g->getNumPlayers();
//	CardGameState *cs = (CardGameState *)g;
//	const Trick *t = cs->getCurrTrick();
//	if (curr == -1) {
//		return (np-t->curr);
//	}
//	if (curr == (int)getMaxDepth())
//		return curr+1;
//	if (curr*2 > (int)getMaxDepth())
//		return getMaxDepth();
//	return curr+np;
//}
//
//Player *ModelingHeartsPlayer::clone() const
//{
//	return new ModelingHeartsPlayer(algorithm);
//}
//
//// the higher number the better the move is
//int ModelingHeartsPlayer::rankMove(Move *m, GameState *gs, bool optional)
//{
//	if (history)
//	{
//		int val = Player::rankMove(m, gs, optional);
//		if (val)
//			return val;
//	}
//	if (optional)
//		return kIgnoreMove;
//	//printf("Using our rank\n";
//	const Trick *t = ((CardGameState*)gs)->getCurrTrick();
//	int c = ((CardMove*)m)->c;
//	
//	// if we are leading
//	if (t->curr == 0)
//	{
//		// avoid the A-Q of Spades
//		if (c <= Deck::getcard(SPADES, QUEEN))
//			return 2-c;
//		if (c < Deck::getcard(CLUBS, TWO))
//			return 39-(c%13);
//		// hearts we prefer to lead low, and a low heart might be better than
//		// another lead
//		return 30+(c%13);
//	}
//	// if we are following an Ace or King of Spades
//	if ((t->curr > 0) && (t->play[0] < Deck::getcard(SPADES, QUEEN)))
//	{
//		// first, drop the queen
//		if (c == Deck::getcard(SPADES, QUEEN))
//			return 100;
//		// if in suit, spades are best from high to low.
//		if (Deck::getsuit(c) == SPADES)
//			return 26-c;
//		// otherwise play from high to low in other suits
//		return 13-(c%13);
//	}
//	// following in spades
//	if ((t->curr > 0) && (Deck::getsuit(t->play[0]) == 0))
//	{
//		if (c == Deck::getcard(SPADES, QUEEN))
//			return 0;
//		return 13-c;
//	}
//	// we are sloughing
//	if ((t->curr > 0) && (Deck::getsuit(c) != Deck::getsuit(t->play[0])))
//	{
//		// drop the queen
//		if (c == Deck::getcard(SPADES, QUEEN))
//			return 100;
//		return 13-(c%13); // play a high card
//	}
//	return 13-(c%13);
//}
//
//void ModelingHeartsPlayer::selectPassCards(int dir, card &a, card &b, card &c)
//{
//	CardGameState *cgs = (CardGameState *)g;
//	int me = g->getPlayerNum(this);
//	int drop[3] = {-1, -1, -1};
//	int curr = 0;
//	
//	// need to check shoot potential first
//	
//	// get rid of high spades
//	if (cgs->cards[me].suitCount(SPADES) < 4)
//	{
//		if (cgs->cards[me].has(Deck::getcard(SPADES, ACE)))
//		{ drop[curr] = Deck::getcard(SPADES, ACE); curr++; }
//		if (cgs->cards[me].has(Deck::getcard(SPADES, KING)))
//		{ drop[curr] = Deck::getcard(SPADES, KING); curr++; }
//		if (cgs->cards[me].has(Deck::getcard(SPADES, QUEEN)))
//		{ drop[curr] = Deck::getcard(SPADES, QUEEN); curr++; }
//	}
//	
//	// diamond suit with high cards not backed by low cards
//	if (((cgs->cards[me].getSuit(DIAMONDS)&0x1FC0) == 0) && (curr < 3))
//	{
//		for (int x = Deck::getcard(DIAMONDS, ACE); x <= Deck::getcard(DIAMONDS, TWO); x++)
//		{
//			if (cgs->cards[me].has(x) && (curr < 3))
//			{
//				drop[curr] = x; curr++;
//			}
//		}
//	}
//	// club suit with high cards not backed by low cards
//	if (((cgs->cards[me].getSuit(CLUBS)&0x1FC0) == 0) && (curr < 3))
//	{
//		for (int x = Deck::getcard(CLUBS, ACE); x <= Deck::getcard(CLUBS, TWO); x++)
//		{
//			if (cgs->cards[me].has(x) && (curr < 3))
//			{
//				drop[curr] = x; curr++;
//			}
//		}
//	}
//	if (curr < 3)
//	{
//		bool stop = false;
//		int cnt = cgs->cards[me].suitCount(HEARTS);
//		for (int x = Deck::getcard(HEARTS, ACE); (x <= Deck::getcard(HEARTS, TWO))&&(cnt>1); x++)
//		{
//			if (cgs->cards[me].has(x))
//			{
//				cnt--;
//				if (!stop)
//					stop = true;
//				else if (curr < 3) {
//					drop[curr] = x; curr++;
//				}
//			}
//			else
//				cnt--;
//		}
//	}
//	for (int x = Deck::getcard(SPADES, ACE); x <= Deck::getcard(SPADES, TWO); x++)
//	{
//		if (curr >= 3)
//			break;
//		for (int y = 1; y < 3; y++)
//		{
//			if (curr >= 3)
//				break;
//			if (cgs->cards[me].has(Deck::getcard(y, x)) && 
//					(drop[0] != Deck::getcard(y, x)) && (drop[1] != Deck::getcard(y, x)))
//			{
//				drop[curr] = Deck::getcard(y, x);
//				curr++;
//			}
//		}
//	}
//	a = drop[0];
//	b = drop[1];
//	c = drop[2];
//}
//

Move *HeartsDucker::Play()
{
	CardGameState *cgs = (CardGameState*)g;
	const Trick *trick = cgs->getCurrTrick();
	card winningCard = trick->WinningCard();

	Move *m = g->getMoves();
	Move *best = m;
	if (winningCard == -1) // leading, play low
	{
		for (Move *t = m->next; t; t = t->next)
		{
			if (Deck::getrank(((CardMove*)t)->c) >
				Deck::getrank(((CardMove*)best)->c))
				best = t;
		}
		best = best->clone();
		g->freeMove(m);
		return best;
	}
	// follow under if possible
	else if (Deck::getsuit(winningCard) == Deck::getsuit(((CardMove*)best)->c))
	{
		for (Move *t = m->next; t; t = t->next)
		{
			card c1 = ((CardMove*)t)->c;
			card bestCard = ((CardMove*)best)->c;
			
			if (Deck::getrank(bestCard) < winningCard)
			{
				if ((Deck::getrank(c1) < Deck::getrank(bestCard)) || // play high if we lose
					(Deck::getrank(c1) > Deck::getrank(winningCard))) // play lower if current best wins
					best = t;
			}
			else if ((Deck::getrank(c1) < Deck::getrank(bestCard)) &&
					 (Deck::getrank(c1) > Deck::getrank(winningCard)))
			{
				best = t;
			}
		}
		best = best->clone();
		g->freeMove(m);
		return best;
	}
	else { // sloughing -- high to low
		for (Move *t = m->next; t; t = t->next)
		{
			if (Deck::getrank(((CardMove*)t)->c) <
				Deck::getrank(((CardMove*)best)->c))
				best = t;
		}
		best = best->clone();
		g->freeMove(m);
		return best;
	}
}

void HeartsDucker::selectPassCards(int dir, card &a, card &b, card &c)
{
	int me = g->getPlayerNum(this);
	CardGameState *cgs = (CardGameState*)g;
	a = cgs->cards[me].getRandomCard();
	do {
		b = cgs->cards[me].getRandomCard();
	} while (b == a);
	do {
		c = cgs->cards[me].getRandomCard();
	} while ((c == b) && (c == a));
}

Move *HeartsShooter::Play()
{
	int me = g->getPlayerNum(this);
	CardGameState *cgs = (CardGameState*)g;
	const Trick *trick = cgs->getCurrTrick();
	card winningCard = trick->WinningCard();

	Move *m = g->getMoves();
	Move *best = m;
	
	for (Move *t = m->next; t; t = t->next)
	{
		card choice = ((CardMove*)t)->c;
		card bestCard = ((CardMove*)best)->c;
		if ((winningCard == -1) && (choice <= 2))
		{
			best = t;
			break;
		}
		
		// first go for longest suit
		if (cgs->original[me].suitCount(Deck::getsuit(choice)) >
			cgs->original[me].suitCount(Deck::getsuit(bestCard)))
			best = t;
		else if (cgs->original[me].suitCount(Deck::getsuit(choice)) ==
						cgs->original[me].suitCount(Deck::getsuit(bestCard)))
		{
			if (Deck::getrank(choice) < // then go for highest card
				Deck::getrank(bestCard))
				best = t;
		}
	}
	best = best->clone();
	g->freeMove(m);
	return best;
}

void HeartsShooter::selectPassCards(int dir, card &a, card &b, card &c)
{
	int me = g->getPlayerNum(this);
	CardGameState *cgs = (CardGameState*)g;
	a = cgs->cards[me].getRandomCard();
	do {
		b = cgs->cards[me].getRandomCard();
	} while (b == a);
	do {
		c = cgs->cards[me].getRandomCard();
	} while ((c == b) && (c == a));
}

cardProbData iiHeartsState::cpd;
//("/Users/nathanst/Desktop/model.txt");

GameState *iiHeartsState::getGameState(double &prob)
{
	prob = 1.0f;
	std::vector<std::vector<card> > newCards;
	HeartsGameState *cgs = (HeartsGameState*)originalGame->create();
	int forbidden[MAXPLAYERS][4];
	// set basic information
	//	cgs->numPlayers = numPlayers;
	cgs->setRules(rules);
	cgs->numCards = numCards;
	cgs->trump = trump;
	cgs->passDir = passDir;
	cgs->currTrick = currTrick;
	cgs->special = special;
	cgs->setFirstPlayer(currPlr);
	cgs->currPlr = currPlr;
	cgs->numCardsPassed = numCardsPassed;

	for (int x = 0; x < numPlayers; x++)
		for (int y = 0; y < 4; y++)
			forbidden[x][y] = 0;

	for (int x = 0; x < numPlayers; x++)
		for (unsigned int y = 0; y < passes[x].size(); y++)
			cgs->passes[x].push_back(passes[x][y]);
	
	cgs->allplayed.setHand(&allplayed);
	//cgs->t = new Trick[numCards+1];
	for (int x = 0; x < numCards+1; x++)
	{
		cgs->t[x].reset(numPlayers, trump);
		for (int y = 0; y < t[x].curr; y++)
		{
			cgs->t[x].AddCard(t[x].play[y], t[x].player[y]);
			if ((y != 0) && (Deck::getsuit(t[x].play[y]) != Deck::getsuit(t[x].play[0])))
			{ // forbidden suit
				forbidden[t[x].player[y]][Deck::getsuit(t[x].play[0])] = 1;
			}
		}
		cgs->t[x].curr = t[x].curr;
	}
	
	for (int x = 0; x < numPlayers; x++)
	{
		Player *p = master->clone();
		cgs->addPlayer(p);
	}
	newCards.resize(numPlayers);
	int base = rand.ranged_long(0, 3);
	while (true)
	{
		d.Shuffle(rand.rand_long());
		int count = 20;
		for (int x = 0; x < numPlayers; x++)
		{
			cgs->cards[x].setHand(&cards[x]);
			cgs->taken[x].setHand(&taken[x]);
			cgs->original[x].setHand(&original[x]);
			cgs->played[x].setHand(&played[x]);
			
			d.removeHand(&cgs->cards[x]);
			d.removeHand(&cgs->taken[x]);
			d.removeHand(&cgs->original[x]);
			if (numCardsPassed != numPlayers*3)
			{
				//printf("numCardsPassed = %d\n", numCardsPassed);
				for (unsigned int y = 0; y < cgs->passes[x].size(); y++) // temporarily give us our passes back
				{
					d.clear(cgs->passes[x][y]);
					cgs->TakeCard(x, cgs->passes[x][y]);
					//printf("%d Temporarily taking: ", x);
					//PrintCard(cgs->passes[x][y]);
				}
			}
		}
		// new QoS code -- 5% chance of not running
//		if (d.has(SPADES, QUEEN) && advancedModeling && (rand.rand_double() > 0.05))
//		{
//			bool lead[4] = {false, false, false, false};
//			for (int x = 0; x <= cgs->getCurrTrickNum(); x++)
//			{
//				const Trick *tr = cgs->getTrick(x);
//				if (tr->curr == 0) break;
//				if (Deck::getsuit(tr->play[0]) == SPADES)
//				{
//					//printf("Player %d load\n", t->player[0]);
//					lead[tr->player[0]] = true;
//				}
//			}
//			double cnt = 1;
//			int who = -1;
//			for (int x = 0; x < numPlayers; x++)
//			{
//				if (lead[x] || forbidden[x][SPADES] || ((int)cgs->original[x].count()==numCards))
//					continue;
//				if (rand.rand_double() <= 1/cnt)
//					who = x;
//				cnt++;
//			}
//			if (who != -1)
//			{
//				cgs->original[who].set(SPADES, QUEEN);
//				cgs->cards[who].set(SPADES, QUEEN);
//				d.clear(SPADES, QUEEN);
//			}
//		}
		for (int y = 0; y < numPlayers; y++)
		{
			int x = (y+base)%numPlayers;
			while ((int)cgs->original[x].count() < numCards)
			{
				card c = d.Deal();
				if (c == -1)
				{
					cgs->Print(1);
					printf("no cards left to deal?!?\n"); fflush(stdout);
					exit(0);
				}
				if (forbidden[x][Deck::getsuit(c)])
				{
					d.set(c);
					count--;
					if (count <= 0)
						break;
				}
				else {
					newCards[x].push_back(c);
					cgs->original[x].set(c);
					cgs->cards[x].set(c);
				}
			}
			if (count <= 0)
				break;
		}
		if (count <= 0)
			continue;

		if (advancedModeling)
		{
			bool legal = true;
			prob = 0;
			for (int x = 0; x < numPlayers; x++)
			{
				double nextProb = GetProbability(x, newCards[x], cgs->t, d);
				if (nextProb == -1)
				{
					printf("rejecting states\n");
					legal = false;
				}
				prob += nextProb;

				if ((passDir != 0) && (numCardsPassed == 3*numPlayers))
				{
					prob += GetPassProbability(passes[x], newCards[x]);
				}
			}
			prob = exp(prob);
			if (!legal)
				continue;
		}
		// if I get here I'm done, otherwise my continue sends me back to the front...
		// and starts over!
		break;
	}
	//	cgs->movesSoFar = totalMoves;
	
	//	printf("Created state:\n");
	//	cgs->Print(1);
	// discard some cards for those who have already done so...but we do it automatically for them
	// but only do this during the initial phase of the game, when we don't know any of their cards;
	// don't want to make the hands consistent
	if (numCardsPassed != 3*numPlayers)
	{
		for (int x = 0; x < numPlayers; x++)
			for (unsigned int y = 0; y < cgs->passes[x].size(); y++) // undo temporarily give us our passes back
				cgs->UntakeCard(x, cgs->passes[x][y]);
		
		int count = 0;
		for (int z = 0; z < numPlayers; z++)
		{
			//int x = (z+firstPlayer)%numPlayers;
			int x = (z+0)%numPlayers;
			count += cgs->passes[x].size();
			//printf("First is %d; curr is %d; %d has already passed %d cards\n", firstPlayer, currPlr, x, cgs->passes[x].size());
			if ((cgs->passes[x].size() < 3) && (count < numCardsPassed))
			{
				card a, b, c;
				//##//X\//
				((HeartsCardPlayer*)cgs->getPlayer(x))->selectPassCards(passDir, a, b, c);
				count++;
				cgs->passes[x].push_back(a);
				cgs->UntakeCard(x, a);
				if (count < numCardsPassed)
				{
					count++;
					cgs->passes[x].push_back(b);
					cgs->UntakeCard(x, b);
				}
				if (count < numCardsPassed)
				{
					count++;
					cgs->passes[x].push_back(c);
					cgs->UntakeCard(x, c);
				}
			}
		}
		assert(count == numCardsPassed);
	}
	
#ifdef __MWERKS__
	for (int x = 0; x < numPlayers; x++)
	{
		char msg[255];
		sprintf(msg, "Player %d has %X in spades", x, cgs->cards[x].getSuit(0));
		ai_debug(msg);
	}
#endif
	//printf("Set up new GameSate with %d to move\n", cgs->getNextPlayerNum());
	if (prob == 0)
		fprintf(stderr, "Error, returning 0 probability\n");
	return cgs;
}

double iiHeartsState::GetCardProbability(int who, std::vector<card> passes[],
												 card c, std::vector<double> &priors)
{
	std::vector<double> probs;
	double sum = 0;
	
	// everyone starts with equal probability of having the card
	for (unsigned int x = 0; x < priors.size(); x++)
	{
		probs.push_back(priors[x]);
	}
	
	for (unsigned int x = 0; x < priors.size(); x++)
	{
		for (unsigned int y = 0; y < passes[x].size(); y++)
		{
			probs[x] *= cpd.freq1[passes[x][y]][c]*51.0/13.0; // 13/51
		}
	}
	
	for (unsigned int x = 0; x < probs.size(); x++)
	{
		sum += probs[x];
	}
	//	for (unsigned int x = 0; x < probs.size(); x++)
	//		printf("Probability that (Player %d) has %d = %f\n", x, c, probs[x]/sum);
	
	return clamp(probs[who]/sum, 0, 1);
}


double iiHeartsState::GetPassProbability(std::vector<card> &passes, std::vector<card> &newCards)
{
	double prob = 0;
	//	for (unsigned int x = 0; x < passes.size(); x++)
	//	{
	//		assert(passes[x] != -1);
	//		for (unsigned int y = 0; y < newCards.size(); y++)
	//		{
	//			prob += log(cpd.freq1[passes[x]][newCards[y]]);
	//		}
	//	}
	
	for (unsigned int x = 0; x < passes.size(); x++)
	{
		for (unsigned int z = x+1; z < passes.size(); z++)
		{
			for (unsigned int y = 0; y < newCards.size(); y++)
			{
				prob += log(cpd.freq2[passes[x]][passes[z]][newCards[y]]);
			}
		}
	}
	return clamp(prob, -10, 1);
}

// prob(card c | evidence) = exp(ln(prob(c|a))+ ln(prob(c|b))+ ...)
double iiHeartsState::GetProbability(int player, std::vector<card> &newCards, Trick *t, Deck &d)
{
	if (newCards.size() == 0)
		return 0.0;
	
	double result = 0;
	for (unsigned int x = 0; x < newCards.size(); x++)
	{
		for (int y = 0; y < numCards; y++)
		{
			double val = GetTrickOdds(player, t[y], newCards[x], d);
			if (val == 0)
				return val = 1e-20; // return -1 to outright reject hand
			result += log(val);
		}
	}
	
	return clamp(result, -10, 1);
	//	std::vector<double> probs;
	//	probs.resize(64);
	//	double sum = 0;
	//	// get probability for every card
	//	for (int x = 0; x < 64; x++)
	//	{
	//		if (!d.has(x))
	//			continue;
	//
	//		// for each trick / evidence
	//		for (int y = 0; y < numCards; y++)
	//		{
	//			probs[x] += log(GetTrickOdds(player, t[y], x, d));
	//		}
	//		probs[x] = exp(probs[x]);
	//		sum += probs[x];
	//	}
	//	double ours = 0;
	//	for (unsigned int x = 0; x < newCards.size(); x++)
	//		ours += probs[newCards[x]];
	//
	//	return ours/sum;
}

double iiHeartsState::GetTrickOdds(int player, Trick &t, int which, Deck &d)
{
	if (t.curr == 0)
	{
		return 1.0;
	}
	if (t.player[0] == player)
	{
		//assert (cpd.probGivenLead[t.play[0]][which] != 0);
		//		if (cpd.probGivenLead[t.play[0]][which] == 0)
		//			return 1e-10;
		return cpd.probGivenLead[t.play[0]][which];
	}
	int myPlay = -1;
	for (int x = 1; x < t.curr; x++)
		if (t.player[x] == player)
			myPlay = x;
	if (myPlay == -1)
		return 1.0;
	
	//assert(cpd.probGivenLeadAndPlay[t.play[0]][t.play[myPlay]][which] != 0);
	//	if (cpd.probGivenLeadAndPlay[t.play[0]][t.play[myPlay]][which] == 0)
	//		return 1e-10;
	return clamp(cpd.probGivenLeadAndPlay[t.play[0]][t.play[myPlay]][which], -10, 1);
}

//cardProbData advancedIIHeartsState::cpd("model.txt");
cardProbData advancedIIHeartsState::cpd;
	
advancedIIHeartsState::advancedIIHeartsState()
:iiHeartsState()
{
	//cpd.load();
}

advancedIIHeartsState::~advancedIIHeartsState()
{
}

GameState *advancedIIHeartsState::getGameState(double &prob)
{
	HeartsGameState *cgs = (HeartsGameState*)originalGame->create();
	int forbidden[MAXPLAYERS][4];
	// set basic information
	//	cgs->numPlayers = numPlayers;
	cgs->setRules(rules);
	cgs->numCards = numCards;
	cgs->trump = trump;
	cgs->passDir = passDir;
	cgs->currTrick = currTrick;
	cgs->special = special;
	cgs->setFirstPlayer(currPlr);
	cgs->currPlr = currPlr;
	cgs->numCardsPassed = numCardsPassed;
	
	for (int x = 0; x < numPlayers; x++)
		for (int y = 0; y < 4; y++)
			forbidden[x][y] = 0;

	for (int x = 0; x < numPlayers; x++)
		for (unsigned int y = 0; y < passes[x].size(); y++)
			cgs->passes[x].push_back(passes[x][y]);
	
	cgs->allplayed.setHand(&allplayed);
	//cgs->t = new Trick[numCards+1];
	for (int x = 0; x < numCards+1; x++)
	{
		cgs->t[x].reset(numPlayers, trump);
		for (int y = 0; y < t[x].curr; y++)
		{
			cgs->t[x].AddCard(t[x].play[y], t[x].player[y]);
			if ((y != 0) && (Deck::getsuit(t[x].play[y]) != Deck::getsuit(t[x].play[0])))
			{ // forbidden suit
				forbidden[t[x].player[y]][Deck::getsuit(t[x].play[0])] = 1;
			}
		}
		cgs->t[x].curr = t[x].curr;
	}
	
	for (int x = 0; x < numPlayers; x++)
	{
		Player *p = master->clone();
		cgs->addPlayer(p);
	}
	
	// deal the cards out
	d.Shuffle(rand.rand_long());
	for (int x = 0; x < numPlayers; x++)
	{
		cgs->cards[x].setHand(&cards[x]);
		cgs->taken[x].setHand(&taken[x]);
		cgs->original[x].setHand(&original[x]);
		cgs->played[x].setHand(&played[x]);
		
		d.removeHand(&cgs->cards[x]);
		d.removeHand(&cgs->taken[x]);
		d.removeHand(&cgs->original[x]);
		if (numCardsPassed != numPlayers*3)
		{
			//printf("numCardsPassed = %d\n", numCardsPassed);
			for (unsigned int y = 0; y < cgs->passes[x].size(); y++) // temporarily give us our passes back
			{
				d.clear(cgs->passes[x][y]);
				cgs->TakeCard(x, cgs->passes[x][y]);
				//printf("%d Temporarily taking: ", x);
				//PrintCard(cgs->passes[x][y]);
			}
		}
//		if (d.has(Deck::getcard(SPADES, QUEEN)) && originalGame->cards[x].has(Deck::getcard(SPADES, QUEEN)))
//		{
//			cgs->original[x].set(Deck::getcard(SPADES, QUEEN));
//			cgs->cards[x].set(Deck::getcard(SPADES, QUEEN));
//			d.clear(Deck::getcard(SPADES, QUEEN));
//		}
	}

	// test for forced cards from others being short
	for (int x = 0; x < 4; x++)
	{
		int available = -1;
		int forbiddenCount = 0;
		for (int y = 0; y < numPlayers; y++)
		{
			if (numCards-cgs->original[y].count() == 0)
				forbiddenCount++;
			else if (forbidden[y][x] == 1)
				forbiddenCount++;
			else
				available = y;
		}
		if (forbiddenCount == numPlayers-1)
		{
			//printf("Only %d can have %d, giving him the rest!\n", available, x);
			assert(available != -1);
			cgs->cards[available].addSuit(x, d.getSuit(x));
			cgs->original[available].addSuit(x, d.getSuit(x));
			d.setSuit(x, 0);
		}
	}

	std::vector<int> counts;
	int total = 0;
	for (int y = 0; y < numPlayers; y++)
	{
		//printf("Player %d needs %d more cards\n", y, numCards-cgs->original[y].count());
		counts.push_back(numCards-cgs->original[y].count());
		total += counts.back();
	}
	uint64_t possibilities = iiCardState::getNumCombos(counts);
	bool legal = false;
	std::vector<std::vector<card> > newCards;
	while (!legal)
	{
		newCards.resize(0);
		legal = true;
		uint64_t value = rand.rand_long();
		value = value<<32;
		value += rand.rand_long();
		value = value%possibilities; // for initial tricks this is biased.
		iiCardState::GetConstellation(counts, value, newCards, d);
		for (int y = 0; y < numPlayers; y++)
		{
			assert((int)newCards[y].size() == counts[y]);
			for (unsigned int x = 0; x < newCards[y].size(); x++)
			{
				if (forbidden[y][Deck::getsuit(newCards[y][x])])
					legal = false;
			}
		}

		// ignore passing for now
		// prob(hand | plays) = prob(play | hand) * prob(hand) / prob (play)
		// prob(card | plays) = prob(card | p1 & p2 & p3 & p4) = prob(card|p1)*prob(card|p2)...
		// = exp(ln(prob(card|p1)) + ln(prob(card|p2)) ...)
		
		// this isn't quite right, because we need to factor in the prob of passing the card
		// this assumes even probability of passing the card
		std::vector<double> prior;
		for (unsigned int x = 0; x < counts.size(); x++)
			prior.push_back((double)counts[x]/total);
		prob = 0.0;
		// this does card probability based on single card stats
//		for (unsigned int x = 0; x < newCards.size(); x++)
//		{
//			for (unsigned int y = 0; y < newCards[x].size(); y++)
//			{
//				if (newCards[x][y] != Deck::getcard(SPADES, QUEEN))
//					continue;
//				// probability that x has the card given the passes
//				// actual probability, no logs or anything
//				double p = GetCardProbability(x, passes, newCards[x][y], prior);
//				prob += log(p);
//			}
//		}
			 
//		prob = 0;
		for (int x = 0; x < numPlayers; x++)
		{
			double nextProb = GetProbability(x, newCards[x], cgs->t, d);
			if (nextProb == -1)
				legal = false;
			prob += nextProb;

			// this is probability based on 
			if ((passDir != 0) && (numCardsPassed == 3*numPlayers))
			{
				prob += GetPassProbability(passes[x], newCards[x]);
			}
		}
		prob = exp(prob);
	}
	if (cgs->allplayed.has(Deck::getcard(SPADES, QUEEN)))
		prob = 1.0;

	int cnt = 0;
	int has = 0;
	for (unsigned int x = 0; x < newCards.size(); x++)
	{
		for (unsigned int y = 0; y < newCards[x].size(); y++)
		{
			if (originalGame->cards[x].has(newCards[x][y]))
				has++;
			cnt++;
		}
	}
//	if ((has != 0) && (prob != 1))
//		printf("better %d of %d %f %e\n", has, cnt, (float)has/cnt, prob);
	
	for (int y = 0; y < numPlayers; y++)
	{
		for (unsigned int x = 0; x < newCards[y].size(); x++)
		{
			cgs->original[y].set(newCards[y][x]);
			cgs->cards[y].set(newCards[y][x]);
		}
	}

	
	//	printf("Created state:\n");
	//	cgs->Print(1);
	// discard some cards for those who have already done so...but we do it automatically for them
	// but only do this during the initial phase of the game, when we don't know any of their cards;
	// don't want to make the hands consistent
	if (numCardsPassed != 3*numPlayers)
	{
		for (int x = 0; x < numPlayers; x++)
			for (unsigned int y = 0; y < cgs->passes[x].size(); y++) // undo temporarily give us our passes back
				cgs->UntakeCard(x, cgs->passes[x][y]);
		
		int count = 0;
		for (int z = 0; z < numPlayers; z++)
		{
			//int x = (z+firstPlayer)%numPlayers;
			int x = (z+0)%numPlayers;
			count += cgs->passes[x].size();
			//printf("First is %d; curr is %d; %d has already passed %d cards\n", firstPlayer, currPlr, x, cgs->passes[x].size());
			if ((cgs->passes[x].size() < 3) && (count < numCardsPassed))
			{
				card a, b, c;
				//##//X\//
				((HeartsCardPlayer*)cgs->getPlayer(x))->selectPassCards(passDir, a, b, c);
				count++;
				cgs->passes[x].push_back(a);
				cgs->UntakeCard(x, a);
				if (count < numCardsPassed)
				{
					count++;
					cgs->passes[x].push_back(b);
					cgs->UntakeCard(x, b);
				}
				if (count < numCardsPassed)
				{
					count++;
					cgs->passes[x].push_back(c);
					cgs->UntakeCard(x, c);
				}
			}
		}
		assert(count == numCardsPassed);
	}

	// take out the cards that have already been played
	for (int x = 0; x < numCards+1; x++)
	{
		for (int y = 0; y < t[x].curr; y++)
		{
			cgs->cards[t[x].player[y]].clear(t[x].play[y]);
		}
	}

//	printf("----------------created: prob: %e-------------------\n", prob);
//	cgs->Print(1);
	//printf("Set up new GameSate with %d to move\n", cgs->getNextPlayerNum());
	if (prob == 0)
		fprintf(stderr, "Error, returning 0 probability\n");
	return cgs;
}

double advancedIIHeartsState::GetCardProbability(int who, std::vector<card> passes[],
												 card c, std::vector<double> &priors)
{
	std::vector<double> probs;
	double sum = 0;
	
	// everyone starts with equal probability of having the card
	for (unsigned int x = 0; x < priors.size(); x++)
	{
		probs.push_back(priors[x]);
	}
	
	for (unsigned int x = 0; x < priors.size(); x++)
	{
		for (unsigned int y = 0; y < passes[x].size(); y++)
		{
			probs[x] *= cpd.freq1[passes[x][y]][c]*51.0/13.0; // 13/51
		}
	}
	
	for (unsigned int x = 0; x < probs.size(); x++)
	{
		sum += probs[x];
	}
//	for (unsigned int x = 0; x < probs.size(); x++)
//		printf("Probability that (Player %d) has %d = %f\n", x, c, probs[x]/sum);

	return clamp(probs[who]/sum, -10, 1);
}


double advancedIIHeartsState::GetPassProbability(std::vector<card> &passes, std::vector<card> &newCards)
{
	double prob = 0;
//	for (unsigned int x = 0; x < passes.size(); x++)
//	{
//		assert(passes[x] != -1);
//		for (unsigned int y = 0; y < newCards.size(); y++)
//		{
//			prob += log(cpd.freq1[passes[x]][newCards[y]]);
//		}
//	}

	for (unsigned int x = 0; x < passes.size(); x++)
	{
		for (unsigned int z = x+1; z < passes.size(); z++)
		{
			for (unsigned int y = 0; y < newCards.size(); y++)
			{
				//if (newCards[y] == Deck::getcard(SPADES, QUEEN))
				prob += log(cpd.freq2[passes[x]][passes[z]][newCards[y]]*50.0/13.0);
			}
		}
	}
	return clamp(prob, -10, 1);
}

// prob(card c | evidence) = exp(ln(prob(c|a))+ ln(prob(c|b))+ ...)
double advancedIIHeartsState::GetProbability(int player, std::vector<card> &newCards, Trick *t, Deck &d)
{
	if (newCards.size() == 0)
		return 0.0;
	
	double result = 0;
	for (unsigned int x = 0; x < newCards.size(); x++)
	{
//		if (newCards[x] != Deck::getcard(SPADES, QUEEN))
//			continue;
		for (int y = 0; y < numCards; y++)
		{
			double val = GetTrickOdds(player, t[y], newCards[x], d);
			if (val == 0)
				//return -1;
				return val = 1e-20;
			result += log(val);
		}
	}

	return clamp(result, -10, 1);
//	std::vector<double> probs;
//	probs.resize(64);
//	double sum = 0;
//	// get probability for every card
//	for (int x = 0; x < 64; x++)
//	{
//		if (!d.has(x))
//			continue;
//
//		// for each trick / evidence
//		for (int y = 0; y < numCards; y++)
//		{
//			probs[x] += log(GetTrickOdds(player, t[y], x, d));
//		}
//		probs[x] = exp(probs[x]);
//		sum += probs[x];
//	}
//	double ours = 0;
//	for (unsigned int x = 0; x < newCards.size(); x++)
//		ours += probs[newCards[x]];
//
//	return ours/sum;
}

double advancedIIHeartsState::GetTrickOdds(int player, Trick &t, int which, Deck &d)
{
	if (t.curr == 0)
	{
		return 1.0;
	}
	if (t.player[0] == player)
	{
		//assert (cpd.probGivenLead[t.play[0]][which] != 0);
//		if (cpd.probGivenLead[t.play[0]][which] == 0)
//			return 1e-10;
		return clamp(cpd.probGivenLead[t.play[0]][which]*51.0/12.0, -10, 1);
	}
	int myPlay = -1;
	for (int x = 1; x < t.curr; x++)
		if (t.player[x] == player)
			myPlay = x;
	if (myPlay == -1)
		return 1.0;

	//assert(cpd.probGivenLeadAndPlay[t.play[0]][t.play[myPlay]][which] != 0);
//	if (cpd.probGivenLeadAndPlay[t.play[0]][t.play[myPlay]][which] == 0)
//		return 1e-10;
	return clamp(cpd.probGivenLeadAndPlay[t.play[0]][t.play[myPlay]][which]*50.0/12.0, -10, 1);
}


simpleIIHeartsState::simpleIIHeartsState()
:iiHeartsState()
{
}

simpleIIHeartsState::~simpleIIHeartsState()
{
}

GameState *simpleIIHeartsState::getGameState(double &prob)
{
	prob = 1.0;
	
	HeartsGameState *cgs = (HeartsGameState*)originalGame->create();
	int forbidden[MAXPLAYERS][4];
	// set basic information
	//	cgs->numPlayers = numPlayers;
	cgs->setRules(rules);
	cgs->numCards = numCards;
	cgs->trump = trump;
	cgs->passDir = passDir;
	cgs->currTrick = currTrick;
	cgs->special = special;
	cgs->setFirstPlayer(currPlr);
	cgs->currPlr = currPlr;
	cgs->numCardsPassed = numCardsPassed;
	
	for (int x = 0; x < numPlayers; x++)
		for (int y = 0; y < 4; y++)
			forbidden[x][y] = 0;
	
	for (int x = 0; x < numPlayers; x++)
		for (unsigned int y = 0; y < passes[x].size(); y++)
			cgs->passes[x].push_back(passes[x][y]);
	
	cgs->allplayed.setHand(&allplayed);
	//cgs->t = new Trick[numCards+1];
	for (int x = 0; x < numCards+1; x++)
	{
		cgs->t[x].reset(numPlayers, trump);
		for (int y = 0; y < t[x].curr; y++)
		{
			cgs->t[x].AddCard(t[x].play[y], t[x].player[y]);
			if ((y != 0) && (Deck::getsuit(t[x].play[y]) != Deck::getsuit(t[x].play[0])))
			{ // forbidden suit
				forbidden[t[x].player[y]][Deck::getsuit(t[x].play[0])] = 1;
			}
		}
		cgs->t[x].curr = t[x].curr;
	}
	
	for (int x = 0; x < numPlayers; x++)
	{
		Player *p = master->clone();
		cgs->addPlayer(p);
	}
	
	// deal the cards out
	d.Shuffle(rand.rand_long());
	for (int x = 0; x < numPlayers; x++)
	{
		cgs->cards[x].setHand(&cards[x]);
		cgs->taken[x].setHand(&taken[x]);
		cgs->original[x].setHand(&original[x]);
		cgs->played[x].setHand(&played[x]);
		
		d.removeHand(&cgs->cards[x]);
		d.removeHand(&cgs->taken[x]);
		d.removeHand(&cgs->original[x]);
		if (numCardsPassed != numPlayers*3)
		{
			//printf("numCardsPassed = %d\n", numCardsPassed);
			for (unsigned int y = 0; y < cgs->passes[x].size(); y++) // temporarily give us our passes back
			{
				d.clear(cgs->passes[x][y]);
				cgs->TakeCard(x, cgs->passes[x][y]);
				//printf("%d Temporarily taking: ", x);
				//PrintCard(cgs->passes[x][y]);
			}
		}
	}
	
	// test for forced cards from others being short
	for (int x = 0; x < 4; x++)
	{
		int available = -1;
		int forbiddenCount = 0;
		for (int y = 0; y < numPlayers; y++)
		{
			if (numCards-cgs->original[y].count() == 0)
				forbiddenCount++;
			else if (forbidden[y][x] == 1)
				forbiddenCount++;
			else
				available = y;
		}
		if (forbiddenCount == numPlayers-1)
		{
			//printf("Only %d can have %d, giving him the rest!\n", available, x);
			assert(available != -1);
			cgs->cards[available].addSuit(x, d.getSuit(x));
			cgs->original[available].addSuit(x, d.getSuit(x));
			d.setSuit(x, 0);
		}
	}
	
	std::vector<int> counts;
	int total = 0;
	for (int y = 0; y < numPlayers; y++)
	{
		//printf("Player %d needs %d more cards\n", y, numCards-cgs->original[y].count());
		counts.push_back(numCards-cgs->original[y].count());
		total += counts.back();
	}
	uint64_t possibilities = iiCardState::getNumCombos(counts);
	bool legal = false;
	std::vector<std::vector<card> > newCards;
	while (!legal)
	{
		newCards.resize(0);
		legal = true;
		
		uint64_t value = rand.rand_long();
		value = value<<32;
		value += rand.rand_long();
		value = value%possibilities; // for initial tricks this is biased.
		iiCardState::GetConstellation(counts, value, newCards, d);
		for (int y = 0; y < numPlayers; y++)
		{
			assert((int)newCards[y].size() == counts[y]);
			for (unsigned int x = 0; x < newCards[y].size(); x++)
			{
				if (forbidden[y][Deck::getsuit(newCards[y][x])])
					legal = false;
			}
		}
	}
	
	int cnt = 0;
	int has = 0;
	for (unsigned int x = 0; x < newCards.size(); x++)
	{
		for (unsigned int y = 0; y < newCards[x].size(); y++)
		{
			if (originalGame->cards[x].has(newCards[x][y]))
				has++;
			cnt++;
		}
	}
//	if (has != 0)
//		printf("simple %d of %d %f %e\n", has, cnt, (float)has/cnt, prob);

	for (int y = 0; y < numPlayers; y++)
	{
		for (unsigned int x = 0; x < newCards[y].size(); x++)
		{
			cgs->original[y].set(newCards[y][x]);
			cgs->cards[y].set(newCards[y][x]);
		}
	}
	
	
	//	printf("Created state:\n");
	//	cgs->Print(1);
	// discard some cards for those who have already done so...but we do it automatically for them
	// but only do this during the initial phase of the game, when we don't know any of their cards;
	// don't want to make the hands consistent
	if (numCardsPassed != 3*numPlayers)
	{
		for (int x = 0; x < numPlayers; x++)
			for (unsigned int y = 0; y < cgs->passes[x].size(); y++) // undo temporarily give us our passes back
				cgs->UntakeCard(x, cgs->passes[x][y]);
		
		int count = 0;
		for (int z = 0; z < numPlayers; z++)
		{
			//int x = (z+firstPlayer)%numPlayers;
			int x = (z+0)%numPlayers;
			count += cgs->passes[x].size();
			//printf("First is %d; curr is %d; %d has already passed %d cards\n", firstPlayer, currPlr, x, cgs->passes[x].size());
			if ((cgs->passes[x].size() < 3) && (count < numCardsPassed))
			{
				card a, b, c;
				//##//X\//
				((HeartsCardPlayer*)cgs->getPlayer(x))->selectPassCards(passDir, a, b, c);
				count++;
				cgs->passes[x].push_back(a);
				cgs->UntakeCard(x, a);
				if (count < numCardsPassed)
				{
					count++;
					cgs->passes[x].push_back(b);
					cgs->UntakeCard(x, b);
				}
				if (count < numCardsPassed)
				{
					count++;
					cgs->passes[x].push_back(c);
					cgs->UntakeCard(x, c);
				}
			}
		}
		assert(count == numCardsPassed);
	}
	
	//printf("Set up new GameSate with %d to move\n", cgs->getNextPlayerNum());
	if (prob == 0)
		fprintf(stderr, "Error, returning 0 probability\n");
	return cgs;
}

} // namespace hearts
