// HoneyAI.cpp

#include "Precompile.h"
#include <stdint.h>
#include "HoneyAI.h"
#include "HoneyGenes.h"

AIFunction ALGORITMS[] = {
	{ NULL, AIVuScoring3, "AIVuScoringGeneralDownPartialHoles", {
		PerturbVuScoringContext,
		0.5f, // positionScoreWeight
	    500.0f, // distanceFromTopReward
	    100.0f, // closenessToWallsReward
	
	    100.0f, // lineCreationReward
	    100.0f, // holesFillReward
	    400.0f, // partialHoleFillReward

	    -100.0f, // emptySpacePenalty
	    -100.0f, // holeCreationPenalty 
	    -100.0f, // orphanCreationPenalty
	} },
	/*
	{ NULL, AIVuScoring3, "AIVuScoringGeneral", {
        PerturbVuScoringContext,
        0.5f, // positionScoreWeight
	    100.0f, // distanceFromTopReward
	    25.0f, // closenessToWallsReward
	
	    5000.0f, // lineCreationReward
	    300.0f, // holesFillReward
	    200.0f, // partialHoleFillReward

	    -10.0f, // emptySpacePenalty
	    -1500.0f, // holeCreationPenalty 
	    -1500.0f, // orphanCreationPenalty
    } },
	/*
    { NULL, AIVuScoring3, "AIVuScoringGeneralPerturbed", {
        PerturbVuScoringContext,
		0.5f, // positionScoreWeight
	    100.0f, // distanceFromTopReward
	    100.0f, // closenessToWallsReward
	
	    100.0f, // lineCreationReward
	    100.0f, // holesFillReward
	    100.0f, // partialHoleFillReward

	    -100.0f, // emptySpacePenalty
	    -100.0f, // holeCreationPenalty 
	    -100.0f, // orphanCreationPenalty
    } },
	*/
	{ AIVuGreedy, NULL, "AIVuGreedy"},
	{ AIVuGreedy2, NULL, "AIVuGreedy2"},
	{ AIVuScoring, NULL,  "AIVuScoring"},
	{ AIVuScoringLineable, NULL,  "AIVuScoringLineable"},
	{ AIVuScoring2, NULL,  "AIVuScoring2"},

    { NULL, AIYCScoringCombined, "AIYCScoringCombined", { PerturbYCScoringContext, 4096, 4096 } },
    { NULL, AIYCScoringLineableCombined, "AIYCScoringLineableCombined", { PerturbYCScoringContext, 4096, 4096 } },
    { NULL, AIYCScoringCombined2, "AIYCScoringCombined2", { PerturbYCScoringContext, 4096, 4096 } },
	{ AILineMaker2_Cover,  NULL, "AILineMaker2_Cover" },
	{ AILineMaker2_Y,  NULL, "AILineMaker2_Y" },
	//{ AILineMaker, "AILineMaker" },
	//{ AIVuGreedy2, "AIVuGreedy2"},
	{ AIPlaceFarthest,  NULL, "AIPlaceFarthest" },
	//{ AIGreedNoHoles,	"AIGreedNoHoles" },
	{ AIGreedPerimeter,	 NULL, "AIGreedPerim" },
	//{ AIPerimeterForSmall,	 NULL, "AIPerimeterForSmall" },
	//{ AIGreedPerimeterLineseeker,	"AIGreedPerimeterLineseeker" },
	//{ AIGreed,	"AIGreed" },
	//{ AITodsLineHunter,	"AITodsLineHunter" },
	//{ AIGreed2,	"AIGreed2" },
};

const int NUM_ALGORITHMS = sizeof(ALGORITMS) / sizeof(ALGORITMS[0]);

Move RandomMove()
{
	return (Move)RandomInt(6);
}

void AIRandom(Path &path)
{
	path.moves[0] = RandomMove();
	path.numMoves = 1;
}

void AIGreed()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);
	//int index = RandomInt(gSim.board.numFinalPositions);
	int best = 0;
	for(int j = 1; j < gSim.board.numFinalPositions; ++j)
	{
		if (gSim.board.finalPositionList[j].poseFinal.position.y > gSim.board.finalPositionList[best].poseFinal.position.y)
		{
			best = j;
		}
	}
	HoneyChooseEnumerateMove(best);
}

BoardCellState AtBoard(Cell neighborPos)
{
	return gSim.board.cells[neighborPos.x][neighborPos.y].state;
}

Cell PositionInDir(Cell pos, int dir)
{
	Cell retval = pos;

	switch(dir)
	{
	case 0:
		retval.x += 1;
		break;
	case 1:
		retval.x -= 1;
		break;
	case 2:
		retval.x += IsEven(pos.y) ? 0 : 1;
		retval.y += 1;
		break;
	case 3:
		retval.x += IsEven(pos.y) ? -1 : 0;
		retval.y += 1;
		break;
	case 4:
		retval.x += IsEven(pos.y) ? 0 : 1;
		retval.y -= 1;
		break;
	case 5:
		retval.x += IsEven(pos.y) ? -1 : 0;
		retval.y -= 1;
		break;
	default:
		XHalt();
	}

	return retval;
}

int CalcCoveredNeigbors(Cell positions[], int numMembers)
{
	int coverage = 0;
	for(int i = 0; i < numMembers; ++i)
	{
		for(int dir = 0; dir < 6; ++dir)
		{
			Cell neighborPos = PositionInDir(positions[i], dir);
			if (OnBoard(neighborPos) == false)
			{
				if (neighborPos.y > gSim.problem.height / 2)
				{
					coverage+=2;
				}
				else
				{
					coverage++;
				}
			}
			else
			{
				if (AtBoard(neighborPos) == CELL_FILLED)
				{
					coverage+=2;
				}
			}
		}
	}

	return coverage;
}

