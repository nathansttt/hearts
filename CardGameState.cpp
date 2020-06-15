#include <cassert>
#include <time.h>
#include "CardGameState.h"

#ifdef __MWERKS__
#include "Diagnostics.h"
#endif

//#define _PRINT_

static unsigned long randPoints[4][32] =
{ 
{ 0x73E15852, 0x54DB3DA1, 0x76F5551E, 0x79DACE40, 0x3C344C2F, 0x54E53189, 0x477C5801, 0x43D6F45A, 
	0x4A432334, 0x7D28582E, 0x70475F46, 0x1031EDFE, 0x57F84ADC, 0x4D4170DD, 0x16569135, 0x2E4D82A5, 
	0x2F4CDD57, 0x4EA04846, 0x6190C8C1, 0x42A80317, 0x4C09E3D6, 0x043EA6DD, 0x27E6053E, 0x3EAFA458, 
	0x7B93EA5F, 0x7B98BE99, 0x1A442A5C, 0x1D2967D9, 0x262DA13A, 0x4D085BE8, 0x3E935841, 0x1A0EF98C
},
{
	0x78ADA871, 0x5382409F, 0x192A6599, 0x6998D286, 0x7F21646B, 0x15665969, 0x2F8045FB, 0x7FDD6C6E, 
	0x4491CBDC, 0x173EECC9, 0x47355681, 0x2DB5F6AA, 0x0728CE52, 0x0AD2A342, 0x2974A884, 0x6D6C9D25, 
	0x72593E1D, 0x550C6019, 0x30947C69, 0x728863FD, 0x5E539D44, 0x0CD77467, 0x1516F9A9, 0x31602F0C, 
	0x40B2A713, 0x422BC273, 0x1462542A, 0x5CC0BC79, 0x6BBD8503, 0x7461B06D, 0x024A9041, 0x646B2D74
},
{
	0x47E3F10D, 0x1B74F5DB, 0x4E03FFFA, 0x47055578, 0x30DB4F44, 0x7D8445F6, 0x46E2C1E6, 0x756D1B20, 
	0x14C332BF, 0x0E181868, 0x232311CA, 0x1BEC0112, 0x18EABBAA, 0x4C97BA4E, 0x09589E37, 0x0B43F9C7, 
	0x21A41A68, 0x39ED1AA0, 0x7DCC5DC4, 0x7FF7B7AC, 0x46C48F07, 0x12E3576D, 0x3157E6B9, 0x0777361B, 
	0x550F19E1, 0x45BA3AE3, 0x6437F294, 0x40CC9EE4, 0x3A1BEB51, 0x668282D5, 0x2537CC58, 0x2589D3C8
},
{
	0x5EC99A6B, 0x01B6F170, 0x0136FE5C, 0x32F87D57, 0x6E6786D5, 0x021E0F7C, 0x6126AE74, 0x3E81FA23, 
	0x2D9E69D7, 0x3D944D76, 0x02DE83C5, 0x2F006D13, 0x044CE4E4, 0x5B879109, 0x1579E702, 0x2EF8B6D2, 
	0x3E090657, 0x4D400A77, 0x5D4F21EE, 0x42D22FD0, 0x65DF7AA9, 0x54F8616F, 0x542CC566, 0x76020C40, 
	0x747A7B14, 0x2C2E1079, 0x0AEFA103, 0x69C2ED5B, 0x6F2E2455, 0x0FCE4137, 0x3996E475, 0x4DF7BEC1 
}
};

static unsigned long randNumbers[4][64] =
{
	{ 0x73D6B0CE,	0xDEFFE50,	0x4CC6A7CB,	0x70EC79A2,	0x2FBB009E,	0x4D96C269,	0x61C39297,	0x1467F26D,
		0x607B041B,	0x14C31C74,	0x6CBC369B,	0x4DEDB8D8,	0x6F665299,	0x1727C359,	0x1AC4ECA6,	0x1AC02A7F,
		0x66D60B1E,	0x31B85805,	0x6C914B9A,	0xA88C0CD,	0x6A889143,	0x750CCB4,	0x5C56F892,	0xED25AB6,
		0x60D313CB,	0xEEAE148,	0x5DE67930,	0x139BBE81,	0x36EB86D,	0x305B0704,	0x8A1EA0B,	0x7745693B,
		0x3E4B0554,	0x556891D7,	0x6831E2DE,	0x6E0605F2,	0x22FF5440,	0x49F57575,	0x26DF85F,	0x37A585B,
		0x5EB891E9,	0x6F2A2EFB,	0x51681133,	0x4E1EE482,	0x651F254,	0x6C2CFDD9,	0x68DF0F01,	0x6D27FD72,
		0x1DE555DF,	0x55705A9B,	0x77B0BE3F,	0x86DE722,	0x5CC1274F,	0x5407B6D1,	0x174041D8,	0x3D943B1A,
		0x62F29819,	0x7526BB09,	0x512FF99B,	0x66615086,	0x2581C20D,	0x59D1E3A7,	0x5DA6B9C2,	0x63CCC762
	},
	{
		0x53CD2745, 0x3AD7A2CC, 0x38D1B0DE, 0x7B69AE50, 0x71A405E1, 0x4EFAFD09, 0x4492C835, 0x51E7D91E, 
		0x229A4CD5, 0x40C66CA, 0x6E92D95A, 0x6367A7B1, 0xC71ED7D, 0x4CCB21A8, 0x7FE8C9B4, 0x101DF5D9, 
		0x14E95F85, 0x1D20F4B, 0x615621CE, 0x2B2BA64C, 0x176BBFE7, 0x37A20D97, 0x459E811E, 0x1777B0A9, 
		0x77C135E, 0x7DA192A4, 0x4D01A389, 0xC60F427, 0x752E4B18, 0x507CA32A, 0x34E2B5F5, 0x48FB725E, 
		0xB5445F7, 0x6DB466D3, 0x446520AE, 0x7CF84BD8, 0x3CAF63DC, 0x8F7E8E3, 0x4EE024F6, 0x5F49B0B2, 
		0xD044FAD, 0x3D72FE50, 0x42B15863, 0x19763D2A, 0xA3E1FF8, 0x429A2217, 0x29943304, 0x1F277F7E, 
		0x446C3163, 0xAEA54D2, 0x4A5325CA, 0x5BD7F14A, 0x428C6269, 0xFF1A6E8, 0x734FA1F3, 0x4A0875C7, 
		0xD93398D, 0x4051457C, 0x566969EE, 0x2C184A5, 0x10CDE8A6, 0xB4C1FE3, 0x4BBCF703, 0x2881C1CE
	},
	{
		0x5B1A124, 0x49F0DBFB, 0x17A6FA6B, 0x000C171A, 0x3763295F, 0x127C4BA5, 0x4CA06674, 0x23780100, 
		0x6E842C55, 0x4C35F97C, 0x7F6B5FE4, 0x8B0B8312, 0x7A7C5FB1, 0x04FC57F6, 0x5BD45AEE, 0x32780F4D, 
		0x015209F9, 0x6F15AB70, 0x31F7E961, 0x2B85AD8E, 0x2831F4DF, 0x6CFA4303, 0x5186B5EB, 0x656667A3, 
		0x3492FD38, 0x78CE223D, 0x7C661DE5, 0x5EEE2AAE, 0x0D6A5B9B, 0x38A60FB4, 0x088C8D1F, 0x131BFCBF, 
		0x0296EBAF, 0x2033878A, 0x132813D9, 0x39FA150E, 0x32AFD32F, 0x5FC87A4E, 0x5D72160F, 0x2133FF85, 
		0x2BFE73CA, 0x5CDD75F3, 0x2C3F8297, 0x267AD37B, 0x61D9CDE9, 0x0813DD85, 0x58F2E2C8, 0x632BD7E2, 
		0x772988F6, 0x0AEACC2A, 0x0EB18571, 0x1F5B7DD5, 0x77E50F2D, 0x60383B5C, 0x04C1E578, 0x2C780C65, 
		0x59065D99, 0x0128035D, 0x0B663713, 0x6670B934, 0x39CE1312, 0x13F2C433, 0x798CB5F3, 0x299E6271
	},
	{
		0x5C4E2852, 0x3C93BE7B, 0x06139E75, 0x51622754, 0x3F0010A4, 0x767A8DA0, 0x41A7587C, 0x55727BA8, 
		0x594C9B84, 0x0A7715D6, 0x1636912C, 0x64D0B0D2, 0x6E0E25B2, 0x0772F8CD, 0x26B79C76, 0x44F80DF5, 
		0x3E15168B, 0x3E37A2D5, 0x49B8ACB6, 0x198B5775, 0x1AF50B58, 0x3A765CE3, 0x333D1346, 0x2B532D73, 
		0x7B87AA06, 0x440BFFB5, 0x0B1EE8B3, 0x493085D1, 0x7E4C5916, 0x1BCD5C33, 0x576526CF, 0x5A9A8168, 
		0x58611AAF, 0x5D78C544, 0x2BFCA8BD, 0x17612B53, 0x53F352E5, 0x6DA40139, 0x6CD3A6FC, 0x2D3FEE69, 
		0x781B170F, 0x030A3828, 0x12109F3B, 0x66293CC2, 0x0A7D30F5, 0x38C83BB2, 0x2B214AB7, 0x48924781, 
		0x76FFDE87, 0x74D9F76D, 0x621D9EF6, 0x11F4E9DF, 0x2F505450, 0x155AB23D, 0x3D481752, 0x2AD7FE56, 
		0x5966B1F2, 0x48670006, 0x74088427, 0x57B30B08, 0x64345C39, 0x4B6DAAF6, 0x324D8C71, 0x0E34A0FE
	}
};

