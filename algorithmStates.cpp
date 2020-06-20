#include <math.h>
#include "algorithmStates.h"
//#include "LimitedEvalPlayer.h"

//#define min(x, y) (((x)>(y))?(y):(x))
//#define max(x, y) (((x)<(y))?(y):(x))

maxnState::maxnState(double lim, short after, short d)
:limit(lim), next(after), depth(d) {}

//bool maxnState::equals(State *val)

void maxnState::Print(int val) const
{
	printf("depth %d\n", depth);
}


minimaxState::minimaxState(double aa, double bb, short nex, short d, short dr)
  :a(aa), b(bb), next(nex), depth(d), depthRemaining(dr)
{ result = log(-1); }

void minimaxState::Print(int val) const
{
	printf("depth %d/%d, a=%1.2f, b=%1.2f ans=%1.2f, next=%d\n", depth, depthRemaining, a, b, result, next);
}

bool minimaxState::equals(State *val)
{
	minimaxState *mms = (minimaxState *)val;
//#pragma WARNING "isnan problem here"
//#warning "unresolved isnan problem here"
	if (isnan(result))
		return mms->equals(this);
	if (next != mms->next)
		return false;
	if ((depth != mms->depth) || (depthRemaining != mms->depthRemaining))
		return false;
	if (result == a)
		if (a != mms->a)
			return false;
	if (result == b)
		return (b == mms->b);
	return true;
}


maxnval::maxnval() :returnValue()
{
	eval[0] = eval[1] = eval[2] = eval[3] = eval[4] = eval[5] = 0;
}

maxnval::~maxnval() { }

returnValue *maxnval::clone(GameState *g) const {
	maxnval *r = new maxnval();
	r->m = (m==0)?0:m->clone(g);
	r->eval[0] = eval[0]; r->eval[1] = eval[1]; r->eval[2] = eval[2];
	r->eval[3] = eval[3]; r->eval[4] = eval[4]; r->eval[5] = eval[5];
	r->numPlayers = numPlayers;
	return r;
}

void maxnval::Print(int v) const
{
		printf("(%1.4f, %1.4f, %1.4f, %1.4f...)\n",
					 eval[0], eval[1], eval[2], eval[3]);
		if (v) {
			if (next) next->Print(v);
			if (m) m->Print(v);
		}
}

double maxnval::getValue(int who) const
{
	return eval[who];
}


minimaxval::minimaxval()
:returnValue()
{ m = 0; }

minimaxval::minimaxval(double v, Move *mv)
{
	val = v; m = mv;
}

minimaxval::~minimaxval()
{ }

returnValue *minimaxval::clone(GameState *g) const {
	Move *mm=0;
	if (m != 0) mm = m->clone(g);
	return new minimaxval(val, mm);
}

double minimaxval::getValue(int who) const
{
	return val;
}

void minimaxval::Print(int v) const
{
	if (next)
	{
		next->Print(1);
	}
	if (m)
	{
		printf(" %1.2f ", val);
		m->Print(v);
	}
}


partition::partition(GameState *g, double score, double d)
{}

returnValue *outcomePDF::clone(GameState *g) const
{
	outcomePDF *p = new outcomePDF(m?m->clone(g):0, /*next?next->clone(g):*/0);
	p->utility.resize(utility.size());
	for (unsigned int x = 0; x < utility.size(); x++)
		for (unsigned int y = 0; y < utility[x].size(); y++)
			p->utility[x].push_back(utility[x][y]);
	p->realUtility = realUtility;
	return p;
}

void outcomePDF::Print(int verbose) const
{
	for (unsigned int x = 0; x < utility.size(); x++)
	{
		printf("(%d)\t", x);
		for (unsigned int y = 0; y < utility[x].size(); y++)
		{
			printf("%1.4f\t", utility[x][y]);
		}
		printf("\n");
	}
	printf("Real util: %1.4f\t", realUtility);
	if (m) m->Print(1);
	printf("\n");
}