int CalcBestY(Cell positions[], int numMembers)
{
	XAssert(numMembers > 0);
	int bestY = positions[0].y;
	for(int i = 1; i < numMembers; ++i)
	{
		if (positions[i].y > bestY)
		{
			bestY = positions[i].y;
		}
	}

	return bestY;
}

int TotalPieceArea()
{
	int total = 0;
	for(int i = 0; i < gSim.problem.numUnits; ++i)
	{
		total += gSim.problem.units[i].numMembers;
	}

	return total;
}

bool InMembers(Cell positions[], int numMembers, Cell pos)
{
	for(int mem = 0; mem < numMembers; ++mem)
	{
		if (positions[mem] == pos) return true;
	}

	return false;
}

int CalcNumHoles(Cell positions[], int numMembers)
{
	int numHoles = 0;

	for(int i = 0; i < gSim.problem.width; ++i)
	for(int j = 0; j < gSim.problem.height; ++j)
	{
		Vec2I pos = V2I(i,j);
		if (AtBoard(pos) != CELL_EMPTY) continue;

		int neigbors = 0;
		for(int dir = 0; dir < 6; ++dir)
		{
			Cell neighborPos = PositionInDir(pos, dir);
			if (OnBoard(neighborPos) == false)
			{
				neigbors++;
			}
			else
			{
				if (AtBoard(neighborPos) == CELL_FILLED)
				{
					neigbors++;
				}
				else if (InMembers(positions, numMembers, pos))
				{
					neigbors++;
				}
			}
		}

		if (neigbors == 6)
		{
			numHoles++;
		}

	}
	return numHoles;
}

int CalcPathLength(UnitPose unitPose)
{
	Cell position = GetUnitFirstPosition(gSim.board.currentUnitIndex, unitPose);
	ParentPointer& parentPointer = gSim.board.finalPositionsParents[position.x][position.y][unitPose.orientation];

	if (parentPointer.moveToHere != MOVE_INVALID)
	{
		if (parentPointer.poseParent.position == unitPose.position)
		{
			return CalcPathLength(parentPointer.poseParent);
		}
		else return 1 + CalcPathLength(parentPointer.poseParent);
	}

	return 0;
}

//int SqrDist(Cell a, Vec2I b)
//{
//	return SqrLen(a-b);
//}

void AIGenes(GeneAI *pAI, HoneyOutputInfo& output, int scoresUnits[], int scoresWords[], int reps)
{
	DisposeOutput();
	InitOutput(2);

	GeneAI geneAI;

	for(int n = 0; n < gSim.problem.numSourceSeeds; ++n)
	{
		GeneInit(pAI, n);
		CopyOutput(0, 1);

		int bestScoreUnits = 0;
		int bestScoreWords = 0;

		for(int i = 0; i < reps; ++i)
		{
			bool didsomething = GeneticStep(pAI);
			if (didsomething == false)
			{
				--i;
				continue;
			}

			if(gSim.board.totalScoreUnits + gSim.board.totalScoreWords > bestScoreUnits + bestScoreWords)
			{
				// mark ai in best sequence as useful
				GeneSolution* pBest = &pAI->solutions.front();
				int numGenes = pBest->sequence.size();
				for(int j = 0; j < numGenes; ++j)
				{
					Gene gene = pBest->sequence[j];
					ALGORITMS[gene.data].usedInBestContextCount++;
				}

				bestScoreUnits = gSim.board.totalScoreUnits;
				bestScoreWords = gSim.board.totalScoreWords;
				CopyOutput(0, 1);

				XTrace("New best: (prob %d, seedidx: %d, result: %d, steps:%d)\n", gSim.problem.id, n, bestScoreUnits + bestScoreWords, i);
				i = 0;
			}
		}

		scoresUnits[n] = bestScoreUnits;
		scoresWords[n] = bestScoreWords;
		HoneyOutputSolutionExt(output, n, 1, "Genetic", "AIGenes", bestScoreUnits + bestScoreWords);
	}

	DisposeOutput();
	InitOutput(1);
}

void AIUnion(HoneyOutputInfo& output, int scoresUnits[], int scoresWords[])
{

	DisposeOutput();
	InitOutput(2);

	for(int n = 0; n < gSim.problem.numSourceSeeds; ++n)
	{
		int bestScoreUnits = 0;
		int bestScoreWords = 0;
		int bestAlg = -1;

		for(int i = 0; i < NUM_ALGORITHMS; ++i)
		{
			HoneyStartGameSeed(n);
			while(1)
			{
				if (gSim.gameState == GAME_PLAYING)
				{
					RunAI(i);
				}
				else
				{
					break;
				}
			}

			if(gSim.board.totalScoreUnits + gSim.board.totalScoreWords > bestScoreUnits + bestScoreWords)
			{
				bestScoreUnits = gSim.board.totalScoreUnits;
				bestScoreWords = gSim.board.totalScoreWords;
				bestAlg = i;
				CopyOutput(0, 1);
			}
		}

		scoresUnits[n] = bestScoreUnits;
		scoresWords[n] = bestScoreWords;
		HoneyOutputSolutionExt(output, n, 1, ALGORITMS[bestAlg].str, "AIUnion", bestScoreUnits + bestScoreWords);
	}

	DisposeOutput();
	InitOutput(1);
}


//Ei!

struct Line
{
	int count;
};

enum Result
{
	RESULT_SAME,
	RESULT_BETTER,
	RESULT_WORSE,
};

