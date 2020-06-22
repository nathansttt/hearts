#include "iiGameState.h"

namespace hearts {

iiGameState::iiGameState()
{
}

iiGameState::~iiGameState()
{
}

void iiGameState::getGameStates(int count, std::vector<GameState *> &states, std::vector<double> &prob)
{
	states.resize(0);
	prob.resize(0);
	double sum = 0;
	for (int x = 0; x < count; x++)
	{
		double probability;
		states.push_back(getGameState(probability));
		prob.push_back(probability);
		sum += probability;
	}
	// normalize
	for (int x = 0; x < count; x++)
		prob[x]/=sum;
}

} // namespace hearts
