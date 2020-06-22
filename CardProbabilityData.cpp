/*
 *  CardProbabilityData.cpp
 *  games
 *
 *  Created by Nathan Sturtevant on 12/11/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "CardProbabilityData.h"

namespace hearts {

cardProbData::cardProbData(char *file)
{
	probGivenLead.resize(64);
	freq1.resize(64);
	for (unsigned x = 0; x < 64; x++)
	{
		probGivenLead[x].resize(64);
		freq1[x].resize(64);
	}

	freq2.resize(64);
	for (unsigned int x = 0; x < freq2.size(); x++)
	{
		freq2[x].resize(64);
		for (unsigned int y = 0; y < freq2[x].size(); y++)
		{
			freq2[x][y].resize(64);
		}
	}
	
	
	probGivenLeadAndPlay.resize(64);
	for (unsigned x = 0; x < 64; x++)
	{
		probGivenLeadAndPlay[x].resize(64);
		for (unsigned y = 0; y < 64; y++)
		{
			probGivenLeadAndPlay[x][y].resize(64);
		}
	}
	if (file)
		load(file);
}

void cardProbData::save(char *file)
{
	FILE *f = fopen(file, "w+");
	if (!f) return;
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			fprintf(f, "%f ", probGivenLead[x][y]);
		}
	}
	
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			for (int z = 0; z < 64; z++)
			{
				fprintf(f, "%f ", probGivenLeadAndPlay[x][y][z]);
			}
		}
	}
	
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			fprintf(f, "%f ", freq1[x][y]);
		}
	}
	
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			for (int z = 0; z < 64; z++)
			{
				fprintf(f, "%f ", freq2[x][y][z]);
			}
		}
	}

	fclose(f);
}

void cardProbData::load(char *file)
{
	FILE *f = fopen(file, "r");
	if (!f) return;
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			fscanf(f, "%f ", &probGivenLead[x][y]);
		}
	}
	
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			for (int z = 0; z < 64; z++)
			{
				fscanf(f, "%f ", &probGivenLeadAndPlay[x][y][z]);
			}
		}
	}
	
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			fscanf(f, "%f ", &freq1[x][y]);
		}
	}
	
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			for (int z = 0; z < 64; z++)
			{
				fscanf(f, "%f ", &freq2[x][y][z]);
			}
		}
	}

	fclose(f);
}

} // namespace hearts