Result NewLinesAreBetter(Line newLines[], int numNewLines, Line bestLines[], int numBestLines)
{
	int bestLenNew = -1;
	int countBestNew = 0;

	for(int i = 0; i < numNewLines; ++i)
	{
		if (newLines[i].count > bestLenNew)
		{
			bestLenNew = newLines[i].count;
			countBestNew = 1;
		}
		else if (newLines[i].count == bestLenNew)
		{
			countBestNew++;
		}
	}



	int bestLenBest = -1;
	int countBestBest = 0;

	for(int i = 0; i < numBestLines; ++i)
	{
		if (bestLines[i].count > bestLenBest)
		{
			bestLenBest = bestLines[i].count;
			countBestBest = 1;
		}
		else if (bestLines[i].count == bestLenBest)
		{
			countBestBest++;
		}
	}

	if (bestLenNew > bestLenBest) return RESULT_BETTER;
	if (bestLenNew == bestLenBest && countBestNew > countBestBest) return RESULT_BETTER;
	if (bestLenNew == bestLenBest && countBestNew == countBestBest) return RESULT_SAME;
	return RESULT_WORSE;
}

int CalcLines(Cell positions[], int numMembers, Line lines[MAX_HEIGHT])
{
	int numLines = 0;
	lines[numLines].count = 0;
	for(int j = 0; j < gSim.problem.height; ++j)
	{
		for(int i = 0; i < gSim.problem.width; ++i)
		{
			Vec2I pos = V2I(i,j);
			if (AtBoard(pos) == CELL_FILLED)
			{
				lines[numLines].count++;
			}
			else if (InMembers(positions, numMembers, pos))
			{
				lines[numLines].count++;
			}
		}

		if (lines[numLines].count > 0)
		{
			numLines++;
			lines[numLines].count = 0;
		}
	}

	return numLines;
}

void CalcNewBestLineDelta(Cell positions[], int numMembers, int &lineDeltaStart, int &lineDeltaEnd)
{
	Bounds bounds;
	XAssert(numMembers > 0);
	bounds.p0 = positions[0];
	bounds.p1 = positions[0];

	for(int i = 0; i < numMembers; ++i)
	{
		BoundsInflate(bounds, positions[i]);
	}

	lineDeltaStart = 0;
	lineDeltaEnd   = 0;

	for(int j = bounds.p0.y; j <= bounds.p1.y; ++j)
	{
		int start = 0;
		int delta = 0;
		for(int i = 0; i < gSim.problem.width; ++i)
		{
			Vec2I pos = V2I(i,j);
			if (AtBoard(pos) == CELL_FILLED)
			{
				start++;
			}
			else if (InMembers(positions, numMembers, pos))
			{
				delta++;
			}
		}

		if (delta > (lineDeltaEnd - lineDeltaStart))
		{
			lineDeltaStart = start;
			lineDeltaEnd = start+delta;
		}
		else if (delta == lineDeltaEnd - lineDeltaStart && start > lineDeltaStart)
		{
			lineDeltaStart = start;
			lineDeltaEnd = start+delta;
		}
	}
}

Result NewLineDeltaIsBetter(int lineDeltaStart, int lineDeltaEnd, int bestLineDeltaStart, int bestLineDeltaEnd)
{
	//if (lineDeltaEnd > bestLineDeltaEnd && lineDeltaEnd == gSim.problem.width)
	//{
	//	return RESULT_BETTER;
	//}

	int newDiff = lineDeltaEnd - lineDeltaStart;
	int oldDiff = bestLineDeltaEnd - bestLineDeltaStart;

	if (newDiff > oldDiff) return RESULT_BETTER;
	if (oldDiff > newDiff) return RESULT_WORSE;

	if (lineDeltaStart > bestLineDeltaStart) return RESULT_BETTER;
	if (lineDeltaStart < bestLineDeltaStart) return RESULT_WORSE;
	
	return RESULT_SAME;
}

void AILineMaker2_Base(bool yfirst)
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	//int bestPathLength = 0;
	//int bestPerim = 0;
	//Line bestLines[MAX_HEIGHT];
	//int bestNumLines = 0;
	int bestLineDeltaStart = 0;
	int bestLineDeltaEnd = 0;
	int bestY = 0;
	int bestCoverage = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

		//int pathLength = CalcPathLength(gSim.board.finalPositionList[j].poseFinal);
		int lineDeltaStart, lineDeltaEnd;
		CalcNewBestLineDelta(positions, numMembers, lineDeltaStart, lineDeltaEnd);
		int newY = CalcBestY(positions, numMembers);
		int newCoverage = CalcCoveredNeigbors(positions, numMembers);

		Result result = NewLineDeltaIsBetter(lineDeltaStart, lineDeltaEnd, bestLineDeltaStart, bestLineDeltaEnd);
		//if (best == -1 || 
		//	result == RESULT_BETTER || 
		//	result == RESULT_SAME && newCoverage > bestCoverage ||
		//	result == RESULT_SAME && newCoverage == bestCoverage && newY > bestY)
		if (yfirst)
		{
			if (best == -1 || 
				result == RESULT_BETTER || 
				result == RESULT_SAME && newY > bestY ||
				result == RESULT_SAME && newY == bestY && newCoverage > bestCoverage)
			{
				best = j;
				//bestY = newY;
				//bestPathLength = pathLength;
				//memcpy(bestLines, newLines, sizeof(Line) * numNewLines);
				//bestNumLines = numNewLines;
				bestLineDeltaStart = lineDeltaStart;
				bestLineDeltaEnd = lineDeltaEnd;
				bestY = newY;
				bestCoverage = newCoverage;
			}
		}
		else
		{
			if (best == -1 || 
				result == RESULT_BETTER || 
				result == RESULT_SAME && newCoverage > bestCoverage ||
				result == RESULT_SAME && newCoverage == bestCoverage && newY > bestY)
			{
				best = j;
				//bestY = newY;
				//bestPathLength = pathLength;
				//memcpy(bestLines, newLines, sizeof(Line) * numNewLines);
				//bestNumLines = numNewLines;
				bestLineDeltaStart = lineDeltaStart;
				bestLineDeltaEnd = lineDeltaEnd;
				bestY = newY;
				bestCoverage = newCoverage;
			}
		}
	}
	HoneyChooseEnumerateMove(best);
}