int CardGame::Play()
{
	// for Sheepshead you can pass cards here. In other games
	// you might do things like set trump, etc.
	if (!started) {
		started = true;
	}
	bool done = false;
	while (!done)
	{
		theGame->Reset();
		//((CardGameState*)theGame)->DealCards();
		theGame->Print();
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
	}
	return 0;
}

void CardGame::doOnePlay()
{
	if (!started)
	{
		started = true;
		theGame->Reset();
	}

	if (theGame->Done())
	{
		theGame->Reset();
	}

	theGame->Print(1);
	Game::doOnePlay();
}

CardGameState::CardGameState(std::vector<std::vector<card> > &theCards, int trmp, int seed, int spec, int nc)
{
	for (unsigned int x = 0; x < theCards.size(); x++)
		addPlayer(new Player(0));
	t = 0;
	special = spec;
	trump = trmp;
	numCards = nc;
	SEED = seed;
	SetInitialCards(theCards);
}

CardGameState::CardGameState(int trmp, int seed, int spec, int nc)
	:GameState()
{
	t = 0;
	special = spec;
	trump = trmp;
	numCards = nc;
	SEED = seed;
	d.Shuffle(SEED);
	Reset();
	//	Print();
}

CardGameState::~CardGameState()
{
	delete [] t;
}

CardGameState *CardGameState::create()
{
	return new CardGameState(trump, -1, special, numCards);
}

Move *CardGameState::allocateMoreMoves(int n)
{
	Move *ret = 0;
	for (int x = 0; x < n; x++)
		ret = new CardMove(ret);
	return ret;
}

void CardGameState::Reset(int NEWSEED)
{
	//printf("Calling void CardGameState::Reset()\n");
	//NEWSEED = 1171648868;
	if (NEWSEED == -1)
	{
		//printf("Starting new card game with SEED: %d\n", SEED);
		d.Shuffle(SEED++);
	}
	else {
		//printf("Starting new card game with SEED: %d\n", NEWSEED);
		d.Shuffle(NEWSEED);
	}
	currPlr = currTrick = 0;
	firstPlayer = 0;
	
	delete [] t;
	t = 0;
	allplayed.reset();
	for (unsigned int x = 0; x < MAXPLAYERS; x++)
	{
		cards[x].reset();
		taken[x].reset();
 		original[x].reset();
		played[x].reset();
	}
	if ((numCards == -1) && (numPlayers > 0))
	{
		numCards = d.count()/numPlayers;
	}
	//printf("Allocating %d tricks\n", numCards+1);
	t = new Trick[numCards+1];
	for (int x = 0; x < numCards+1; x++)
		t[x].reset(numPlayers, trump);
	this->DealCards();
}

void CardGameState::SetInitialCards(std::vector<std::vector<card> > &theCards)
{
	currPlr = currTrick = 0;
	firstPlayer = 0;

	allplayed.reset();
	numPlayers = theCards.size();
	for (unsigned int x = 0; x < numPlayers; x++)
	{
		cards[x].reset();
		taken[x].reset();
		original[x].reset();
		played[x].reset();
	}
	for (unsigned int x = 0; x < theCards.size(); x++)
	{
		for (unsigned int y = 0; y < theCards[x].size(); y++)
		{
			original[x].set(theCards[x][y]);
			cards[x].set(theCards[x][y]);
		}
	}
	delete [] t;
	numCards = theCards[0].size();
	t = new Trick[numCards+1];
	for (int x = 0; x < numCards+1; x++)
		t[x].reset(numPlayers, trump);
}

void CardGameState::DealCards()
{
	//printf("Calling CardGameState::DealCards()\n");   

#ifdef _PRINT_
	printf("Initializing tricks...\n");
#endif

	
#ifdef _PRINT_
	printf("Dealing Cards! (%d each -- %d players)\n", numCards, numPlayers);
#endif
	if (numCards == -1)
	{
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
		{
			for (unsigned int y = 0; y < numPlayers; y++)
			{
				card c = d.Deal();
				TakeCard(y, c);

#ifdef _PRINT_
				printf("%d gets : %d", x+1, c);
				PrintCard(c);
				printf("\n");
#endif
			}
		}
	}
	while (d.count() > 0)
		allplayed.set(d.Deal());

#ifdef _PRINT_
	Print();
#endif
	SEED+=81;
}

