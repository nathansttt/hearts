/*
 *  UCT.cpp
 *  games
 *
 *  Created by Nathan Sturtevant on 11/16/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Player.h"
#include "UCT.h"
#include "fpUtil.h"
#include <string>
#include <sstream>

namespace hearts {

const int debugState = 0;
static bool verbose = false;

UCT::UCT(int numRuns, double cval1, double cval2)
{
	//sprintf(name, "UCT_N-%d_C1-%1.3f_C2-%1.3f", numRuns, cval1, cval2);
	numSamples = numRuns;
	currTreeLoc = -1;
	switchLimit = -1;
	C1 = cval1;
	C2 = cval2;
	epsilon = 0;
	pm = 0;
//	RAVE = 0;
	HH = false;
	rand.srand(time(0));
	verboseMoves = false;
}

UCT::UCT(int numRuns, int crossOver, double cval1, double cval2)
{
	//sprintf(name, "UCT_N-%d_S-%d_C1-%1.3f_C2-%1.3f", numRuns, crossOver, cval1, cval2);
	numSamples = numRuns;
	currTreeLoc = -1;
	switchLimit = crossOver;
	C1 = cval1;
	C2 = cval2;
	epsilon = 0;
	pm = 0;
//	RAVE = 0;
	HH = false;
	rand.srand(time(0));
	verboseMoves = false;
}

UCT::UCT(int numRuns, double cval)
{
	//sprintf(name, "UCT_N-%d_C-%1.3f", numRuns, cval);
	//printf("%s\n", name);
	numSamples = numRuns;
	currTreeLoc = -1;
	C1 = C2 = cval;
	switchLimit = -1;
	epsilon = 0;
	pm = 0;
//	RAVE = 0;
	HH = false;
	rand.srand(time(0));
	verboseMoves = false;
	//printf("%s\n", getName());
}

UCT::UCT(char *n, int numRuns, double cval)
{
	name = n;
	//strncpy(name, n, 64);
	numSamples = numRuns;
	currTreeLoc = -1;
	C1 = C2 = cval;
	switchLimit = -1;
	epsilon = 0;
	pm = 0;
//	RAVE = 0;
	HH = false;
	rand.srand(time(0));
	verboseMoves = false;
}

const char *UCT::getName()
{
	std::stringstream out;
	out.clear();
	//out.str(std::string());

	if (numSamples == -1)
		out << "UCT_N-e" << getSearchNodeLimit();
	else
		out << "UCT_N-" << numSamples;
	if (C1 == C2)
		out << "_C-" << C1;
	else
		out << "_S-" << switchLimit << "_C1-" << C1 << "_C2-" << C2;
	if (pm)
		out << "_PM-" << pm->GetModuleName();
	out << "_e-" << epsilon;
//	if (RAVE > 0)
//		out << "_RAVE-" << RAVE;
//	if (HH)
//		out << "_HH";

	name = out.str();
	return name.c_str();
	//	if (pm == 0)
//	{
//		if (C1 == C2)
//			sprintf(localname, "UCT_N-%d_C-%1.3f_e-%1.3f", numSamples, C1, epsilon);
//		else
//			sprintf(localname, "UCT_N-%d_S-%d_C1-%1.3f_C2-%1.3f_e-%1.3f", numSamples, switchLimit, C1, C2, epsilon);
//	}
//	else {
//		if (C1 == C2)
//			sprintf(localname, "UCT_N-%d_C-%1.3f_PM-%s_e-%1.3f", numSamples, C1, pm->GetName(), epsilon);
//		else
//			sprintf(localname, "UCT_N-%d_S-%d_C1-%1.3f_C2-%1.3f_PM-%s_e-%1.3f", numSamples, switchLimit, C1, C2, pm->GetName(), epsilon);
//	}
//	return localname.c_str();
}

maxnval *UCT::GetValue(GameState *g)
{
	maxnval *v = new maxnval();
	
	for (unsigned int x = 0; x < g->getNumPlayers(); x++)
	{
		//printf("%d:%f  ", x, who->cutoffEval(x));
		v->eval[x] = who->cutoffEval(x);//g->score(x);//
	}
	//printf("\n");
	return v;
}

double UCT::GetUCTVal(GameState *g, int parent, int child)
{
	if ((parent == debugState) && (verbose))
		printf("Parent: %d - samples %d, child: %d - samples %d\n", parent,
			   tree[parent].count, child, tree[child].count);
	if (tree[child].count == 0)
	{
		if (pm)
		{
			pm->GetPreInformation(g, g->getNextPlayerNum(), tree[child].count, tree[child].reward);
		}
		if (tree[child].count == 0) 
		{
			return 100+100*GetC(g, parent, child);
		}
	}
	double UCTValue = tree[child].reward + GetC(g, parent, child)*sqrt(log((double)tree[parent].count)/((double)tree[child].count));
	return UCTValue;
}

void UCT::setPlayoutModule(UCTModule *m)
{
	pm = m;
}

void UCT::setEpsilonPlayout(double v)
{
	epsilon = v;
}

double UCT::GetC(GameState *g, int parent, int child)
{
	if (C1 == -1)
	{
		if (tree[parent].count < 30)
			parent = 0;
		double avg = 0;
		double cnt = 0;
		double max = -1e9;
		for (unsigned int x = 0; x < tree[parent].children.size(); x++)
		{
			cnt+=1;
			avg += tree[tree[parent].children[x]].reward;
			if (tree[tree[parent].children[x]].reward > max)
				max = tree[tree[parent].children[x]].reward;
		}
		//return fabs(avg/cnt);
		return max;
	}
	if (switchLimit == -1)
	{
		if (g->getPreviousPlayer() == who)
			return C1;
		return C2;
	}
	if (currentSample > switchLimit)
		return C2;
	return C1;
}

void UCT::Played(int p, Move *m)
{
	if (tree.size() != 0)
	{
		for (unsigned int y = 0; y < tree[currTreeLoc].children.size(); y++)
		{
			if (tree[tree[currTreeLoc].children[y]].m->equals(m))
			{
				if (verbose)
					printf("%d moved! Moving location in tree from %d to %d\n", p,
						   currTreeLoc, tree[currTreeLoc].children[y]);
				currTreeLoc = tree[currTreeLoc].children[y];
				return;
			}
		}
	}
	assert(false);
}

returnValue *UCT::Play(GameState *g, Player *p) // choose next move
{
	resetCounters(g);
	who = p;
	if (currTreeLoc == debugState)
	{
		currTreeLoc = 0;
		UCTNode n;
		tree.push_back(n);
	}
	else {
		currTreeLoc = 0;
		FreeTree(g);
		UCTNode n;
		tree.push_back(n);
	}
	int loopCount = 0;
	int limit = 10000;
	time_t start = time(0);
	while (1)
	{
		//iterDepth = x;
		if (time(0)-start > limit)
		{
			printf("%ld elapsed\n", time(0)-start);
			break;
		}
		if (((numSamples != -1) && (loopCount >= numSamples)) ||
			((numSamples == -1) && (getNodesExpanded() >= getSearchNodeLimit())))
		{
			if (numSamples == -1 && verbose)
				printf("%ld of %ld nodes expanded\n", getNodesExpanded(), getSearchNodeLimit());
			break;
		}
		
		currentSample = loopCount++;
		if (((loopCount%5000) == 0) && (verbose))
			printf("Sample %d\n", loopCount);
		delete PlayUCTTree(g, currTreeLoc);
		tree[currTreeLoc].count++;
	}
	int best = 0;
	
	for (unsigned int y = 1; y < tree[currTreeLoc].children.size(); y++)
	{
		if (fgreater(tree[tree[currTreeLoc].children[y]].reward,
					 tree[tree[currTreeLoc].children[best]].reward))
//		if (tree[tree[currTreeLoc].children[y]].count >
//					 tree[tree[currTreeLoc].children[best]].count)
			best = y;
	}
	if (verbose||verboseMoves)
	{
		PrintTreeNode(currTreeLoc);
		PrintTreeNode(tree[currTreeLoc].children[best], 5);
	}
	minimaxval *rv =  new minimaxval(tree[tree[currTreeLoc].children[best]].reward,
									 tree[tree[currTreeLoc].children[best]].m->clone(g));
	////PrintTreeStats();
	FreeTree(g);
	assert(rv->m != 0);
	logNodes();
	return rv;
}

void UCT::PrintTreeNode(int location, int indent)
{
	for (int x = 0; x < indent; x++)
		printf(" ");
	printf("Values for node %d\n", location);

//	for (int x = 0; x < indent; x++)
//		printf(" ");
//	tree[tree[location].children[0]].m->Print(0);
//	printf(" - move %d samples %d reward %f\n",
//		   0, tree[tree[location].children[0]].count,
//		   tree[tree[location].children[0]].reward);
	for (unsigned int y = 0; y < tree[location].children.size(); y++)
	{
		for (int x = 0; x < indent; x++)
			printf(" ");
		tree[tree[location].children[y]].m->Print(0);
		printf(" - move %d samples %d reward %f\n",
			   y, tree[tree[location].children[y]].count,
			   tree[tree[location].children[y]].reward);
	}
	
}


maxnval *UCT::PlayUCTTree(GameState *g, int location)
{
	if ((g->Done()) || (searchExpired(g)))
		return GetValue(g);
	
	bool sample = false;
	
	
	if (tree[location].children.size() == 0)
	{
//		if ((tree[location].children.size() == 0) && (tree[location].count != -1))
//			printf("Strange: no children, but count is %d\n", tree[location].count);
		ExpandChildren(g, location);
		sample = true;
	}

	int best = 0;
	double val = GetUCTVal(g, location, tree[location].children[0]);
	if ((location == debugState) && (verbose))
		printf("At node %d value for move %d is %f\n", location, best, val);
	for (unsigned int y = 1; y < tree[location].children.size(); y++)
	{
		double childVal = GetUCTVal(g, location, tree[location].children[y]);
		if ((location == debugState) && (verbose))
			printf("At node %d value for move %d is %f\n", location, y, childVal);
		if (fgreater(childVal, val))
		{
			val = childVal;
			best = y;
		}
	}
	int index = tree[location].children[best];

	maxnval *result = 0;

	ApplyMove(g, tree[index].m);
	if (!sample)
	{
		result = PlayUCTTree(g, index);
	}
	else {
		if (pm)
			result = pm->DoRandomPlayout(g, who, epsilon);
		if (result == 0)
			result = DoRandomPlayout(g);
	}
	UndoMove(g, tree[index].m);

	if ((index == 0) && (verbose))
		printf("At loc %d Updaing move: %d\n", location, index);
	if (location == debugState)
	{
		if (verbose)
		{
			tree[index].m->Print(0);
			printf(" - move %d (%d already) adding reward %f (old was %f)\n",
				   best, tree[index].count, result->getValue(g->getNextPlayerNum()),
				   tree[index].reward);
		}
	}
	tree[index].reward *= tree[index].count;
	tree[index].reward += result->getValue(g->getNextPlayerNum());
	tree[index].count++;
	tree[index].reward /= tree[index].count;
	if (location == debugState)
	{
		if (verbose)
			printf(" - move %d (%d now) new reward %f\n",
				   best, tree[index].count, tree[index].reward);
	}

//	if ((RAVE > 0) || (HH))
//	{
//		uint32_t hashv = g->getMoveHash(tree[index].m);
//		RAVEMap[hashv].count++;
//		RAVEMap[hashv].value += (result->getValue(g->getNextPlayerNum()) - RAVEMap[hashv].value)/RAVEMap[hashv].count;
//
////		tree[index].m->Print(0);
////		printf(" STORING %f for %d\n", RAVEMap[hashv].value, g->getNextPlayerNum());
//	}
	
	return result;
}

void UCT::ExpandChildren(GameState *g, int location)
{
	Move *m = g->getMoves();
	while (m)
	{
		UCTNode n(m, location);
		m = m->next;
		n.m->next = 0;
		n.depth = tree[location].depth+1;
		tree[location].children.push_back(tree.size());
//		printf("Adding child at %d to %d\n", tree[location].children.back(),
//			   location);
		tree.push_back(n);
	}
}

returnValue *UCT::Analyze(GameState *g, Player *p) // return eval of all moves
{
	who = p;
	currTreeLoc = 0;
	FreeTree(g);
	UCTNode n;
	tree.push_back(n);
	
	for (int x = 0; x < numSamples; x++)
	{
		currentSample = x;
		if (((x%5000) == 0) && (verbose))
			printf("Sample %d\n", x);
		delete PlayUCTTree(g, currTreeLoc);
		tree[currTreeLoc].count++;
	}
	//int best = 0;
	
	minimaxval *rv=0;
	for (unsigned int y = 0; y < tree[currTreeLoc].children.size(); y++)
	{
		minimaxval *tmp =  new minimaxval(tree[tree[currTreeLoc].children[y]].reward,
										   tree[tree[currTreeLoc].children[y]].m->clone(g));
		tmp->next = rv;
		rv = tmp;
	}
	//PrintTreeStats();
	FreeTree(g);
	assert(rv->m != 0);
	return rv;
}

maxnval *UCT::DoRandomPlayout(GameState *g)
{
	static std::vector<int> distribution;

	if (!g->Done())
	{
		if (HH)
		{
			Move *best = GibbsSample(g);
			ApplyMove(g, best);
			maxnval *v = DoRandomPlayout(g);
			UndoMove(g, best);
			g->freeMove(best);
			return v;
		}
		else if (fequal(epsilon, 0) || (rand.rand_double() < epsilon))
		{
			Move *m = g->getRandomMove();
			ApplyMove(g, m);
			maxnval *v = DoRandomPlayout(g);
			UndoMove(g, m);
			g->freeMove(m);
			return v;
		}
		else {
			Move *m = g->getMoves(); 

//			if (distribution.size() <= m->length())
//				distribution.resize(m->length()+1);
//			distribution[m->length()]++;
//			if ((random()%1000) == 1)
//			{
//				printf("Distribution of values\n");
//				for (unsigned int x = 0; x < distribution.size(); x++)
//					printf("%d\t%d\n", x, distribution[x]);
//			}
			
			//Move *m = getMoves(g, who); // this causes the randomization to take effect!
			ApplyMove(g, m);
			maxnval *v = DoRandomPlayout(g);
			UndoMove(g, m);
			g->freeMove(m);
			return v;
		}
	}
	return GetValue(g);
}

Move *UCT::GibbsSample(GameState *g)
{
	double T = epsilon;
	Move *m = g->getMoves();
	std::vector<double> values;
	std::vector<Move *> moves;
	double sum = 0;
	for (Move *t = m; t; t = t->next)
	{
		double val = 0;
		uint32_t hashv = g->getMoveHash(t);
//		if (RAVEMap.find(hashv) != RAVEMap.end())
//			val = RAVEMap[hashv].value;
		if (val < 0)
			val = 0;

		moves.push_back(t);
		values.push_back(exp(val/T));
		sum += values.back();
	}
	for (unsigned int x = 0; x < values.size(); x++)
		values[x]/=sum;
	double selection = rand.rand_double();
	
	for (unsigned int x = 0; x < values.size(); x++)
	{
		selection -= values[x];
		if (fless(selection, 0))
		{
			Move *tmp = moves[x]->clone(g);
			g->freeMove(m);
			return tmp;
		}
	}
	Move *tmp = moves.back()->clone(g);
	g->freeMove(m);
	return tmp;
}

void UCT::FreeTree(GameState *g)
{
	for (unsigned int x = 0; x < tree.size(); x++)
		g->freeMove(tree[x].m);
	tree.resize(0); // bad memory!
}

void UCT::PrintTreeStats()
{
	printf("%d nodes in UCT tree\n", (int)tree.size());
	std::vector<int> depths;
	for (unsigned int x = 0; x < tree.size(); x++)
	{
		if (tree[x].depth >= (int)depths.size())
			depths.resize(tree[x].depth+1);
		depths[tree[x].depth]++;
	}
	int sum = 0;
	printf("depth\tcount\n");
	for (unsigned int x = 0; x < depths.size(); x++)
	{
		printf("%d\t%d\n", x, depths[x]);
		sum += x*depths[x];
	}
	printf("Average depth = %f\n", (double)sum/tree.size());
	printf("Principle variation:\n");
	int loc = 0;
	while (tree[loc].children.size() > 0)
	{
		if ((tree[loc].m) && (verbose))
			tree[loc].m->Print(0);
		int best = 0;
		for (unsigned int x = 1; x < tree[loc].children.size(); x++)
		{
			if (fgreater(tree[tree[loc].children[x]].reward,
						 tree[tree[loc].children[best]].reward))
				best = x;
		}
		loc = tree[loc].children[best];
	}
}

} // namespace hearts