void AILineMaker2_Cover()
{
	AILineMaker2_Base(false);
}

void AILineMaker2_Y()
{
	AILineMaker2_Base(true);
}

void AILineMaker()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	//int bestPathLength = 0;
	//int bestPerim = 0;
	Line bestLines[MAX_HEIGHT];
	int bestNumLines = 0;
	int bestY = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

		//int pathLength = CalcPathLength(gSim.board.finalPositionList[j].poseFinal);
		Line newLines[MAX_HEIGHT];
		int numNewLines = CalcLines(positions, numMembers, newLines);
		int newY = CalcBestY(positions, numMembers);

		Result result = NewLinesAreBetter(newLines, numNewLines, bestLines, bestNumLines);
		if (best == -1 || result == RESULT_BETTER || (result == RESULT_SAME && newY > bestY))
		{
			best = j;
			//bestY = newY;
			//bestPathLength = pathLength;
			memcpy(bestLines, newLines, sizeof(Line) * numNewLines);
			bestNumLines = numNewLines;
			bestY = newY;
		}
	}
	HoneyChooseEnumerateMove(best);
}

void AIPlaceFarthest()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	int bestPathLength = 0;
	//int bestPerim = 0;
	int bestY = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

		//int pathLength = CalcPathLength(gSim.board.finalPositionList[j].poseFinal);
		int pathLength = SqrDist(gSim.board.finalPositionList[j].poseFinal.position, V2I(gSim.problem.width/2, 0));
		int newY = CalcBestY(positions, numMembers);

		if (best == -1 || (pathLength > bestPathLength) || (pathLength == bestPathLength && newY > bestY))
		{
			best = j;
			bestY = newY;
			bestPathLength = pathLength;
		}
	}
	HoneyChooseEnumerateMove(best);
}


// only slightly better on #18
void AIGreedNoHoles()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	int bestY = 0;
	int bestHoles = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

		int newHoles = CalcNumHoles(positions, numMembers);
		int newY = CalcBestY(positions, numMembers);

		if (best == -1 || (newHoles < bestHoles) || (newHoles == bestHoles && newY > bestY))
		{
			best = j;
			bestY = newY;
			bestHoles = newHoles;
		}
	}
	HoneyChooseEnumerateMove(best);
}

void AIGreedPerimeterLineseeker()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int lineableY = -1;
	for(int y = 0; y < gSim.problem.height; ++y)
	{
		int emptyCount = 0; 
		for(int x = 0; x < gSim.problem.width; ++x)
		{
			if(gSim.board.cells[x][y].state == CELL_EMPTY)
			{
				emptyCount++;
			}
		}

		if(emptyCount == 1)
		{
			// potential line
			lineableY = y;
			break;
		}
	}

	int best = -1;
	int bestY = 0;
	int bestCoverage = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

		for(int i = 0; i < numMembers; ++i)
		{
			if(lineableY != -1 && positions[i].y == lineableY)
			{
				HoneyChooseEnumerateMove(j);
				return;
			}
		}

		int newCoverage = CalcCoveredNeigbors(positions, numMembers);
		int newY = CalcBestY(positions, numMembers);

		if (best == -1 || (newCoverage > bestCoverage) || (newCoverage == bestCoverage && newY > bestY))
		{
			best = j;
			bestY = newY;
			bestCoverage = newCoverage;
		}
		//for(int i = 0; i < numMembers; ++i)
		//{
		//	if (best == -1 || positions[i].y > bestY)
		//	{
		//		best = j;
		//		bestY = positions[i].y;
		//	}
		//}
	}
	HoneyChooseEnumerateMove(best);
}

void AIGreedPerimeter()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	int bestY = 0;
	int bestCoverage = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);
		
		int newCoverage = CalcCoveredNeigbors(positions, numMembers);
		int newY = CalcBestY(positions, numMembers);

		if (best == -1 || (newCoverage > bestCoverage) || (newCoverage == bestCoverage && newY > bestY))
		{
			best = j;
			bestY = newY;
			bestCoverage = newCoverage;
		}
		//for(int i = 0; i < numMembers; ++i)
		//{
		//	if (best == -1 || positions[i].y > bestY)
		//	{
		//		best = j;
		//		bestY = positions[i].y;
		//	}
		//}
	}
	HoneyChooseEnumerateMove(best);
}

void AIGreed2()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	int bestY = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

		for(int i = 0; i < numMembers; ++i)
		{
			if (best == -1 || positions[i].y > bestY)
			{
				best = j;
				bestY = positions[i].y;
			}
		}
	}
	HoneyChooseEnumerateMove(best);
}

