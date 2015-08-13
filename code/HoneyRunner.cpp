// HoneyRunner.cpp : Defines the entry point for the console application.
//

#include "Precompile.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <SDKDDKVer.h>
    #include <tchar.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "HoneySim.h"
#include "HoneyAI.h"
#include "HoneyGenes.h"

int gTimeLimit = 60*60*24; // time limit in seconds
int gMemLimit = 2 * 1024;  // memory limit in MB
int gNumCores = 1;

#if defined(_WIN32) || defined(_WIN64)
    uint64 HoneyGetSystemFrequency()
    {
        uint64 qwTicksPerSec;
        QueryPerformanceFrequency((LARGE_INTEGER *)&qwTicksPerSec);
        return qwTicksPerSec;
    }

    float MillisecondsFromMachineTimeDiffFloat(uint64 machineTimeStart, uint64 machineTimeEnd)
    {
        static float sSystemFrequency = 1.0f / (float)HoneyGetSystemFrequency();
        uint64 timeDelta = machineTimeEnd - machineTimeStart;
        return (float)timeDelta * 1000.0f * sSystemFrequency;
    }

    uint64 GetMachineTime()
    {
        uint64 timeRead;
        QueryPerformanceCounter((LARGE_INTEGER *)&timeRead);
        return timeRead;
    }
#else
    #include <time.h>
    #include <sys/time.h>
    uint64_t GetMachineTime()
    {
        timeval val;
        gettimeofday(&val, NULL);

        uint64_t timeMillisec = (uint64_t)val.tv_sec * 1000 + (uint64_t)val.tv_usec / 1000;
        return timeMillisec;
    }

    float MillisecondsFromMachineTimeDiffFloat(uint64 machineTimeStart, uint64 machineTimeEnd)
    {
        return (float)(machineTimeEnd - machineTimeStart);
    }
#endif

float GetGenerationTime(GeneAI* pAI, int numtrials)
{
	int count = numtrials < 5 ? numtrials : 5;

	float total = pAI->GenerationTimes[0];
	for(int i = 1; i < count; ++i)
	{
		total += pAI->GenerationTimes[i];
	}

	total /= count;
	return total;
}

bool CheckBudget(GeneAI* pAI, int numtrials, uint64 startTime, float budget)
{
	float expectedTime = GetGenerationTime(pAI, numtrials);
	float remaining = budget - MillisecondsFromMachineTimeDiffFloat(startTime, GetMachineTime());
	return expectedTime < remaining;
}

int RunnerSolveSeed(HoneyOutputInfo& output, int seedIdx, GeneAI* pAI, float timeBudget)
{
	uint64 startTime = GetMachineTime();

	// clear any previous outputs
	DisposeOutput();
	InitOutput(2);

	int bestScore = -1;
	GeneInit(pAI, seedIdx);

	int trial = 0;

	while(bestScore < 0 || CheckBudget(pAI, trial, startTime, timeBudget))
	{
		uint64 startStepTime = GetMachineTime();

		if(GeneticStep(pAI))
		{
			// add time to trial
			pAI->GenerationTimes[trial % kNumGenerationsBudget] = MillisecondsFromMachineTimeDiffFloat(startStepTime, GetMachineTime());
			++trial;

			// Check for new best
			int score = gSim.board.totalScoreUnits + gSim.board.totalScoreWords;
			if(score > bestScore)
			{
				bestScore = score;
				CopyOutput(0,1);
			}
		}
	}

	char elapsedMS[256];
    sprintf(elapsedMS, "%.3fs", MillisecondsFromMachineTimeDiffFloat(startTime, GetMachineTime()) / 1000.0f);

	char genIdxStr[256];
	sprintf(genIdxStr, "gen%d", pAI->trials);

	HoneyOutputSolutionExt(output, seedIdx, 1, genIdxStr, elapsedMS, bestScore);

	return bestScore;
}

int RunnerSolveProblem(HoneyOutputInfo& output, const char* filename, GeneAI seedAIs[], float timeBudget)
{
	int problemScore = 0;

	HoneyLoadProblem(filename);

	timeBudget /= gSim.problem.numSourceSeeds;

	for(int n = 0; n < gSim.problem.numSourceSeeds; ++n)
	{
		problemScore += RunnerSolveSeed(output, n, &seedAIs[n], timeBudget);
	}

	problemScore /= gSim.problem.numSourceSeeds;
	return problemScore;
}