void CardGameState::TakeCard(int who, card c)
{
	cards[who].set(c);
	original[who].set(c);
}

void CardGameState::UntakeCard(int who, card c)
{
	cards[who].clear(c);
	original[who].clear(c);
}


Move *CardGameState::getRandomMove()
{
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
/*
bool CardGameState::moreThanOneMove()
{
	uint16_t theSuit = cards[me].getSuit(suit);
	uint16_t allSuit = allplayed.getSuit(suit);
	uint16_t fullSuit = (getFullDeck()>>(16*suit))&0xFFFF;
	uint16_t result = (theSuit|allSuit)
}
*/
Move *CardGameState::getMoves()
{
	int me = getNextPlayerNum();
	//Player *who = getNextPlayer();
	const Trick *tr = getCurrTrick();
	//CardMove m;
	Move *ret=0;

	// following in suit
	if ((tr->curr != 0) && (cards[me].hasSuit(Deck::getsuit(tr->play[0]))))
	{
		int ledSuit = Deck::getsuit(tr->play[0]);
		uint16_t theSuit = cards[me].getSuit(ledSuit);
		uint16_t allSuit = allplayed.getSuit(ledSuit);
		int curr = Deck::getcard(ledSuit, 0);
		while (theSuit)
		{
			if (theSuit&1)
			{
				CardMove *cm = (CardMove*)getNewMove();
				cm->init(curr, kFollowCard, me, ret);
				ret = cm;
				while ((theSuit&1) || (allSuit&1))
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
		if (((tr->curr == 0) && (special == y) && (allplayed.getSuit(special))) ||
				((tr->curr == 0) && (special != y)) ||
				(tr->curr != 0))
		{
			uint16_t theSuit = cards[me].getSuit(y);
			uint16_t allSuit = allplayed.getSuit(y);
			int curr = Deck::getcard(y, 0);
			while (theSuit)
			{
				if (theSuit&1)
				{
					CardMove *cm = (CardMove*)getNewMove();
					cm->init(curr, (t->curr==0)?kLeadCard:kSloughCard, me, ret);
					ret = cm;
					while ((theSuit&1) || (allSuit&1))
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

	// here we only really need to try the special suit...
	uint16_t theSuit = cards[me].getSuit(special);
	uint16_t allSuit = allplayed.getSuit(special);
	int curr = Deck::getcard(special, 0);
	while (theSuit)
	{
		if (theSuit&1)
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(curr, (t->curr==0)?kLeadCard:kSloughCard, me, ret);
			ret = cm;
			while ((theSuit&1) || (allSuit&1))
			{ theSuit>>=1; allSuit>>=1; curr++; }
		}
		curr++;
		theSuit>>=1;
		allSuit>>=1;			
	}
	
	return ret;
}

Move *CardGameState::getAllMoves()
{
	int me = getNextPlayerNum();
	//Player *who = getNextPlayer();
	const Trick *tr = getCurrTrick();
	//CardMove m;
	Move *ret=0;
	
	// following in suit
	if ((tr->curr != 0) && (cards[me].hasSuit(Deck::getsuit(tr->play[0]))))
	{
		int ledSuit = Deck::getsuit(tr->play[0]);
		uint16_t theSuit = cards[me].getSuit(ledSuit);
		uint16_t allSuit = allplayed.getSuit(ledSuit);
		int curr = Deck::getcard(ledSuit, 0);
		while (theSuit)
		{
			if (theSuit&1)
			{
				CardMove *cm = (CardMove*)getNewMove();
				cm->init(curr, kFollowCard, me, ret);
				ret = cm;
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
		if (((tr->curr == 0) && (special == y) && (allplayed.getSuit(special))) ||
			((tr->curr == 0) && (special != y)) ||
			(tr->curr != 0))
		{
			uint16_t theSuit = cards[me].getSuit(y);
			uint16_t allSuit = allplayed.getSuit(y);
			int curr = Deck::getcard(y, 0);
			while (theSuit)
			{
				if (theSuit&1)
				{
					CardMove *cm = (CardMove*)getNewMove();
					cm->init(curr, (t->curr==0)?kLeadCard:kSloughCard, me, ret);
					ret = cm;
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
	
	// here we only really need to try the special suit...
	uint16_t theSuit = cards[me].getSuit(special);
	uint16_t allSuit = allplayed.getSuit(special);
	int curr = Deck::getcard(special, 0);
	while (theSuit)
	{
		if (theSuit&1)
		{
			CardMove *cm = (CardMove*)getNewMove();
			cm->init(curr, (t->curr==0)?kLeadCard:kSloughCard, me, ret);
			ret = cm;
		}
		curr++;
		theSuit>>=1;
		allSuit>>=1;			
	}
	
	return ret;
}

void CardGameState::Print(int val) const
{
	printf("Game State: %d players, currPlr %d, currTrick %d, numCards %d\n",
				 numPlayers, currPlr, currTrick, numCards);

	for (unsigned int x = 0; x < numPlayers; x++)
	{
		if (val || ((unsigned int)getNextPlayerNum() == x))
		{
			printf("Player %d (%s) cards:\n", x, getNextPlayer()->getName());
			cards[x].print();
		}
	}
	if (allplayed.count() != 0 || currTrick != 0 || t[0].curr != 0)
	{
		printf("Taken cards:\n");
		for (unsigned int x = 0; x < numPlayers; x++)
		{
			taken[x].print();
		}
		printf("Played cards:\n");
		for (unsigned int x = 0; x < numPlayers; x++)
		{
			played[x].print();
		}
		printf("All played cards:\n");
		allplayed.print();
		
		for (int x = 0; x < currTrick; x++)
		{
			t[x].print();
		}
		if ((t) && (currTrick < numCards))
		{
			printf("Current trick:\n");
			t[currTrick].print();
		}
	}
}

Player *CardGameState::getNextPlayer() const
{
	return getPlayer(currPlr);
}

int CardGameState::getNextPlayerNum() const
{
	return currPlr;
}

Player *CardGameState::getPreviousPlayer() const
{
	if (t[currTrick].curr == 0)
	{
		if (currTrick == 0)
			return 0;
		return getPlayer(t[currTrick-1].player[t[currTrick-1].curr-1]);
	}
	return getPlayer(t[currTrick].player[t[currTrick].curr-1]);
}

int CardGameState::getPreviousPlayerNum() const
{
	if (t[currTrick].curr == 0)
	{
		if (currTrick == 0)
			return -1;
		return t[currTrick-1].player[t[currTrick-1].curr-1];
	}
	return t[currTrick].player[t[currTrick].curr-1];
}


void CardGameState::ApplyMove(Move *move)
{
	//	static int cnt = 0;
	CardMove *m = (CardMove*)move;

	assert(m->player == currPlr);

	//	printf("MOVE: %d (%d) by %d\n", m->c, cnt++, m->player);
	/*
		printf("Trying ");
		PrintCard(m->c);
		printf("\n");
	*/
	if (!cards[currPlr].has(m->c))
	{
		Print(1);
		printf("Trying to play card I (%d/%d) don't have! (%d)\n", m->player, currPlr, m->c);
		assert(cards[currPlr].has(m->c));
	}
	
	card last = t[currTrick].WinningCard(); // get previous winning card
	t[currTrick].AddCard(m->c, currPlr);    // add our card to the trick
#ifdef _PRINT_
	printf("After playing, the winning card is %d\n", t[currTrick].WinningCard());
#endif
	if (t[currTrick].WinningCard() != m->c) // if we're winning, we consider that out card was already played
	{
		allplayed.set(m->c);
#ifdef _PRINT_
		printf("%d isn't winning, so we set it played\n", m->c);
#endif
	}
	else if (last != -1) // otherwise we're winning the trick, and we consider prev winner already played
	{
		allplayed.set(last);
#ifdef _PRINT_
		printf("%d is winning, so we set %d winning instead\n", m->c, last);
#endif
	}

	cards[currPlr].clear(m->c);
	played[currPlr].set(m->c);

	if (t[currTrick].Done()) {
		currPlr = t[currTrick].Winner();
		allplayed.set(t[currTrick].WinningCard());
		for (int x = 0; x < t[currTrick].curr; x++) {
			taken[currPlr].set(t[currTrick].play[x]);
			//allplayed.set(t[currTrick].play[x]);
		}
		currTrick++;
	}
	else
		currPlr = (currPlr+1)%numPlayers;
//#ifdef _PRINT_
//	printf("=====================Apply MOVE!=======================\n");
//	PrintCard(m->c);
//	printf("---\n");
//	Print(1);
//	printf("<<<<<<<<<<<<<<<<<<<<<Apply MOVE!>>>>>>>>>>>>>>>>>>>>>>>\n");
	//#endif
}

void CardGameState::UndoMove(Move *move)
{
	CardMove *m = (CardMove*)move;

//	printf("UNDO: %d by %d\n", m->c, m->player);

	if (t[currTrick].curr == 0)
	{
		currTrick--;

		if (currTrick >= 0)
		{
			currPlr = (t[currTrick].player[t[currTrick].curr-1]+1)%numPlayers;
			for (int x = 0; x < t[currTrick].curr; x++)
			{
				taken[t[currTrick].Winner()].clear(t[currTrick].play[x]);
				//allplayed.clear(t[currTrick].play[x]);
			}
		}
		else {
			printf("!!!!!!!!!!!!!!!!!!I shouldn't get here.\n");
			currPlr = 1;
			currTrick = 0;
		}
	}
	t[currTrick].RemoveCard();

	allplayed.clear(m->c);
  card last = t[currTrick].WinningCard();
  if (last != -1)
		allplayed.clear(last);
	currPlr = (currPlr + numPlayers - 1)%(numPlayers);
	cards[currPlr].set(m->c);
	played[currPlr].clear(m->c);

	
	//#ifdef _PRINT_
//	printf("=====================UNDO MOVE!=======================\n");
//	PrintCard(m->c);
//	printf("---\n");
//	Print(1);
//	printf("<<<<<<<<<<<<<<<<<<<<<UNDO MOVE!>>>>>>>>>>>>>>>>>>>>>>>\n");
	//#endif
}

bool CardGameState::Done() const
{
	if (currTrick == numCards)
		return true;
	return false;
}

Move *CardGameState::getMoveFromHash(uint32_t hash)
{
	return new CardMove(hash, -1, 0);
}

// TOTALLY REWRITE THIS?!?
uint32_t CardGameState::getMoveHash(const Move *m)
{
	CardMove *cm = (CardMove*)m;
	return cm->c;//randNumbers[cm->player][cm->c]/*|randPoints[0][cm->context]*/;
//	const Trick *tr = getCurrTrick();
//	unsigned long lead;
//
//	if (tr->curr == 0)
//	{
//		lead = 64; // we lead
//	}
//	else {
//		lead = getRelativeRank(tr->WinningCard());//t->play[0];//
//		//PrintCard(lead); printf(" ");
//	}
//	//printf("Getting card move hash: "); PrintCard(cm->c); printf("\n");
//	return lead*64+getRelativeRank(cm->c);
}

card CardGameState::getRelativeRank(card c)
{
	return c;
	uint16_t allSuit = allplayed.getSuit(Deck::getsuit(c));
	card position = 0;
	for (int x = 0; x < Deck::getrank(c); x++)
	{
		if ((allSuit&1) == 0)
			position++;
		allSuit>>=1;
	}
	return Deck::getcard(Deck::getsuit(c), position);
}

HashState *CardGameState::getHashState(HashState *mem)
{
	if (getCurrTrick()->curr != 0)
		return 0;
	HashState *hs;
	cardHashState *cHS;
	CardPlayer *p[6];
	uint64_t hands[6];

	if (mem)
	{
		hs = mem;
		cHS = (cardHashState*)mem->ghs;
	}
	else {
		hs = new HashState();
		cHS = new cardHashState();
		hs->ghs = cHS;
	}
	
	cHS->nextp = getNextPlayerNum();
	cHS->nump = getNumPlayers();

	cHS->numCards = 0;
	for (int w = 0; w < cHS->nump; w++)
	{
		p[w] = (CardPlayer*)getPlayer(w);
		cHS->cards[w] = 0;
		cHS->pts[w] = (int)score(w);//(int)p[w]->score(w);//taken[w].count()/cHS->nump;//

		cHS->numCards = (cHS->numCards<<16)|
					((cards[w].suitCount(0)&0xF)<<0)|
					((cards[w].suitCount(1)&0xF)<<4)|
					((cards[w].suitCount(2)&0xF)<<8)|
					((cards[w].suitCount(3)&0xF)<<12);

		hands[w] = cards[w].getHand();
	}
	
	for (int x = 0; x < 64; x++)
	{
		if ((x%16)==0)
		{
			for (int w = 0; w < cHS->nump; w++)
				cHS->cards[w] <<= 1;
		}
		for (int w = 0; w < cHS->nump; w++)
		{
			if (hands[w]&1)
			{
				cHS->cards[w]++;
				for (int y = 0; y < cHS->nump; y++)
					cHS->cards[y] = cHS->cards[y]<<1;
			}
			hands[w] = hands[w]>>1;
		}
	}

	return hs;
}
/*
partition *CardGameState::getPartition(int me, double value)
{
	Player *who = getPlayer(me);
	return new cardPartition(this, who->score(me), value);
}

HashState *CardGameState::getPartitionHashState(GameState *g, int me)
{
	if ((getCurrTrick()->curr != 0) || (currTrick == numCards))
		return 0;
	HashState *hs = new HashState();
	hs->ghs = new cardPartitionHashState(g, getPlayer(me)->score(me));
	return hs;
}
*/

iiGameState *CardGameState::getiiGameState(bool consistent, int who, Player *playerModel)
{
	if (playerModel == 0)
		playerModel = (Player*)getPlayer(who);
	iiCardState *cs = (iiCardState *)playerModel->getiiModel();
	cs->init(trump, special, numCards, numPlayers, SEED++);

	cs->master = playerModel;
	cs->originalGame = this;
	cs->rules = rules;

	if (t[0].curr != 0)
		cs->currPlr = t[0].player[0];
	else
		cs->currPlr = currPlr;
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
	return cs;
}

void iiCardState::init(int Trump, int Spec, int nc, int players, int seed)
{
	trump = Trump;
	special = Spec;
	numCards= nc;
	numPlayers = players;
	SEED = seed;
	//SEED = clock()+time((time_t*)0);
	rand.srand(SEED);
	currTrick = currPlr = 0;
	t = new Trick[numCards+1];
	for (int x = 0; x < numCards+1; x++)
		t[x].reset(numPlayers, trump);
	for (int x = 0; x < numPlayers; x++)
	{
		cards[x].reset();
		taken[x].reset();
		original[x].reset();
		played[x].reset();
		allplayed.reset();
	}
}

iiCardState::~iiCardState()
{
	delete [] t;
}

uint64_t iiCardState::getNumCombos(std::vector<int> &counts)
{
	int available = 0;
	for (unsigned int x = 0; x < counts.size(); x++)
		available += counts[x];
	uint64_t result = 1;
	for (unsigned int x = 0; x < counts.size(); x++)
	{
		if (counts[x] > 0)
		{
			result *= choose(available, counts[x]);
			available-=counts[x];
		}
	}
	return result;
}

uint64_t iiCardState::getNumCombos(std::vector<int> &counts, Deck &d)
{
	int available = d.count();
//	for (unsigned int x = 0; x < counts.size(); x++)
//		available += counts[x];
	uint64_t result = 1;
	for (unsigned int x = 0; x < counts.size(); x++)
	{
		if (counts[x] > 0)
		{
			result *= choose(available, counts[x]);
			available-=counts[x];
		}
	}
	return result;
}

void iiCardState::GetConstellation(std::vector<int> &counts, uint64_t arrangement, std::vector<std::vector<card> > &theCards, Deck &d)
{
	int available = d.count();
//	for (unsigned int x = 0; x < counts.size(); x++)
//		available += counts[x];
	theCards.resize(0);
	theCards.resize(counts.size());

//	assert((int)d.count() == available);
	std::vector<card> actualCards;
	for (int x = 0; x < 64; x++)
		if (d.has(x))
			actualCards.push_back(x);
	
	for (unsigned int x = 0; x < counts.size(); x++)
	{
		theCards[x].resize(counts[x]);
		if (counts[x] > 0)
		{
			uint64_t possibilities = choose(available, counts[x]);
			uint64_t here = arrangement%possibilities;
			arrangement /= possibilities;
			getCards(here, theCards[x], available, counts[x]);
//			uint64_t index = getIndexFromCards(theCards[x], available, counts[x]);
//			assert(index == here);
			available -= counts[x];
		}
		else {
		}
	}
	for (unsigned int x = 0; x < theCards.size(); x++)
	{
		std::vector<int> toRemove;
		for (unsigned int y = 0; y < theCards[x].size(); y++)
		{
			int cnt = 0;
			for (unsigned int z = 0; z < actualCards.size(); z++)
			{
				if (actualCards[z] == -1)
					continue;
				if (theCards[x][y] == cnt)
				{
					theCards[x][y] = actualCards[z];
					toRemove.push_back(z);
					break;
				}
				cnt++;
			}
		}
		while (toRemove.size() > 0)
		{
			actualCards[toRemove.back()] = -1;
			toRemove.pop_back();
		}
	}
}

uint64_t iiCardState::getIndexFromCards(std::vector<card> &theCards, int available, int thisPlayer)
{
	uint64_t index=0;
	
	for (int x = 0; x < thisPlayer; x++)
    {
		int offset, used;
		if (x != 0) {
			offset = theCards[x]-theCards[x-1]-1;
			used = theCards[x-1]+1;
		}
		else {
			offset = theCards[0];
			used = 0;
		}
		for (int y = 1; y <= offset; y++)
			index+=choose(available-used-y, thisPlayer-1-x);
		//printf("%ld  ", index);
    }
	//printf("\n");
	return index;
}

void iiCardState::getCards(uint64_t val, std::vector<card> &theCards, int available, int thisPlayer)
{
	theCards.resize(thisPlayer);
	for (int y = 0; y < thisPlayer; y++)
    {
		// assume all pieces up to y are set...
		// we put the others in a row, and increase the
		// next pieces until it goes over the value
		// we are trying to reach
		if (y == 0)
			theCards[0] = 0;
		else
			theCards[y] = theCards[y-1]+1;
		for (int x = theCards[y]; x <= available-thisPlayer+y; x++)
		{
			theCards[y] = x;
			for (int z = y+1; z < thisPlayer; z++)
				theCards[z] = theCards[z-1]+1;
			if (getIndexFromCards(theCards, available, thisPlayer) > val)
			{
				theCards[y]--;
				break;
			}
			
		}
    }
}

uint64_t iiCardState::choose(int n, int k)
{
	static std::vector<std::vector<uint64_t> > lookups;

	if (k > n)
        return 0;
	
//    if (k > n/2)
//        k = n-k; // Take advantage of symmetry

	if (n >= (int)lookups.size())
		lookups.resize(n+1);
	if (k >= (int)lookups[n].size())
		lookups[n].resize(k+1);
	if (lookups[n][k] != 0)
		return lookups[n][k];

	//printf("%d choose %d\n", n, k);
	
    long double accum = 1;
    for (int i = 1; i <= k; i++)
		accum = accum * (n-k+i) / i;
	
    lookups[n][k] = (uint64_t)(accum+0.5);
	return accum + 0.5; // avoid rounding error
	
//	uint64_t ans=1;
//	for (int x = 0; x < m; x++)
//		ans=(ans*(n-x))/(x+1);
//	return ans;
}


void iiCardState::Played(Move *move)
{
	CardMove *m = (CardMove*)move;
	if (m->player != currPlr)
	{
		printf("Oh No @@@ not my turn %d/%d\n", m->player, currPlr);
            fflush(stdout);
		exit(0);
	}
	card last = t[currTrick].WinningCard(); // get previous winning card
	t[currTrick].AddCard(m->c, currPlr);    // add our card to the trick

#ifdef _PRINT_
	printf("After playing, the winning card is %d\n", t[currTrick].WinningCard());
#endif
	if (t[currTrick].WinningCard() != m->c) // if we're winning, we consider that out card was already played
	{
		allplayed.set(m->c);
#ifdef _PRINT_
		printf("%d isn't winning, so we set it played\n", m->c);
#endif
	}
	else if (last != -1) // otherwise we're winning the trick, and we consider prev winner already played
	{
		allplayed.set(last);
#ifdef _PRINT_
		printf("%d is winning, so we set %d winning instead\n", m->c, last);
#endif
	}
	
	
	original[currPlr].set(m->c);
	cards[currPlr].clear(m->c);
	played[currPlr].set(m->c);
	if (t[currTrick].Done()) {
		currPlr = t[currTrick].Winner();
		for (int x = 0; x < t[currTrick].curr; x++) {
			taken[currPlr].set(t[currTrick].play[x]);
			allplayed.set(t[currTrick].play[x]);
		}
		currTrick++;
	}
	else
		currPlr = (currPlr+1)%numPlayers;
}

void iiCardState::Unplayed(Move *move)
{
	CardMove *m = (CardMove*)move;

	if (t[currTrick].curr == 0)
	{
		currTrick--;

		if (currTrick >= 0) {
			currPlr = (t[currTrick].player[t[currTrick].curr-1]+1)%numPlayers;
			for (int x = 0; x < t[currTrick].curr; x++) {
				taken[t[currTrick].Winner()].clear(t[currTrick].play[x]);
				allplayed.clear(t[currTrick].play[x]);
			}
		}
		else {
			printf("!!!!!!!!!!!!!!!!!!I shouldn't get here.\n");
			currPlr = 1;
			currTrick = 0;
		}
	}
	t[currTrick].RemoveCard();

	allplayed.clear(m->c);
  card last = t[currTrick].WinningCard();
  if (last != -1)
		allplayed.clear(last);

	currPlr = (currPlr + numPlayers - 1)%(numPlayers);
	// oh no; if we knew they had the card, we'll forget it now!!
	original[currPlr].clear(m->c);
	played[currPlr].clear(m->c);
}

void iiCardState::hasCard(card c, int who)
{
	cards[who].set(c);
	original[who].set(c);
}

void iiCardState::hasCards(uint64_t c, int who)
{
	cards[who].addHand(c);
	original[who].addHand(c);
}

GameState *iiCardState::getGameState(double &prob)
{
	prob = 1.0;
	CardGameState *cgs = originalGame->create();
	int forbidden[MAXPLAYERS][4];
	// set basic information
	//	cgs->numPlayers = numPlayers;
	cgs->setRules(rules);
	cgs->numCards = numCards;
	cgs->trump = trump;
	//cgs->currPlr = currPlr;
	cgs->currTrick = currTrick;
	cgs->special = special;
	cgs->setFirstPlayer(currPlr);

	for (int x = 0; x < numPlayers; x++)
		for (int y = 0; y < 4; y++)
			forbidden[x][y] = 0;

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
		}
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
					cgs->original[x].set(c);
					cgs->cards[x].set(c);
				}
			}
			if (count <= 0)
				break;
		}
		if (count <= 0)
			continue;
		// if I get here I'm done, otherwise my continue sends me back to the front...
		// and starts over!
		break;
	}
//	cgs->movesSoFar = totalMoves;

//	printf("Created state:\n");
//	cgs->Print(1);
#ifdef __MWERKS__
	for (int x = 0; x < numPlayers; x++)
	{
		char msg[255];
		sprintf(msg, "Player %d has %X in spades", x, cgs->cards[x].getSuit(0));
		ai_debug(msg);
	}
#endif
	return cgs;
}

CardPlayer::CardPlayer(Algorithm *alg)
	:Player(alg)
{
//	FILE *f = fopen("ranks.move.saved", "r");
//	if (f)
//	{
//		for (int x = 0; x < 52; x++)
//			fscanf(f, "%llu\n", &ranks[x]);
//		fclose(f);
//	}
//	for (int x = 0; x < tSIZE; x++)
//		ranks[x] = 0;
}

CardPlayer::~CardPlayer()
{
/*	long64 tmp[tSIZE];
	for (int x = 0; x < tSIZE; x++)
		tmp[x] = 0;

	FILE *f = fopen("ranks.move", "r");	if (f)
	{
		for (int x = 0; x < tSIZE; x++)
			fscanf(f, "%llu\n", &tmp[x]);
		fclose(f);
	}
	f = fopen("ranks.move", "w+");
	if (f) {
		for (int x = 0; x < tSIZE; x++)
			fprintf(f, "%llu\n", ranks[x]+tmp[x]);
		fclose(f);
	}*/
}

int CardPlayer::getNextDepth(int curr)
{
	int np = g->getNumPlayers();
	CardGameState *cs = (CardGameState *)g;
	const Trick *tr = cs->getCurrTrick();
	if (curr == -1) {
		return (np-tr->curr);
	}
	return curr+np;
}

unsigned int CardPlayer::getMaxDepth()
{
	unsigned int total = 0;
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
		total += ((CardGameState*)g)->cards[x].count();
	return total;
}

Move *CardMove::clone(GameState *g) const
{
	CardMove *cm = (CardMove*)g->getNewMove();
	cm->init(c, context, player, (next==0)?(0):(next->clone(g)));
	return cm;
}

Move *CardMove::clone() const
{
	CardMove *cm = new CardMove();
	cm->init(c, context, player, (next==0)?(0):(next->clone()));
	//cm->init(c, context, player, 0/*(next==0)?(0):(next->clone())*/);
	return cm;
}

void CardMove::Print(int ex, FILE *f)
{
	PrintCard(c, f);
	fprintf(f, " ");
	if (ex == 0) {
		//if (next) {
		//	next->Print(ex, f);
		//}
		//else
		//	fprintf(f, "\n");
	}
	else {
		//fprintf(f, "(%d-%d) ", player, context);		
		if (next) {
			next->Print(ex, f);
		}
		else
			fprintf(f, "\n");
	}
}

void Deck::print() const
{
	for (int x = 0; x < 4; x++)
	{
		uint16_t suitcards = getSuit(x);
		uint16_t counter = 0;
		while (suitcards)
		{
			if (suitcards&0x1)
			{
				card c = getcard(x, counter);
				PrintCard(c);
				printf(" ");
			}
			suitcards >>= 1;
			counter++;
		}
	}
	printf("\n");
}

//__builtin_clzl
uint32_t Deck::count() const
{
//#ifdef __builtin_popcountl
//	return __builtin_popcountl(cards);
//#endif
	unsigned short answer = 0;
	for (int x = 0; x < 4; x++)
		answer += suitCount(x);
	return answer;
}

uint32_t Deck::suitCount(int which) const
{
	uint16_t suit_count = 0;
	if (counts[which] != -1)
		return counts[which];
	uint16_t tmp = getSuit(which);
//#ifdef __builtin_popcountl
//	counts[which] = __builtin_popcountl(tmp);
//	return counts[which];
//#endif
	while (tmp)
	{
//		if (tmp&1)
//			suit_count++;
//		tmp = tmp>>1;
		suit_count++;
		tmp = tmp&(tmp-1);
	}
	counts[which] = suit_count;
	return suit_count;
}

card Deck::suitHigh(int which) const
{
	unsigned short tmp = getSuit(which);
	if (tmp == 0)
		return -1;
//#ifdef __builtin_clz
//	return Deck::getcard(which, 31-__builtin_clz(tmp));
//#endif
	
	int x = 0;
	while (tmp)
	{
		if (tmp&1)
			return Deck::getcard(which, x);
		tmp = tmp>>1;
		x++;
	}
	return -1;
}

card Deck::suitLow(int which) const
{
	uint16_t tmp = getSuit(which);
	if (tmp == 0)
		return -1;
//#ifdef __builtin_ctz
//	return Deck::getcard(which, __builtin_ctz(tmp));
//#endif
	int x = 0;
	while (true)
	{
		tmp = tmp>>1;
		if (tmp==0)
			return Deck::getcard(which, x);
		x++;
	}
	return -1; //which*13+12; // huh?
}

int Deck::numCardsHigher(card c) const
{
	uint16_t tmp = getSuit(getsuit(c));
	int answer = 0;
	for (int x = 0; x < getrank(c); x++)
	{
		if (tmp&0x1)
			answer++;
		tmp = tmp>>1;
	}
	return answer;
}

card Deck::getRandomCard()
{
	int x;
	if (cards == 0)
	{
		return -1;
	}
	//r.SetInterval(0, 63);
	if (count() > 20)
	{
		do {
			//x = r.iRandom();
			x = r.ranged_long(0, 63);
			//if ((x < 0) || (x > 51))
			//printf("Error, not in range!!!\n");
		} while (!has(x));//[x] != 1);
	}
	else {
		int val = count();
		val = r.ranged_long(0, val-1);
		uint64_t c = cards;
		while (val > 0)
		{
			c = (c-1)&c;
			val--;
		}
#ifdef __builtin_ctzll
		x = __builtin_ctzll( c );
#else
		x = 0;
		while ((c&0x1) == 0)
		{
			x++;
			c>>=1;
		}
#endif
		assert(has(x));
	}
	return x;
}

card Deck::getRandomCard(int suit)
{
	int x;
	if ((cards == 0) || (!hasSuit(suit)))
	{
		return -1;
	}
	//r.SetInterval(0+16*suit, 15+16*suit);
	if (suitCount(suit) > 4)
	{
		do {
			x = r.ranged_long(0+16*suit, 15+16*suit);
			//x = r.iRandom();
			//if ((x < 0) || (x > 51))
			//printf("Error, not in range!!!\n");
		} while (!has(x));//[x] != 1);
	}
	else {
		int val = suitCount(suit);
		val = r.ranged_long(0, val-1);
		int c = getSuit(suit);
		while (val > 0)
		{
			c = (c-1)&c;
			val--;
		}
#ifdef __builtin_ctzll
		x = __builtin_ctzll( c );
#else
		x = 0;
		while ((c&0x1) == 0)
		{
			x++;
			c>>=1;
		}
#endif
		x += 16*suit;
		assert(has(x));
	}
	
	return x;
}

card Deck::Deal()
{
	int x = getRandomCard();
	clear(x);
	return x;
}

int Trick::Winner() const
{
	if (curr == 0)
		return -1;
	int suit, x = 0;
	int winner = x;
	card rank;

	suit = Deck::getsuit(play[x]);
	rank = Deck::getrank(play[x]);

	for (x = 1; x < curr; x++)
	{
		if (((Deck::getsuit(play[x]) == suit) && (Deck::getrank(play[x]) < rank)) ||
				((Deck::getsuit(play[x]) == trump) && (suit != trump)))
		{
			rank = Deck::getrank(play[x]);
			suit = Deck::getsuit(play[x]);
			winner = x;
		}
	}
	//printf("Player %d took the trick\n", player[winner]);
	return player[winner];
}

int Trick::WinningCard() const
{
    if (curr == 0)
        return -1;
    int suit, x = 0;
    int winner = x;
    card rank;
    
    suit = Deck::getsuit(play[x]);
    rank = Deck::getrank(play[x]);
    
    for (x = 1; x < curr; x++)
    {
        if (((Deck::getsuit(play[x]) == suit) && (Deck::getrank(play[x]) < rank)) ||
            ((Deck::getsuit(play[x]) == trump) && (suit != trump)))
        {
            rank = Deck::getrank(play[x]);
            suit = Deck::getsuit(play[x]);
            winner = x;
        }
    }
    return play[winner];
}

int Trick::bestRankWin() const
{
	card c = WinningCard();
	for (int x = 0; x < curr; x++)
	{
		if (play[x] != c)
		{
			// if the suits match, it won by rank.
			if (Deck::getsuit(play[x]) == Deck::getsuit(c))
				return c;
		}
	}
	if (c != play[0])
		return c;
	return -1;
}

void Trick::Display()
{
	for (int x = 0; x < curr; x++)
	{
		if (play[x] != -1)
			PrintCard(play[x]);
		else
			printf("pass \n");

		printf("(%d)", player[x]);
	}
	printf("\n");
}

void Trick::AddCard(card c, int who)
{
	play[curr] = c;
	player[curr] = who;
	curr++;
}

card Trick::RemoveCard()
{
	card last = play[curr];
	curr--;
	return last;
}

void Trick::reset(int numP, int t)
{
	np = numP;
	trump = t;
	curr = 0;
}

void cardHashState::Print(int val) const
{
	for (int x = 0; x < nump; x++)
	{
		printf("Player %d\n", x);
		printf("cards: %016llX\n", cards[x]);
		//(long)(cards[x]>>32)&0xFFFFFFFF, (long)(cards[x]&0xFFFFFFFF));
		printf("points: %d\n", pts[x]);
	}
}



unsigned long cardHashState::hash_key()
{
	unsigned long v=0;

	for (int x = 0; x < nump; x++)
	{
		unsigned long temp = cards[x];
		for (int y = 0; (y < 64)&&temp; y++)
		{
			if (temp&1)
				v^=(randNumbers[x][y]);
			temp>>=1;
		}
//		if (x%2)
//			v ^= ((cards[x]<<12)&0xFFFFFFFF)^((cards[x]>>35)&0xFFFFFFFF)^((cards[x]>>15)&0xFFFFFFFF);
//		else
//			v ^= ((cards[x]<<17)&0xFFFFFFFF)^((cards[x]>>48)&0xFFFFFFFF)^((cards[x]>>16)&0xFFFFFFFF);
		v^=randPoints[x][(pts[x])&0x1F];
	}
//	v ^= ((numCards>>32)^(numCards));
//	v ^= numCards;
	return v;//+nextp;
}

bool cardHashState::equals(State *val)
{
	assert(val->type() == type());

	cardHashState *st = (cardHashState *)val;
	for (int x = 0; x < nump; x++)
	{
		if (cards[x] != st->cards[x])
			return 0;
		if (pts[x] != st->pts[x])
			return 0;
	}
	if (numCards != st->numCards)
		return 0;
	if (nextp != st->nextp)
		return 0;
	/*
	if (a != st->a)
		return 0;
	if (b != st->b)
		return 0;
	*/
	return 1;
}

/*
cardPartition::cardPartition(GameState *g, double skor, double d)
	:partition(g, skor, d)
{
	CardGameState *cgs = (CardGameState*)g;
	// when we start we are a win or loss and
	// all cards in the had are x's.
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (g)
				cardcount[x][y] = cardxs[x][y] = cgs->cards[x].suitCount(y);
			cards[x][y] = 0;
		}
	}
	score = (int)skor;
	if (d == INF)
		won = true;
	else
		won = false;
}

void cardPartition::Print(int val) const
{
	printf("Score: %d, won: %s\n", score, won?"y":"n");
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			printf("Player %d count: %d card %d x: %d\n", x, 
						cardcount[x][y], cards[x][y], cardxs[x][y]);
		}
	}
}

returnValue *cardPartition::clone() const
{
	cardPartition *cp = new cardPartition(0, 0, 0);
	cp->won = won;
	cp->score = score;
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			cp->cardcount[x][y] = cardcount[x][y];
			cp->cards[x][y] = cards[x][y];
			cp->cardxs[x][y] = cardxs[x][y];
		}
	}
	return cp;
}

double cardPartition::getValue(int who) const
{
	if (won)
		return INF;
	return NINF;
}

void cardPartition::appendMove(Move *mm, GameState *g)
{
	CardGameState *cgs = (CardGameState*)g;
	mm->next = m;
	m = mm;
	CardMove *cm = (CardMove*)mm;
	CardMove *iter = cm;
	
	Trick t(*cgs->getCurrTrick());
	while ((!t.Done()) && iter)
	{
		t.AddCard(iter->c, iter->player);
		iter = (CardMove*)iter->next;
	}
	
	if (t.bestRankWin() == cm->c)
	{
		cards[cm->player][getsuit(cm->c)]++;
	}
	else
		cardxs[cm->player][getsuit(cm->c)]++;
	cardcount[cm->player][getsuit(cm->c)]++;
}

void cardPartition::Union(partition *val, GameState *g)
{
	cardPartition *cp = (cardPartition*)val;
	if (won != cp->won)
	{
		printf("Trying to union states with different results\n");
		exit(0);
	}
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (cardcount[x][y] != cp->cardcount[x][y])
			{
				printf("Error - unioning mismatching hands\n");
				exit(0);
			}
			cards[x][y] = max(cards[x][y], cp->cards[x][y]);
			cardxs[x][y] = cardcount[x][y] - cards[x][y];
		}
	}
}

HashState *cardPartition::getHashState(GameState *g, int me)
{
//	return getHashState(g);
	HashState *hs = new HashState();
	cardPartitionHashState *cghs = new cardPartitionHashState(g, g->getPlayer(me)->score(me), cards);
	cghs->won = won;
	hs->ghs = cghs;
	return hs;
}

cardPartitionHashState::cardPartitionHashState(GameState *g, double score, const byte countlimit[4][4])
{
	byte suitcounts[4][4];
	CardGameState *cgs = (CardGameState*)g;
	won = log(-1);
	this->score = (int)score;
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			cardcount[x][y] = cgs->cards[x].suitCount(y);
			cards[x][y] = 0;
			if (countlimit)
				suitcounts[x][y] = countlimit[x][y];
			else
				suitcounts[x][y] = cardcount[x][y];
			cardxs[x][y] = cardcount[x][y]-suitcounts[x][y];
		}
	}
	int cardhand[4]; // this isn't coded for more than 4 players...
	// make the condensed hand representation
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < g->getNumPlayers(); y++)
		{
			cardhand[y] = cgs->cards[y].getSuit(x);
		}
		
		int location = 0;
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < g->getNumPlayers(); z++)
			{
				if ((cardhand[z]&1) && (suitcounts[z][x] > 0))
				{
					cards[z][x]+=(1<<location);
					suitcounts[z][x]--;
					location++;
				}
				cardhand[z] = cardhand[z]>>1;
			}
		}
	}
}

unsigned long cardPartitionHashState::hash_key()
{
	unsigned long answer = 0;
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			answer = (answer<<4)+cardcount[x][y];
		}
	}
	return answer;
}

bool cardPartitionHashState::equals(State *val)
{
	if (isnan(won))
	{
		printf("Need to do equals the other way around!\n");
		return val->equals(this);
	}
	cardPartitionHashState *cphs = (cardPartitionHashState *)val;
	if (score != cphs->score)
		return false;
	unsigned short part[3], hand[3]; // 3 players, 4 suits

	// we need to match count, and count-xs
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (cardcount[x][y] != cphs->cardcount[x][y])
				return false;
		}
	}
	byte exemptions[3];
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 3; x++)
		{
			part[x] = cards[x][y];
			hand[x] = cphs->cards[x][y]; 
			exemptions[x] = 0;
		}

		while (part[0] || part[1] || part[2])
		{
			if ((hand[0] == 0) && (hand[1] == 0) && (hand[2] == 0))
				return false;
			for (int x = 0; x < 3; x++)
			{
				if (hand[x]&1)	// \_/ if we have the card it must either be in the
				{				// / \ partition, or no cards can be in the partition
					if (part[x]&1)
					{
						for (int z = 0; z < 3; z++)
						{
							part[z] = part[z]>>1;
							if ((z != x) && (exemptions[x] == 0))
								exemptions[z]++;
						}
						exemptions[x] = 0;
					}
					else if ((part[x] != 0) || (hand[x] > 1) || (exemptions[x] == 0))
						return false;
					else if (exemptions[x] > 0)
						exemptions[x]--;
				}
				hand[x] = hand[x]>>1;
			}
		}
	}

	return true;
}

void cardPartitionHashState::Print(int val) const
{
	printf("Score: %d, won: %s\n", score, won?"yes":"no");
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			printf("Player %d count: %d card 0x%X x: %d\n", x, 
						cardcount[x][y], cards[x][y], cardxs[x][y]);
		}
	}
}

*/