int AINumLinesCreated(int positionIndex)
{
	Cell positions[MAX_UNIT_MEMBERS];
	Cell pivot;
	int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[positionIndex].poseFinal, positions, pivot);

	Bounds bounds;
	bounds.p0 = positions[0];
	bounds.p1 = positions[0];

	for(int i = 0; i < numMembers; ++i)
	{
		XAssert(gSim.board.cells[positions[i].x][positions[i].y].state == CELL_EMPTY);
		gSim.board.cells[positions[i].x][positions[i].y].state = CELL_FILLED;
		BoundsInflate(bounds, positions[i]);
	}

	int rowsCleared = 0;

	for(int i = bounds.p0.y; i <= bounds.p1.y; ++i)
	{
		if (CheckRowClear(i))
		{
			rowsCleared++;
		}
	}

	// undo the set
	for(int i = 0; i < numMembers; ++i)
	{
		gSim.board.cells[positions[i].x][positions[i].y].state = CELL_EMPTY;
		BoundsInflate(bounds, positions[i]);
	}

	return rowsCleared;
}

void AITodsLineHunter()
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	int bestLines = 0;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		int lines = AINumLinesCreated(j);

		if (lines > bestLines)
		{
			bestLines = lines;
			best = j;
		}
	}

	if (best == -1)
	{
		int bestY = 0;

		for(int j = 0; j < gSim.board.numFinalPositions; ++j)
		{
			Cell positions[MAX_UNIT_MEMBERS];
			Cell pivot;
			int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

			for(int i = 0; i < numMembers; ++i)
			{
				if (best == -1 || positions[i].y > bestY)
				{
					best = j;
					bestY = positions[i].y;
				}
			}
		}
	}

	HoneyChooseEnumerateMove(best);
}

void AIPowerWorder()
{
	PowerStep powerStep = POWERSTEP_INVALID;

	for(int i = 0; i < NUM_POWERSTEPS; ++i)
	{
		powerStep = (PowerStep)gSim.sortedPowerPhrasesForUnused[i];

		int bit = 1 << powerStep;
		if ((gSim.board.powerStepUsedBitField & bit) == 0)
		{
			break; // unused power phrase found
		}
	}

	if(powerStep < 0)
	{
		// all power words used so do something else
		//AIVuGreedy();
		return;
	}
	
	PowerWord& word = gSim.powerPhrases[powerStep];
	if(word.len <= 0)
	{
		//AIVuGreedy();
		return;
	}

	for(int i = 0; i < word.len; ++i)
	{
		Move move = HoneyCommandToMove(word.chars[i]);

		UnitPose newPose = PoseMove(gSim.board.currentUnitPose, move);
		MoveResult result = ValidatePose(newPose);

		if (result == MOVERESULT_INVALID)
		{
			//AIVuGreedy();
			return;
		}

		HoneyMoveCommand(move, word.chars[i]);
	}
}

void AIVuGreedy()
{
	// lowest and furthest to left and right
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);	

	int best = 0;
	int bestY = -1;
	int bestDist = -1;
	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);

		for(int i = 0; i < numMembers; ++i)
		{
			if (bestY == -1 || positions[i].y > bestY)
			{
				best = j;
				bestY =  positions[i].y;
				bestDist = -1;
			}

			if(bestY == positions[i].y)
			{
				int dist = Min(positions[i].x, gSim.problem.width - positions[i].x);
				if(bestDist == -1 || dist < bestDist)
				{
					bestDist = dist;
					best = j;
				}
			}
		}
	}

	HoneyChooseEnumerateMove(best);
	return;
}

void AIVuGreedy2()
{
	// lowest and furthest to left and right
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);
	
	int best = 0;
	int bestY = -1;
	int bestDist = -1;

    int maxNumLinesCleared = 0;
    int bestMoveForClearingLines = -1;
    int bestMoveForClearingLinesY = -1;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
        int numLinesCleared = 0 ;

		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
        Bounds bounds;
		int numMembers = GetUnitPositionsWithBounds(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot, bounds);

		for(int i = 0; i < numMembers; ++i)
		{
            if ( (--gSim.board.numEmptyCellsPerRow[positions[i].y]) == 0)
			{
                numLinesCleared += 1;
			}
            
			if (bestY == -1 || positions[i].y > bestY)
			{
				best = j;
				bestY =  positions[i].y;
				bestDist = -1;
			}

			if(bestY == positions[i].y)
			{
				int dist = Min(positions[i].x, gSim.problem.width - positions[i].x);
				if(bestDist == -1 || dist < bestDist)
				{
					bestDist = dist;
					best = j;
				}
			}
		}

        for (int i = 0; i < numMembers; ++i)
        {
            gSim.board.numEmptyCellsPerRow[positions[i].y] += 1;
	}

        if (numLinesCleared > 0)
        {
            if (numLinesCleared > maxNumLinesCleared)
            {
                maxNumLinesCleared = numLinesCleared;
                bestMoveForClearingLines = j;
                bestMoveForClearingLinesY = bounds.p0.y;
            }
            else if (numLinesCleared == maxNumLinesCleared)
            {
                if (bounds.p0.y > bestMoveForClearingLinesY)
                {
                    maxNumLinesCleared = numLinesCleared;
                    bestMoveForClearingLines = j;
                    bestMoveForClearingLinesY = bounds.p0.y;
                }
            }
        }
	}

    if (bestMoveForClearingLines != -1)
        HoneyChooseEnumerateMove(bestMoveForClearingLines);
    else
	HoneyChooseEnumerateMove(best);
	return;
}

void AIVuScoringBase(bool useLineability, int vuScoreScale, int powerScoreScale);

void AIVuScoringLineable()
{
    AIVuScoringBase(true, 1, 0);
}

void AIVuScoring()
{
    AIVuScoringBase(false, 1, 0);
}

