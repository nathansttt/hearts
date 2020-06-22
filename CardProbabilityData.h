/*
 *  CardProbabilityData.h
 *  games
 *
 *  Created by Nathan Sturtevant on 12/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <vector>
#include <stdint.h>
#include <stdio.h>

#ifndef CARDPROBDATA_H
#define CARDPROBDATA_H

namespace hearts {

class cardProbData {
public:
	cardProbData(char *file = 0);
	void save(char *file);
	void load(char *file);
	
	// given that I lead x, what's the probability I have y. [64][64]
	std::vector<std::vector<float> > probGivenLead;
	// given that x is lead and I play y, what's the probability I have z [64][64][64]
	std::vector<std::vector<std::vector<float> > > probGivenLeadAndPlay;
	// frequency of cards given passes [64][64]
	std::vector<std::vector<float> > freq1;
	// frequency of cards given 2 passes [64][64][64]
	std::vector<std::vector<std::vector<float> > > freq2;

	// given that you lead x, I follow y, what is the probability of length z in each of 4 suits [64][64][4][13]
	std::vector<std::vector<uint32_t> > suitLengthProbGivenLeadAndPlay;
	// given that I lead x, what is the probability of length y in each of 4 suits [64][4][13]
	std::vector<std::vector<uint32_t> > suitLengthProbGivenLead;
	// given that I pass x, what is the probability of length y in each of 4 suits [64][4][13]
	std::vector<std::vector<uint32_t> > suitLengthProbGivenPass;
	// prior from data; before pass counts of lengths [4][13]
	std::vector<std::vector<uint32_t> > suitLengthsPrior;
};

} // namespace hearts

#endif
