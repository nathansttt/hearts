#include "States.h"

namespace hearts {

//#define RANDOMIZEMOVE

Move::~Move()
{
	while (next)
	{
		Move *nn = next;
		next = nn->next;
		nn->next = 0;
		delete nn;
	}
}

void Move::append(Move *mm)
{
	if (mm == 0)
		return;
	if (next == 0)
		next = mm;
	else
		next->append(mm);
}

void Move::insert(Move *m)
{
	if (next == 0)
	{
		next = m;
		return;
	}
	else if (m->dist > next->dist)
	{
		m->next = this->next;
		this->next = m;
		return;
	}
#ifdef RANDOMIZEMOVE
	else if (m->dist == next->dist)
	{
		int cnt = 1;
		Move *where = this;	
		while ((where->next != 0) && (where->next->dist == m->dist))
		{
			double f = random();//rand.Random();
			cnt++;
			//printf("TIE! %1.2f %1.2f\n", f, ((double)1/cnt));
			if (f < ((double)1/cnt))
			{
				m->next = where->next;
				where->next = m;
				return;
			}
			where = where->next;
		}
		where->insert(m);
	}
#endif
	else
		next->insert(m);
}

HashState::HashState()
{
	ret = 0; as = 0; ghs = 0;
}

void HashState::Print(int val) const
{
	if (ret)
	{
		printf("Return value:\n");
		ret->Print(0);
	}
	if (as)
	{
		printf("Algorithm State:\n");
		as->Print();
	}
	if (ghs)
	{
		printf("Game Hash State:\n");
		ghs->Print();
	}
}

unsigned long HashState::hash_key()
{
	return ghs->hash_key()^as->hash_key();
}

bool HashState::equals(State *val)
{
	HashState *hs = (HashState*)val;
	if ((as == 0) || (ghs == 0))
		return false;

	if (as->equals(hs->as) &&
			ghs->equals(hs->ghs))
		return true;

	return false;
}

} // namespace hearts