void AIVuScoringBase(bool useLineability, int globalVuScoreScale, int globalPowerScoreScale)
{
    // Tuning values for blending utility scores + power phrase scores
    int64_t vuScoreScale = 100 * globalVuScoreScale; // 100%
    int64_t powerScoreScale = (gSim.problem.height * 20 + gSim.problem.width * 10) * globalPowerScoreScale;


	// lowest and furthest to left and right based on a scoring system
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = 0;
	int64_t bestScore = -1;
	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
        int numLinesCleared = 0;
		
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
        Bounds bounds;
		int numMembers = GetUnitPositionsWithBounds(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot, bounds);

		int score = 0;
		for(int i = 0; i < numMembers; ++i)
		{
            if (useLineability && gSim.board.numEmptyCellsPerRow[positions[i].y] == 1)
			{
				HoneyChooseEnumerateMove(j);
				return;
			}

            // We could count lines cleared to add to score but seems like the above heuristic somehow gives better results than maximizing lines clear...
            if (!useLineability)
            {
                if ( (--gSim.board.numEmptyCellsPerRow[positions[i].y]) == 0)
                {
                    numLinesCleared += 1;
                }
            }
			
			int dist = Min(positions[i].x, gSim.problem.width - positions[i].x);
			score += positions[i].y * 100 + (gSim.problem.width-dist) * 25; 
		}

        if (!useLineability)
		{
            for (int i = 0; i < numMembers; ++i)
            {
                gSim.board.numEmptyCellsPerRow[positions[i].y] += 1;
            }

            score += numLinesCleared * gSim.problem.height * 400; // need to normalize by height to keep the scoring consistent
        }

        int64_t powerScore = gSim.board.finalPositionList[j].powerScore;

        int64_t combinedScore = score* vuScoreScale + powerScore * powerScoreScale * numMembers;

		if(combinedScore > bestScore)
		{
			bestScore = combinedScore;
			best = j;
		}
	}

	HoneyChooseEnumerateMove(best);
	return;
}


void AIYCPowerPhrase()
{
    // WIP
    // This will be a AI move to match power phrase for one unit only (won't do multi-unit invocation). Hopefully the genetic algorithm will be able to tune this
    // Also will need a backup if there's no power phrase left
}

int GetNeighborCount(Cell pos)
{
	int neigbors = 0;
	for(int dir = 0; dir < 6; ++dir)
	{
		Cell neighborPos = PositionInDir(pos, dir);
        if (neighborPos.x < 0 || neighborPos.x >= gSim.problem.width || neighborPos.y < 0 || neighborPos.y >= gSim.problem.height)
        {
            neigbors++;
            continue;
        }
		if (AtBoard(neighborPos) == CELL_FILLED)
		{
			neigbors++;
		}
	}

	return neigbors;
}

int GetNumEmptyCellsProduced(int finalPositionIndex)
{
	Cell positions[MAX_UNIT_MEMBERS];
	Cell pivot;
	int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[finalPositionIndex].poseFinal, positions, pivot);

	Bounds bounds;
	bounds.p0 = positions[0];
	bounds.p1 = positions[0];

	for(int i = 0; i < numMembers; ++i)
	{
		XAssert(gSim.board.cells[positions[i].x][positions[i].y].state == CELL_EMPTY);
		gSim.board.cells[positions[i].x][positions[i].y].state = CELL_FILLED;
		BoundsInflate(bounds, positions[i]);
	}

	int total = 0;
	int numFilled = 0;
	int numHolesCreated = 0;
	int numHolesFilled = 0;
	//int numPotentialHoles = 0;
	int numLinesCompleted = 0;
	for(int y = bounds.p0.y; y <= bounds.p1.y; ++y)
	{
		int numFilledOnThisLine = 0;
		for(int x = 0; x < gSim.problem.width; ++x)
		{
			Cell pos;
			pos.x = x;
			pos.y = y;
			
			int numNeighbors = GetNeighborCount(pos);

			++total;
			if(gSim.board.cells[x][y].state == CELL_FILLED)
			{
				++numFilled;
				++numFilledOnThisLine;
				if(numNeighbors == 6)
				{
					++numHolesFilled;
				}
			}
			else if(numNeighbors == 6)
			{
				++numHolesCreated;
			}


			/*
			else if(numNeighbors > 3)
			{
				++numPotentialHoles;
			}
			*/
		}

		if(numFilledOnThisLine == gSim.problem.width)
		{
			++numLinesCompleted;
		}
	}

	int numEmpty = total-numFilled;

	int score = numEmpty; 
	score -= numLinesCompleted*gSim.problem.width/2; // reward for lines
	score -= numHolesFilled*gSim.problem.width; // reward for filling holes
	score += numHolesCreated*gSim.problem.width;  // penalize for creating holes
	//score += numPotentialHoles*gSim.problem.width; // penalize for potential holes
	if(numFilled == numMembers)
	{
		score = -1;
	}

	// undo the set
	for(int i = 0; i < numMembers; ++i)
	{
		gSim.board.cells[positions[i].x][positions[i].y].state = CELL_EMPTY;
		BoundsInflate(bounds, positions[i]);
	}

	return score;
}

int GetVuScore(int finalPositionIndex)
{
	Cell positions[MAX_UNIT_MEMBERS];
	Cell pivot;
	int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[finalPositionIndex].poseFinal, positions, pivot);

	int score = 0;
	for(int i = 0; i < numMembers; ++i)
	{			
	    int dist = Min(positions[i].x, gSim.problem.width - positions[i].x);
	    score += positions[i].y * 100 + (gSim.problem.width-dist) * 25; 
	}
	return score;
}

typedef long long int64;

