
#include "Precompile.h"
#include "HoneyGenes.h"

#include "HoneySim.h"
#include "HoneyAI.h"  // NUM_ALGORITHMS

// Tunable values

// 1/6 chance of working on each solution in order from the beginning.
// We will spend time based on this value working on each solution, with the first solution
// always proportionally higher (at least this % of the generations will be spent on our current best)
const int kRandomEval = 1;
const int kRandomCheck = 3;

// 1/n chance of adding an improved solution to the list of possible answers
const int kReplaceOldSolutionChance = 30;

// how many generations to run
const int kNumGenerations = 10000;

// Maximum size of a plan
const int kMaxGenes = 1000;

// Maximum number of solutions we work on at once
const int kInitialSolutions = 10;
const int kMaxSolutions = 40;

using namespace std;


int RandomAIIndex()
{
	// TODO: we might want to weight the AIs by how good we think they are
	return RandomInt(NUM_ALGORITHMS);
}


Gene RandomGene(int len)
{
	// we currently only have the 'choose ai' gene type

	// pick a random ai
	int ai = RandomAIIndex();

	Gene gene;
	memset(&gene, 0, sizeof(Gene));
	gene.cmd = kGeneSetAI;
	gene.len = len;
	gene.data = ai;
    gene.canPerturbContext = (ALGORITMS[ai].fnContext != NULL) && ALGORITMS[ai].context.perturbFunc;
    gene.funcContext = ALGORITMS[ai].context;

	return gene;
}

void InitSolution(GeneSolution* pSolution)
{
	pSolution->score = -1;
	pSolution->sequence.clear();

	Gene gene = RandomGene(gSim.problem.sourceLength);
	pSolution->sequence.push_back(gene);
}

bool DeleteGene(GeneSolution* pSolution)
{
	// TODO: do it
	return false;
}

bool AddWordGene(GeneSolution* pSolution)
{
	// Pick a gene to split
	int numGenes = pSolution->sequence.size();
	int geneIdx = RandomInt(numGenes+1);

	Gene gene;
	memset(&gene, 0, sizeof(Gene));
	gene.cmd = kGeneSetWord;
	gene.len = 0;
	//gene.data = ai;
    //gene.canPerturbContext = (ALGORITMS[ai].fnContext != NULL) && ALGORITMS[ai].context.perturbFunc;
    //gene.funcContext = ALGORITMS[ai].context;

	// insert the new gene with the new time
	pSolution->sequence.insert(pSolution->sequence.begin() + geneIdx, gene);

	return true;
}

bool AddAIGene(GeneSolution* pSolution)
{
	// Pick a gene to split
	int numGenes = pSolution->sequence.size();
	int geneIdx = RandomInt(numGenes);

	// Pick how much time to give the new gene
	int lifetime = pSolution->sequence[geneIdx].len;
	if(pSolution->sequence[geneIdx].len <= 1)
		return false;

	// figure out times
	int newTime = RandomIntRange(1, lifetime - 1);
	Gene gene = RandomGene(newTime);
	newTime = gene.len;

	int oldTime = lifetime - newTime;

	if(oldTime < 1)
		return false;

	int insertLocation = geneIdx;
	if(RandomInt(2) == 0)
	{
		// insert after
		insertLocation++;
	}

	// assign time to the old gene
	pSolution->sequence[geneIdx].len = oldTime;

	// insert the new gene with the new time
	pSolution->sequence.insert(pSolution->sequence.begin() + insertLocation, gene);

	// success
	return true;
}


bool PerturbGeneTime(int geneIdx, int numGenes, GeneSolution* pSolution)
{
    // move some time from this gene into another gene
    int otherGeneIdx = geneIdx - 1;
    if (RandomInt(100) < 10)
    {
        otherGeneIdx = RandomInt(numGenes);
    }

	while(otherGeneIdx >= 0 && pSolution->sequence[otherGeneIdx].len <= 0)
	{
		otherGeneIdx--;
	}

    if (otherGeneIdx < 0 || otherGeneIdx == geneIdx)
        return false;

    int totalTime = pSolution->sequence[otherGeneIdx].len
        + pSolution->sequence[geneIdx].len;

    int newTime = RandomIntRange(1, totalTime - 1);
    pSolution->sequence[geneIdx].len = newTime;
    pSolution->sequence[otherGeneIdx].len = totalTime - newTime;
    return true;
}

bool PerturbGeneContext(int geneIdx, GeneSolution* pSolution)
{
    Gene& gene = pSolution->sequence[geneIdx];
    return gene.funcContext.perturbFunc(gene.funcContext);
}

bool PerturbGene(GeneSolution* pSolution)
{
	int numGenes = pSolution->sequence.size();
	int geneIdx = RandomInt(numGenes);

	Gene gene = pSolution->sequence[geneIdx];
	switch(gene.cmd)
	{
	case kGeneSetAI:
		if(RandomInt(100) < 50)
		{
			// just replace this command with a new one
			pSolution->sequence[geneIdx] = RandomGene(pSolution->sequence[geneIdx].len);
			return true;
		}

		if (pSolution->sequence[geneIdx].canPerturbContext)
		{
			if (RandomInt(100) < 30)
			{
				return PerturbGeneContext(geneIdx, pSolution);
			}
		}

		return PerturbGeneTime(geneIdx, numGenes, pSolution);
	case kGeneSetWord:
		// TODO: implement me
		break;
	}

	return false;
}

