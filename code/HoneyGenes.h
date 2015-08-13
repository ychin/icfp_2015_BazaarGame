// HoneyGenes.h

#ifndef HONEYGENES_H
#define HONEYGENES_H

#include <list>
#include <vector>

#include "HoneyAI.h"

enum GeneCmd
{
	kGeneSetAI,
	kGeneSetWord,
};

struct Gene
{
	GeneCmd cmd;
	int len;
	int data;
    bool canPerturbContext;
    AIFuncContext funcContext;
};

struct GeneSolution
{
	int score;				// -1 if unevaluated
	std::vector<Gene> sequence;
};

// average this many generations when guessing how long it will take to run a generation
static const int kNumGenerationsBudget = 5;

struct GeneAI
{
	std::list<GeneSolution> solutions; // in descending order by score
	int seed;
	int numSolutions;
	int trials;

	// Used by HoneyRunner
	float GenerationTimes[kNumGenerationsBudget];
};

void GeneInit(GeneAI *pAI, int seed);
bool GeneticStep(GeneAI* pAI);


#endif // HONEYGENES_H