int64 LerpVuScoreWithPowerScore(int finalPositionIndex, int64 vuScoreScale, int64 powerScoreScale)
{
    int64 vuScore = GetVuScore(finalPositionIndex);
    int64 powerScore = gSim.board.finalPositionList[finalPositionIndex].powerScore;

    return vuScore * vuScoreScale + powerScore * vuScoreScale;
}

void AIVuScoring2()
{
	// lowest and furthest to left and right based on a scoring system
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	int bestScore = -1;
	int leastEmptyCellProduction = gSim.problem.width*gSim.problem.height;
	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		int emptyCellProduction = GetNumEmptyCellsProduced(j);
		if(emptyCellProduction != -1)
		{
			if(emptyCellProduction < leastEmptyCellProduction)
			{
				leastEmptyCellProduction = emptyCellProduction;
				bestScore = GetVuScore(j);
				best = j;
			}
			else if(emptyCellProduction == leastEmptyCellProduction)
			{
				int score = GetVuScore(j);
				if(score > bestScore)
				{
					bestScore = score;
					best = j;
				}
			}
		}
	}

	if(best == -1)
	{
		AIVuScoring();
	}
	else
	{
		HoneyChooseEnumerateMove(best);
	}
}

void AIVuScoring3(const AIFuncContext& context)
{
	const float positionScoreWeight = context.context1;
	const float distanceFromTopReward = context.context2;
	const float closenessToWallsReward = context.context3;
	
	const float lineCreationReward = context.context4;
	const float holesFillReward = context.context5;
	const float partialHoleFillReward = context.context6;

	const float emptySpacePenalty = context.context7;
	const float holeCreationPenalty = context.context8;
	const float orphanCreationPenalty = context.context9;

	AIVuScoringGeneral(positionScoreWeight, distanceFromTopReward, closenessToWallsReward, lineCreationReward, holesFillReward, partialHoleFillReward, emptySpacePenalty, holeCreationPenalty, orphanCreationPenalty);
}

bool PerturbVuScoringContext(AIFuncContext& context)
{
    // Vu: fill this in, and return true if you have perturbed something
	/*
    float perturbPercent = 0.25f;
	
	context.context1 = Clamp(context.context1+((RandomInt(100) > 50)?-1:1)*context.context1*perturbPercent, 0.0f, 1.0f); // positionScoreWeight
	context.context2 += ((RandomInt(100) > 50)?-1:1)*context.context2*perturbPercent; // distanceFromTopReward
	context.context3 += ((RandomInt(100) > 50)?-1:1)*context.context3*perturbPercent; // closenessToWallsReward
	
	context.context4 += ((RandomInt(100) > 50)?-1:1)*context.context4*perturbPercent; // lineCreationReward
	context.context5 += ((RandomInt(100) > 50)?-1:1)*context.context5*perturbPercent; // holesFillReward
	context.context6 += ((RandomInt(100) > 50)?-1:1)*context.context6*perturbPercent; // partialHoleFillReward

	context.context7 += ((RandomInt(100) > 50)?-1:1)*context.context7*perturbPercent; // emptySpacePenalty
	context.context8 += ((RandomInt(100) > 50)?-1:1)*context.context8*perturbPercent; // holeCreationPenalty 
	context.context9 += ((RandomInt(100) > 50)?-1:1)*context.context9*perturbPercent; // orphanCreationPenalty
	*/
	float changeAmount = 100.0f;
	

	int valueToChange = RandomInt(9);
	switch(valueToChange)
	{
	case 0:
		context.context1 = Clamp(context.context1+((RandomInt(100) > 50)?-1:1)*0.05f*RandomInt(11), 0.0f, 1.0f); // positionScoreWeight
		break;

	case 1:
		context.context2 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // distanceFromTopReward
		break;

	case 2:
		context.context3 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // closenessToWallsReward
		break;

	case 3:
		context.context4 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // lineCreationReward
		break;

	case 4:
		context.context5 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // holesFillReward
		break;

	case 5:
		context.context6 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // partialHoleFillReward
		break;

	case 6:
		context.context7 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // emptySpacePenalty
		break;

	case 7:
		context.context8 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // holeCreationPenalty 
		break;

	case 8:
		context.context9 += ((RandomInt(100) > 50)?-1:1)*changeAmount*RandomInt(11); // orphanCreationPenalty
		break;
	}
    return true;
}