bool MutateSolution(GeneSolution* pSolution)
{
	int mutationRng = RandomInt(100);
	int numGenes = pSolution->sequence.size();

	if(mutationRng < 10)
	{
		if(mutationRng < 3 || numGenes >= kMaxGenes)
		{
			return DeleteGene(pSolution);
		}
		else
		{
			int geneSelectRng = RandomInt(100);
			if(geneSelectRng < 98)
			{
				return AddAIGene(pSolution);
			}
			else
			{
				return AddWordGene(pSolution);
			}
		}
	}
	else
	{
		return PerturbGene(pSolution);
	}
}

void EvaluateSolution(GeneSolution* pSolution, int seed)
{
	HoneyStartGameSeed(seed);

	int geneIdx = 0;
	int currentAI = 0;
	int lifetime = 0;
	int numGenes = pSolution->sequence.size();

	while(1)
	{
		if(lifetime <= 0)
		{
			// out of genes somehow
			// (this shouldn't really happen, but deal with it)
			if(geneIdx >= numGenes)
				break;

			Gene newGene = pSolution->sequence[geneIdx];
			geneIdx++;

			lifetime += newGene.len;

			switch(newGene.cmd)
			{
			case kGeneSetAI:
				XAssert(newGene.data >= 0 && newGene.data < NUM_ALGORITHMS);
				currentAI = newGene.data;
				break;
			case kGeneSetWord:
				AIPowerWorder();
				continue;
			default:
				XHalt();
			}
		}

		if(gSim.gameState != GAME_PLAYING)
		{
			// game is over
			break;
		}

		// Run the current AI
		int prevSpawned = gSim.board.unitsSpawned;

		RunAIWithContext(currentAI, pSolution->sequence[geneIdx-1].funcContext);

		int newSpawned = gSim.board.unitsSpawned;

		// And decrement our current gene lifetime by how many units we spawned
		if(newSpawned > prevSpawned)
			lifetime -= (newSpawned - prevSpawned);
		else
			lifetime--;
	}

	pSolution->score = gSim.board.totalScoreUnits + gSim.board.totalScoreWords;
}

list<GeneSolution>::iterator ChooseSolutionToWorkOn(GeneAI* pAI)
{
	list<GeneSolution>::iterator it = pAI->solutions.begin();

	if(it == pAI->solutions.end())
		return it;

	while(RandomInt(kRandomCheck) >= kRandomEval)
	{
		++it;

		if(it == pAI->solutions.end())
			it = pAI->solutions.begin();
	}

	return it;
}

void InsertSorted(GeneAI* pAI, GeneSolution* pNewSolution)
{
	list<GeneSolution>::iterator it = pAI->solutions.begin();

	for(; it != pAI->solutions.end() && it->score > pNewSolution->score; ++it)
	{
		/*nothing*/
	}

	pAI->solutions.insert(it, *pNewSolution);
	pAI->numSolutions++;
}

bool NormalGeneticStep(GeneAI* pAI)
{
	// Pick a solution to modify
	list<GeneSolution>::iterator it = ChooseSolutionToWorkOn(pAI);

	if(it == pAI->solutions.end())
		return false;

	// Copy it
	GeneSolution solution = *it;

	// Try to modify it
	if(!MutateSolution(&solution))
		return false;

	solution.score = -1;

	// Score the updated solution
	EvaluateSolution(&solution, pAI->seed);

	// If the new solution is at least as good, use it
	if(solution.score >= it->score)
	{
		pAI->solutions.erase(it);
		pAI->numSolutions--;
		InsertSorted(pAI, &solution);
	}
	else if(pAI->numSolutions < kMaxSolutions || RandomInt(kReplaceOldSolutionChance) == 0)
	{
		// Sometimes add the mutation to the working set
		if(pAI->numSolutions >= kMaxSolutions)
		{
			if(pAI->solutions.back().score < solution.score)
			{
				pAI->solutions.pop_back();
				pAI->numSolutions--;
				InsertSorted(pAI, &solution);
			}
		}
		else
		{
			InsertSorted(pAI, &solution);
		}
	}
	// otherwise just ignore it

	// We finished a new generation
	pAI->trials++;
	return true;
}

#if 1 || defined(NDEBUG)
#define DebugCheckSolution(x,y) do{}while(0)
#else
void DebugCheckSolution(GeneSolution* pSolution, int seedIndex)
{
	if(gSim.ouput.numSolutions <= 2)
		return;

	// Run the AI plan again to validate that there are no non-deterministic operations

	CopyOutput(0,2);
	EvaluateSolution(pSolution, seedIndex);

	XAssert(gSim.ouput.solutions[0].numCommands == gSim.ouput.solutions[2].numCommands);
	int numCommands = gSim.ouput.solutions[0].numCommands;
	const int kCommandSize =  sizeof(gSim.ouput.solutions[0].pcommandBuffer[0]);
	XAssert(!memcmp(gSim.ouput.solutions[0].pcommandBuffer, gSim.ouput.solutions[2].pcommandBuffer, kCommandSize * numCommands));
}
#endif


bool GeneticStep(GeneAI* pAI)
{
	if(pAI->trials < 10)
	{
		// we are still seeding the gene pool
		GeneSolution solution;
		InitSolution(&solution);
		EvaluateSolution(&solution, pAI->seed);
		DebugCheckSolution(&solution, pAI->seed);
		InsertSorted(pAI, &solution);
		pAI->trials++;

		return true;
	}
	else
	{
		return NormalGeneticStep(pAI);
	}
}

void GeneInit(GeneAI *pAI, int seed)
{
	pAI->numSolutions = 0;
	pAI->solutions.clear();
	pAI->seed = seed;
	pAI->trials = 0;
	memset(pAI->GenerationTimes, 0, sizeof(pAI->GenerationTimes));
}