int _tmain(int argc, const char* argv[])
{
	uint64 startTime = GetMachineTime();
	const int maxProblems = 256;
	const char* problemFile[maxProblems];
	int numProblems = 0;
	bool todMode = false;
	bool drawSummary = false;

	// parse global commandline parameters
	for(int i = 1; i < argc; ++i)
	{
		if(!strcmp(argv[i], "-tod"))
		{
			todMode = true;
			AddKnownPowerPhrases();
			problemFile[numProblems++] = "problems/problem_0.json";
			problemFile[numProblems++] = "problems/problem_1.json";
			problemFile[numProblems++] = "problems/problem_2.json";
			problemFile[numProblems++] = "problems/problem_3.json";
		}
		else if(!strcmp(argv[i], "-p") && gSim.numPowerPhrases < kMaxPowerPhrases)
		{
			AppendPowerPhrase(argv[++i]);
		}
		else if(!strcmp(argv[i], "-pp"))
		{
			AddKnownPowerPhrases();
		}
		else if(!strcmp(argv[i], "-ff"))
		{
			int start = atoi(argv[++i]);
			int end = atoi(argv[++i]);
			for(int i = start; i <= end; ++i)
			{
				char* filename = (char*)malloc(256); // leak
				sprintf(filename, "problems/problem_%d.json", i);
				XAssert(numProblems < maxProblems);
				problemFile[numProblems++] = filename;
			}
		}
		else if(!strcmp(argv[i], "-t"))
		{
			gTimeLimit = atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-m"))
		{
			gMemLimit = atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-c"))
		{
			gNumCores = atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-f"))
		{
			XAssert(numProblems < maxProblems);
			problemFile[numProblems++] = argv[++i];
		}
		else if(!strcmp(argv[i], "-summary"))
		{
			drawSummary = true;
		}
		else
		{
			printf("unknown arg '%s'\n", argv[i]);
			//exit(0);
		}
	}

	if (numProblems == 0)
	{
		printf("no problems specified!\n");
		return 1;
	}

    InitHoneySimSystems();

	// Find number of seeds per problem
	int *startSeeds = new int[numProblems];
	int *scores = new int[numProblems];
	int totalSeeds = 0;

	for(int i = 0; i < numProblems; ++i)
	{
		HoneyLoadProblem(problemFile[i]);
		startSeeds[i] = totalSeeds;
		totalSeeds += gSim.problem.numSourceSeeds;
	}

	// Allocate geneAIs
	GeneAI* pAIs = new GeneAI[totalSeeds];

	float timeLimitMs = gTimeLimit * 1000.0f;

	// Assume 1 second overhead
	timeLimitMs -= 1000.0f;

	// Handle startup time
	timeLimitMs -= MillisecondsFromMachineTimeDiffFloat(startTime, GetMachineTime());

	// Run each problem for the right amount of time
	float problemTimeMs = timeLimitMs / numProblems;

	HoneyOutputInfo info;
	HoneyInitOutputInfo(info, stdout);
	HoneyOutputHeader(info);
	InitOutput(2);

	// solve all puzzles
	for(int i = 0; i < numProblems; ++i)
	{
		scores[i] = RunnerSolveProblem(info, problemFile[i], &pAIs[startSeeds[i]], problemTimeMs);
	}

	HoneyOutputFooter(info);

	if (todMode)
	{
		float ellapse = MillisecondsFromMachineTimeDiffFloat(startTime, GetMachineTime());

		char msg[256];
		sprintf(msg, "run time %.03f seconds", ellapse / 1000.0f);
		puts(msg);
		OutputDebugStringA(msg);

	}

	if(drawSummary)
	{
		for(int i = 0; i < numProblems; ++i)
		{
			printf("%s: Score %d\n", problemFile[i], scores[i]);
		}
	}

	return 0;
}

#if !(defined(_WIN32) || defined(_WIN64))
    int main(int argc, const char* argv[])
    {
        return _tmain(argc, argv);
    }
#endif