void AIVuScoringGeneral(
	float positionScoreWeight, 
	float distanceFromTopReward, 
	float closenessToWallsReward, 
	float lineCreationReward, 
	float holesFillReward, 
	float partialHoleFillReward, 
	float emptySpacePenalty, 
	float holeCreationPenalty,
	float orphanCreationPenalty)
{
	HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	float bestScore = -1;

	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		Cell positions[MAX_UNIT_MEMBERS];
		Cell pivot;
		int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.finalPositionList[j].poseFinal, positions, pivot);
		
		float positionScore = 0;
		int numHolesFilled = 0;
		int numPartialHolesFilled = 0;
		int numOrphaned = 0;

		int minY = gSim.problem.height;
		int maxY = -1;

		// place cells into board and get affected bounds
		for(int i = 0; i < numMembers; ++i)
		{
			Cell unitCell = positions[i];

			XAssert(gSim.board.cells[unitCell.x][unitCell.y].state == CELL_EMPTY);
			// place cells into board
			gSim.board.cells[unitCell.x][unitCell.y].state = CELL_FILLED;

			// expand bounds
			if(unitCell.y < minY)
			{
				minY = unitCell.y;
			}
			if(unitCell.y > maxY)
			{
				maxY = unitCell.y;
			}

			// calculate positionScore
			int dist = Min(unitCell.x, gSim.problem.width - unitCell.x);
			positionScore += unitCell.y * distanceFromTopReward + (gSim.problem.width-dist) * closenessToWallsReward; 

			// count number of neighbors filled
			int numNeighbors = GetNeighborCount(unitCell);
			if(numNeighbors == 6)
			{
				++numHolesFilled;
			}
			else if(numNeighbors == 5)
			{
				++numPartialHolesFilled;
			}
			else if(numNeighbors == 0)
			{
				++numOrphaned;
			}
		}

		
		float actionScore = 0;

		int totalCellsInAffectedLines = 0;
		int numFilled = 0;
		int numEmpty = 0;
		int numLinesCreated = 0;
		int numHolesCreated = 0;
		for(int y = minY; y <= maxY; ++y)
		{
			int numFilledOnThisLine = 0;
			for(int x = 0; x < gSim.problem.width; ++x)
			{
				++totalCellsInAffectedLines;
				BoardCell cell = gSim.board.cells[x][y];

				if(cell.state == CELL_FILLED)
				{
					++numFilled;
					++numFilledOnThisLine;
				}
				else
				{
					int numNeighbors = GetNeighborCount(V2I(x,y));
					if(numNeighbors == 6)
					{
						++numHolesCreated;
					}
				}
			}

			if(numFilledOnThisLine == gSim.problem.width)
			{
				++numLinesCreated;
			}
		}

		// remove unit cells from board
		for(int i = 0; i < numMembers; ++i)
		{
			gSim.board.cells[positions[i].x][positions[i].y].state = CELL_EMPTY;
		}

		numEmpty = totalCellsInAffectedLines-numFilled;

		// get action score
		actionScore += numLinesCreated*lineCreationReward;
		actionScore += numHolesFilled*holesFillReward;
		actionScore += numPartialHolesFilled*partialHoleFillReward;

		actionScore += numHolesCreated*holeCreationPenalty;
		actionScore += numEmpty*emptySpacePenalty;
		actionScore += numOrphaned*orphanCreationPenalty;
		
		// apply position score
		float finalScore= positionScore*positionScoreWeight + actionScore*(1.0f-positionScoreWeight); 
	
		if(best == -1 || finalScore > bestScore)
		{
			bestScore = finalScore;
			best = j;
		}
	}
	//XTrace("BestScore: %f", bestScore);
	HoneyChooseEnumerateMove(best);
}

bool PerturbYCScoringContext(AIFuncContext& context)
{
    if (RandomInt(100) < 50)
    {
        if (context.context1 > 2 && context.context2 < 4096 * 4096)
        {
            context.context1 = context.context1 * 3 / 4;
            context.context2 = context.context2 * 4 / 3;
            return true;
        }
    }
    else
    {
        if (context.context2 > 2 && context.context1 < 4096 * 4096)
        {
            context.context1 = context.context1 * 4 / 3;
            context.context2 = context.context2 * 3 / 4;
            return true;
        }
    }
    return false;
}

void AIYCScoringCombined(const AIFuncContext& context)
{
    int globalVuScoreScale = (int)context.context1;
    int globalPowerScoreScale = (int)context.context2;
    AIVuScoringBase(false, globalVuScoreScale, globalPowerScoreScale);
}

void AIYCScoringLineableCombined(const AIFuncContext& context)
{
    int globalVuScoreScale = (int)context.context1;
    int globalPowerScoreScale = (int)context.context2;
    AIVuScoringBase(true, globalVuScoreScale, globalPowerScoreScale);
}


void AIYCScoringCombined2(const AIFuncContext& context)
{
	// Basically AIVuScoring2 + power phrase score heuristic
    int64 globalVuScoreScale = (int64)context.context1;
    int64 globalPowerScoreScale = (int64)context.context2;
    int64 vuScoreScale = 100 * globalVuScoreScale; // 100%
    int64 powerScoreScale = (gSim.problem.height * 100 + gSim.problem.width * 40) * globalPowerScoreScale;

    HoneyEnumerateFinalPositions();
	XAssert(gSim.board.numFinalPositions > 0);

	int best = -1;
	int64 bestScore = -1;
	int leastEmptyCellProduction = gSim.problem.width*gSim.problem.height;
	for(int j = 0; j < gSim.board.numFinalPositions; ++j)
	{
		int emptyCellProduction = GetNumEmptyCellsProduced(j);
		if(emptyCellProduction != -1)
		{
			if(emptyCellProduction < leastEmptyCellProduction)
			{
				leastEmptyCellProduction = emptyCellProduction;
				bestScore = LerpVuScoreWithPowerScore(j, vuScoreScale, powerScoreScale * UnitGet(gSim.board.currentUnitIndex).numMembers);
				best = j;
			}
			else if(emptyCellProduction == leastEmptyCellProduction)
			{
				int64 score = LerpVuScoreWithPowerScore(j, vuScoreScale, powerScoreScale * UnitGet(gSim.board.currentUnitIndex).numMembers);
				if(score > bestScore)
				{
					bestScore = score;
					best = j;
				}
			}
		}
	}

	if(best == -1)
	{
		AIVuScoring();
	}
	else
	{
		HoneyChooseEnumerateMove(best);
	}
}


void RunAIWithContext(int algorithm, const AIFuncContext& context)
{
	const AIFunction& aiFunc = ALGORITMS[algorithm];
	if (aiFunc.fnContext)
	{
		aiFunc.fnContext(context);
	}
	else
	{
		aiFunc.fn();
	}
}

void RunAI(int algorithm)
{
	const AIFunction& aiFunc = ALGORITMS[algorithm];
	RunAIWithContext(algorithm, aiFunc.context);
}
