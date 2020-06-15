/* File: hash.cpp */

#include <stdio.h>
#include <iostream>
#include <math.h>
#include "hash.h"

using namespace std;

int creationCounter;

State::State()
{
	//ret = 0;
	nodeNum = creationCounter++;
  /*printf("%d c\n", nodeNum);*/
}

State::~State()
{
	//	delete ret; /*printf("%d d\n", nodeNum);*/
}


/*  HashTable
 *
 *  A hash table of States. A State is required to
 *  have a hash function, and we use that here.
 */

HNode::HNode(State *val, HNode *nxt)
{ key = val; next = nxt; hits = 0; }

HNode::~HNode()
{
  delete key;
  key = 0;
  while (next)
	{
		HNode *n = next;
		next = n->next;
		n->next = 0;

		delete n;
	}
}

HashTable::HashTable(int size)
{
  table = new HNode * [size];
  for (int i = 0;i < size; i++)
    table[i] = 0;
  tsize = size;
  elts = 0;
}

HashTable::~HashTable()
{
  for (int i = 0; i < tsize; i++)
	{
		if (table[i])
		{
			delete table[i];
			table[i] = 0;
		}
	}
  delete [] table;
}

State *HashTable::IsIn(State *val) const
{
  HNode *trav;
  State *ans = 0;
  unsigned long hval = val->hash_key();
  unsigned int key = hval%tsize;

  for (trav = table[key]; ans==0; trav=trav->next)
	{
		if (!trav)
			break;
		if (trav->key->equals(val))
		{
			trav->hits++;
			ans = trav->key;
			break;
		}
	}
  return ans;
}

void HashTable::Add(State *val)
{
  unsigned long hval = val->hash_key();
  unsigned int key = hval%tsize;

  table[key] = new HNode(val, table[key]);

  elts++;
}

void HashTable::Remove(State *val)
{
  HNode *trav, *prev = 0;
  unsigned long hval = val->hash_key();
  unsigned int key = hval%tsize;
  
  if (!IsIn(val))
    return;
  
  for (trav = table[key];trav;trav=trav->next)
	{
		if (trav->key->equals(val))
			break;
		prev = trav;
	}
  
  if (prev)
	{
		trav = prev->next;
		prev->next = trav->next;
	}
  else {
    trav = table[key];
    table[key] = trav->next;
  }
  trav->next = 0;
  delete trav;
  trav = 0;
  
  elts--;
}

void HashTable::Clear()
{
	//PrintStats();
  elts = 0;
  for (int i = 0; i < tsize; i++)
	{
		if (table[i])
		{
			delete table[i];
			table[i] = 0;
		}
	}
}

void HashTable::iterReset()
{
	iterIndex = 0;
	iterNode = 0;
	for (int x = iterIndex; x < tsize; x++)
	{
		if (table[x] != 0)
		{
			iterIndex = x;
			iterNode = table[x];
			break;
		}
	}
}

bool HashTable::iterDone()
{
	return (iterNode == 0);
}

State *HashTable::iterNext()
{
	if (iterDone())
		return 0;
	State *ret = iterNode->key;
	if (iterNode->next)
	{
		iterNode = iterNode->next;
	}
	else {
		iterIndex++;
		iterNode = 0;
		for (int x = iterIndex; x < tsize; x++)
		{
			if (table[x] != 0)
			{
				iterIndex = x;
				iterNode = table[x];
				break;
			}
		}
	}
	return ret;
}

void HashTable::PrintStats() const
{
  int max = 0, min = elts, counter, x;
	int hsum = 0;
  int maxhits[10];
  HNode *trav;
  
  for (x = 0; x < 10; x++)
    maxhits[x] = 0;
  for (x = 0; x < tsize; x++)
	{
		counter = 0;
		trav = table[x];
      
		while (trav)
		{
			counter++;
			for (int y = 0; y < 10; y++)
	    {
	      if (trav->hits > maxhits[y])
				{
					for (int z = 9; z > y; z--)
						maxhits[z] = maxhits[z-1];
					maxhits[y] = trav->hits;
					break;
				}
	    }
			trav = trav->next;
		}
		if (counter > 0)
			hsum++;
		if (counter < min)
			min = counter;
		if (counter > max)
			max = counter;
	}
  cout << "There are " << elts << " items in the table (tsize " << tsize << ")." << endl;
  cout << "Minimum keys in bucket: " << min << endl;
  cout << "Maximum keys in bucket: " << max << endl;
  cout << "Average keys in bucket: " << elts/tsize << endl;
  cout << hsum << " buckets have keys, adjusted average " << elts/hsum << endl;
  cout << "Ten best Hit count:" << endl;
  for (int y = 0; y < 10; y++)
    cout << y+1 << ": " << maxhits[y] << endl;

  //  delete [] count;
}
