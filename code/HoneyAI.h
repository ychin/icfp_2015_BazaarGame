// HoneyAI.h
#ifndef HONEYAI_H
#define HONEYAI_H

#include "HoneySim.h"

const int MAX_MOVES = 256;

// Context for functors and gene perturbations
struct AIFuncContext;
typedef bool (*PerturbAIFnContext)(AIFuncContext& context);
struct AIFuncContext
{
    PerturbAIFnContext perturbFunc;

    float context1;
    float context2;
    float context3;
    float context4;
    float context5;
    float context6;
    float context7;
    float context8;
    float context9;
    float context10;
};

typedef void (*AIFn)();
typedef void (*AIFnContext)(const AIFuncContext& context);

struct AIFunction
{
    AIFn fn;
    AIFnContext fnContext;
	const char *str;
    AIFuncContext context;

	int usedInBestContextCount;
};

extern AIFunction ALGORITMS[];
extern const int NUM_ALGORITHMS;

struct Path
{
	Move moves[MAX_MOVES];
	int numMoves;
};

void AIRandom(Path &path);

void AIGreed();
void AITodsLineHunter();
void AIPowerWorder();
void AIVuGreedy();
void AIVuGreedy2();
void AIVuScoring();
void AIVuScoringLineable();
void AIVuScoring2();
void AIVuScoring3(const AIFuncContext& context);

bool PerturbVuScoringContext(AIFuncContext& context);
void AIVuScoringGeneral(
	float positionScoreWeight, 
	float distanceFromTopReward, 
	float closenessToWallsReward, 
	float lineCreationReward, 
	float holesFillReward, 
	float partialHoleFillReward, 
	float emptySpacePenalty, 
	float holeCreationPenalty,
	float orphanCreationPenalty);
void AIGreed2();
void AIGreedPerimeter();
void AIGreedNoHoles();
void AIPlaceFarthest();
void AIUnion(HoneyOutputInfo& output, int scoreUnits[], int scoreWords[]);
void AIGreedPerimeterLineseeker();
void AILineMaker();
//void AILineMaker2();
void AILineMaker2_Y();
void AILineMaker2_Cover();
void AIYCPowerPhrase();
struct GeneAI;
void AIGenes(GeneAI* pAI, HoneyOutputInfo& output, int scoresUnits[], int scoresWords[], int reps);

bool PerturbYCScoringContext(AIFuncContext& context);
void AIYCScoringCombined(const AIFuncContext& context);
void AIYCScoringLineableCombined(const AIFuncContext& context);
void AIYCScoringCombined2(const AIFuncContext& context);

void AIGenes(HoneyOutputInfo& output, int scoresUnits[], int scoresWords[]);
void RunAIWithContext(int algorithm, const AIFuncContext& context);
void RunAI(int algorithm);
void AIPerimeterForSmall();
//given a board, evaluate it's max score potential, and min score potential
//choose a board with the max score potential
//floodfill : compute all placements of the current piece (Tod, for now just assume you can put it everywhere!)
//choose a random placement

#endif