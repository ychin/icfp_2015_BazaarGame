// HoneySim.cpp

#include "Precompile.h"
#include "HoneySim.h"
#include <algorithm>

#define USE_FINAL_POSITIONS_WITH_SCORE true

Sim gSim;

void GameLCGInit(GameLCG &rng, uint32 seed)
{
	rng.state = seed;
}

const uint32 LCG_MULTIPLIER = 1103515245;
const uint32 LCG_INCREMENT = 12345;

uint32 GameLCGNext(GameLCG &rng)
{
	uint32 retval = (rng.state >> 16) & ((1<<15)-1);
	rng.state = LCG_MULTIPLIER * rng.state + LCG_INCREMENT;
	return retval;
}


//void BoardInit(Board &board, Problem &problem)
//{
//	board.width = problem.width;
//	board.height = problem.height;
//}

const Unit &UnitGet(int unitIndex)
{
	XAssert(unitIndex >= 0);
	XAssert(unitIndex < gSim.problem.numUnits);
	return gSim.problem.units[unitIndex];
}

void BoundsInflate(Bounds &bounds, const Cell &pos)
{
	if (pos.x < bounds.p0.x) bounds.p0.x = pos.x;
	else if (pos.x > bounds.p1.x) bounds.p1.x = pos.x;

	if (pos.y < bounds.p0.y) bounds.p0.y = pos.y;
	else if (pos.y > bounds.p1.y) bounds.p1.y = pos.y;
}

Bounds UnitCalcBounds(const Unit &unit)
{
	Bounds retval;
	XAssert(unit.numMembers > 0);
	retval.p0 = unit.members[0];
	retval.p1 = unit.members[0];

	for(int i = 1; i < unit.numMembers; ++i)
	{
		BoundsInflate(retval, unit.members[i]);
	}

	return retval;
}

Cell CalcSpawnBosition(const Unit &unit)
{
	Bounds bounds = UnitCalcBounds(unit);

	Cell retval;

	retval.y = -bounds.p0.y;
	retval.x = (gSim.problem.width - (bounds.p0.x + bounds.p1.x) - 1) / 2;

	return retval;
}

bool OnBoard(Cell pos)
{
	if (pos.x < 0) return false;
	if (pos.y < 0) return false;

	if (pos.x >= gSim.problem.width) return false;
	if (pos.y >= gSim.problem.height) return false;

	return true;
}

Cell AxialToCell(Axial axial)
{
	Cell retval;
	retval.x = axial.x + (axial.z - (axial.z&1))/2;
	retval.y = axial.z;
	return retval;
}

Axial CellToAxial(Cell cell)
{
	Axial retval;
	retval.x = cell.x - (cell.y - (cell.y&1)) / 2;
	retval.z = cell.y;
	retval.y = -retval.x-retval.z;
	return retval;
}

Cell CellTransform(const UnitPose& unitPose, const Unit& unitDef, int memberIndex)
{
	Axial aorigin = CellToAxial(unitPose.position);
	Axial diff = unitDef.offsetCache[unitPose.orientation][memberIndex];
	Axial aresult = aorigin + diff;
	Cell retval = AxialToCell(aresult);
	return retval;
}

Cell CellTransformWithPosition(Axial origin, int orientation, const Unit& unitDef, int memberIndex)
{
	Axial diff = unitDef.offsetCache[orientation][memberIndex];
	Axial aresult = origin + diff;
	Cell retval = AxialToCell(aresult);
	return retval;
}

MoveResult ValidatePose(UnitPose newPose)
{
	//Cell positions[MAX_UNIT_MEMBERS];
	//Cell pivot;
	//int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, newPose, positions, pivot);

	//for(int i = 0; i < numMembers; ++i)
	//{
	//	if (OnBoard(positions[i]) == false)
	//	{
	//		return MOVERESULT_GAME_OVER;
	//	}

	//	if (i == 0)
	//	{
	//		int bits = 1 << newPose.orientation;
	//		int currentBits = gSim.board.usedPositions[positions[i].x][positions[i].y];
	//		if ((currentBits & bits) != 0)
	//		{
	//			return MOVERESULT_INVALID;
	//		}
	//	}

	//	if (gSim.board.cells[positions[i].x][positions[i].y].state == CELL_FILLED)
	//	{
	//		return MOVERESULT_GAME_OVER;
	//	}
	//}

	///*
	const Unit &unitDef = UnitGet(gSim.board.currentUnitIndex);
	int parity = (int)!IsEven(newPose.position.y);

	//Axial origin = CellToAxial(newPose.position);
	//Cell pivot = AxialToCell(CellToAxial(unitDef.pivot) + origin);

	{
		Cell position = newPose.position + unitDef.cellCache[newPose.orientation][parity][0];
		//Cell position = CellTransform(newPose, unitDef, 0);

		if (OnBoard(position) == false)
		{
			return MOVERESULT_GAME_OVER;
		}

		int bits = 1 << newPose.orientation;
		int currentBits = gSim.board.usedPositions[position.x][position.y];
		if ((currentBits & bits) != 0)
		{
			return MOVERESULT_INVALID;
		}

		if (gSim.board.cells[position.x][position.y].state == CELL_FILLED)
		{
			return MOVERESULT_GAME_OVER;
		}
	}

	for(int i = 1; i < unitDef.numMembers; ++i)
	{
		Cell position = newPose.position + unitDef.cellCache[newPose.orientation][parity][i];
		//Cell position = CellTransform(newPose, unitDef, i);
		//Cell position = CellTransformWithPosition(origin, newPose.orientation, unitDef, i);

		if (OnBoard(position) == false)
		{
			return MOVERESULT_GAME_OVER;
		}

		if (gSim.board.cells[position.x][position.y].state == CELL_FILLED)
		{
			return MOVERESULT_GAME_OVER;
		}
	}
	//*/

	return MOVERESULT_OK;
}

bool BoardLoadUnit(Board &board)
{
	gSim.board.numFinalPositions = 0;

	for(int i = 0; i < gSim.problem.width; ++i)
	{
		for(int j = 0; j < gSim.problem.height; ++j)
		{
			board.usedPositions[i][j] = 0;
			for (int orientation = 0; orientation < NUM_ORIENTATIONS; orientation++)
			{
				gSim.board.finalPositionsParents[i][j][orientation].moveToHere = MOVE_INVALID;
			}
		}
	}

	uint32 spawnIndex = GameLCGNext(board.lcg) % gSim.problem.numUnits;
	board.currentUnitIndex = spawnIndex;
	const Unit &unit = UnitGet(board.currentUnitIndex);
	board.currentUnitPose.position = CalcSpawnBosition(unit);
	board.currentUnitPose.orientation = 0;
	board.unitsSpawned++;

	if (ValidatePose(board.currentUnitPose) != MOVERESULT_OK)
	{
		return false;
	}

	HoneyMarkPoseAsUsed(board.currentUnitPose);

	return true;
}

void BoardLoad(Board &board, int gameIndex)
{
	for(int i = 0; i < gSim.problem.width; ++i)
	{
		for(int j = 0; j < gSim.problem.height; ++j)
		{
			//int index = i + j * board.width;
			board.cells[i][j].state = CELL_EMPTY;
		}
	}

    for (int i = 0; i < gSim.problem.height; ++i)
    {
        board.numEmptyCellsPerRow[i] = gSim.problem.width;
    }

	for(int i = 0; i < gSim.problem.numFilled; ++i)
	{
		//int index = problem.filled[i].x + problem.filled[i].y * board.width;
		//board.cells[index].state = CELL_FILLED;

		int x = gSim.problem.filled[i].x;
		int y = gSim.problem.filled[i].y;
		board.cells[x][y].state = CELL_FILLED;
        //board.cells[x][y].unitNum = -1;

        board.numEmptyCellsPerRow[y] -= 1;
	}

	board.currentUnitIndex = -1;
	board.linesClearedPrev = 0;
	board.last_TileScore = 0;
	board.lastWordCount = 0;
	board.lastWordScore = 0;
	board.currentWordScore = 0;
	board.currentWordCount = 0;
	board.totalScoreUnits = 0;
	board.totalScoreWords = 0;
    board.currentWordIndex = -1;

	//GameLCGInit(board.lcg, 17);
	//for(int i = 0; i < 10; ++i)
	//{
	//	XTrace("%d", GameLCGNext(board.lcg));
	//}

	GameLCGInit(board.lcg, gSim.problem.sourceSeeds[gameIndex]);


	board.unitsSpawned = 0;
	gSim.gameState = GAME_PLAYING;

	BoardLoadUnit(board);
}

bool IsEven(int y)
{
	return (y%2 == 0);
}

void PoseClampOrientation(UnitPose &pose)
{
	// Clamp to symmetry

	XAssert(gSim.board.currentUnitIndex >= 0 && gSim.board.currentUnitIndex < gSim.problem.numUnits);
	Unit& unit = gSim.problem.units[gSim.board.currentUnitIndex];

	static const int symmetries[] = { 6, 1, 2, 3 };
	int sym = symmetries[unit.symmetry];
	if (pose.orientation < 0) pose.orientation += sym;
	else if (pose.orientation >= sym) pose.orientation -= sym;


	//if (unit.symmetry == SYMMETRY_NONE)
	//{
	//	if (pose.orientation < 0) pose.orientation += 6;
	//	else if (pose.orientation >= 6) pose.orientation -= 6;
	//}
	//else if (unit.symmetry == SYMMETRY_1)
	//{
	//	pose.orientation = 0;
	//}
	//else if (unit.symmetry == SYMMETRY_2)
	//{
	//	if (pose.orientation < 0) pose.orientation += 2;
	//	else if (pose.orientation >= 2) pose.orientation -= 2;
	//}
	//else if (unit.symmetry == SYMMETRY_3)
	//{
	//	if (pose.orientation < 0) pose.orientation += 3;
	//	else if (pose.orientation >= 3) pose.orientation -= 3;
	//}
	//else
	//{
	//	XHalt();
	//}
}

UnitPose PoseMove(UnitPose pose, Move move)
{

	//x   evenx, oddx, y,  turn
	struct MoveNums
	{
		int x;
		int y;
		int orientation;
	};

	const static MoveNums MOVESTUFF[2][6] = {
		{
			{-1, 0, 0},
			{ 1, 0, 0},
			{-1, 1, 0},
			{ 0, 1, 0},
			{ 0, 0, 1},
			{ 0, 0,-1},
		},

		{
			{-1, 0, 0},
			{ 1, 0, 0},
			{ 0, 1, 0},
			{ 1, 1, 0},
			{ 0, 0, 1},
			{ 0, 0,-1},
		},
	};

	UnitPose retval2 = pose;

	int parity = pose.position.y & 1;
	MoveNums MOVE = MOVESTUFF[parity][move];
	retval2.position.x += MOVE.x;
	retval2.position.y += MOVE.y;
	retval2.orientation += MOVE.orientation;
	PoseClampOrientation(retval2);
	
	return retval2;

	//UnitPose retval = pose;

	//switch(move)
	//{
	//case MOVE_W:
	//	retval.position.x -= 1;
	//	break;
	//case MOVE_E:
	//	retval.position.x += 1;
	//	break;

	//case MOVE_SW:
	//	retval.position.x += IsEven(pose.position.y) ? -1 : 0;
	//	//retval.position.x -= (~pose.position.y & 1);
	//	retval.position.y += 1;
	//	break;

	//case MOVE_SE:
	//	//retval.position.x += IsEven(pose.position.y) ? 0 : 1;
	//	retval.position.x += (pose.position.y & 1);
	//	retval.position.y += 1;
	//	break;

	//case TURN_CW:
	//	retval.orientation += 1;
	//	PoseClampOrientation(retval);
	//	break;
	//
	//case TURN_CCW:
	//	retval.orientation -= 1;
	//	PoseClampOrientation(retval);
	//	break;


	//default:
	//	XHalt();
	//}

	//XAssert(retval.position == retval2.position);
	//XAssert(retval.orientation == retval2.orientation);

	//return retval;
}

//void UnitGetCells(Unit &unit, UnitPose &pose)
//{
//
//}

//void BoardSim(Board &board, Move move)
//{
//
//}

//void HoneyLoadProblem()
//{
//	gSim.problem.id = 0;
//	gSim.problem.width = 10;
//	gSim.problem.height = 10;
//	static int SOURCE_SEEDS[] = {0};
//	gSim.problem.sourceSeeds = SOURCE_SEEDS;
//	gSim.problem.numSourceSeeds = ARRAYSIZE(SOURCE_SEEDS);
//	static Cell UNITS_MEMBERS[] = {0,0, -1,0, 1,0, 2,0};
//	static Unit UNITS[] = {
//		{UNITS_MEMBERS, ARRAYSIZE(UNITS_MEMBERS), {0,0}},
//	};
//	gSim.problem.units = UNITS;
//	gSim.problem.numUnits = ARRAYSIZE(UNITS);
//	//static Cell FILLED[] = {};
//	gSim.problem.filled = NULL; //FILLED;
//	gSim.problem.numFilled = 0; //ARRAYSIZE(FILLED);
//	gSim.problem.sourceLength = 100;
//}

//struct Axial
//{
//	int x;
//	int y;
//	int x;
//};

//typedef Vec3I Axial;

//# convert cube to odd-r offset
//	col = x + (z - (z&1)) / 2
//	row = z
//
//# convert odd-r offset to cube
//	x = col - (row - (row&1)) / 2
//	z = row
//	y = -x-z
//
Axial AxialRotateCCW_60(Axial axial)
{
	Axial retval;
	retval.x = -axial.z;
	retval.y = -axial.x;
	retval.z = -axial.y;
	return retval;
}

int GetUnitPositions_OldSlow(int unitIndex, const UnitPose &unitPose, Cell positions[], Cell &pivot)
{
	const Unit &unitDef = UnitGet(unitIndex);
	pivot = AxialToCell(CellToAxial(unitDef.pivot) + CellToAxial(unitPose.position));
	for(int i = 0; i < unitDef.numMembers; ++i)
	{
		Cell rotatedPos = CellTransform(unitPose, unitDef, i);
		positions[i] = rotatedPos;
	}

	return unitDef.numMembers;
}

static void CacheCellTransforms(int unitIndex, Unit& unitDef)
{
	int cacheOffset = 0;
	Axial apivot = CellToAxial(unitDef.pivot);
	Axial adiff[MAX_UNIT_MEMBERS];

	for(int iMember = 0; iMember < unitDef.numMembers; ++iMember)
	{
		Axial acell = CellToAxial(unitDef.members[iMember]);
		adiff[iMember] = acell - apivot;
	}

	for(int iRotation = 0; iRotation < NUM_ORIENTATIONS; ++iRotation)
	{
		// allocate space
		unitDef.offsetCache[iRotation] = &unitDef.offsetCacheData[cacheOffset];
		cacheOffset += unitDef.numMembers;

		for(int iMember = 0; iMember < unitDef.numMembers; ++iMember)
		{
			// calculate & cache data
			Axial aresult = apivot + adiff[iMember];
			unitDef.offsetCache[iRotation][iMember] = aresult;

			// rotate for next iteration of rotation
			adiff[iMember] = AxialRotateCCW_60(adiff[iMember]);
		}
	}

	for(int iRotation = 0; iRotation < NUM_ORIENTATIONS; ++iRotation)
	{
		for(int iParity = 0; iParity < NUM_PARITIES; ++iParity)
		{
			UnitPose pose;
			pose.position.x = 0;
			pose.position.y = iParity;
			pose.orientation = iRotation;

			int resultSize = GetUnitPositions_OldSlow(unitIndex, pose, unitDef.cellCache[iRotation][iParity], unitDef.pivotCache[iRotation][iParity]);
			XAssert(unitDef.numMembers == resultSize);

			for(int i = 0; i < unitDef.numMembers; ++i)
			{
				unitDef.cellCache[iRotation][iParity][i] = unitDef.cellCache[iRotation][iParity][i] - pose.position;
			}

			unitDef.pivotCache[iRotation][iParity] = unitDef.pivotCache[iRotation][iParity] - pose.position;
		}
	}
}

Cell GetUnitFirstPosition(int unitIndex, const UnitPose &unitPose)
{
	const Unit &unitDef = UnitGet(unitIndex);
	XAssert(unitDef.numMembers > 0);
	
	int parity = (int)!IsEven(unitPose.position.y);
	return unitPose.position + unitDef.cellCache[unitPose.orientation][parity][0];
	//return CellTransform(unitPose, unitDef, 0);
}

//typedef void (*UnitPosFn)(const UnitPose &pose, const Cell &position, const Cell &pivot);
//
//void MapOverUnitPositions(int unitIndex, const UnitPose &unitPose, UnitPosFn fn)
//{
//	const Unit &unitDef = UnitGet(unitIndex);
//
//	Cell pivot = AxialToCell(CellToAxial(unitDef.pivot) + origin);
//	for(int i = 0; i < unitDef.numMembers; ++i)
//	{
//		Cell rotatedPos = CellTransform(unitPose, unitDef, i);
//		fn(unitPose, rotatedPos, pivot);
//	}
//}

int GetUnitPositions(int unitIndex, const UnitPose &unitPose, Cell positions[], Cell &pivot)
{
	const Unit &unitDef = UnitGet(unitIndex);
	int parity = (int)!IsEven(unitPose.position.y);

	for(int i = 0; i < unitDef.numMembers; ++i)
	{
		positions[i] = unitPose.position + unitDef.cellCache[unitPose.orientation][parity][i];
	}
		
	pivot = unitPose.position + unitDef.pivotCache[unitPose.orientation][parity];

	return unitDef.numMembers;
}

int GetUnitPositionsWithBounds(int unitIndex, const UnitPose &unitPose, Cell positions[], Cell &pivot, Bounds& bounds)
{
	const Unit &unitDef = UnitGet(unitIndex);
	pivot = AxialToCell(CellToAxial(unitDef.pivot) + CellToAxial(unitPose.position));
    
	for(int i = 0; i < unitDef.numMembers; ++i)
	{
		Cell rotatedPos = CellTransform(unitPose, unitDef, i);
		positions[i] = rotatedPos;

        if (i != 0)
        {
            BoundsInflate(bounds, rotatedPos);
        }
        else
        {
            bounds.p0 = rotatedPos;
            bounds.p1 = rotatedPos;
        }
	}

	return unitDef.numMembers;
}

bool CheckRowClear(int row)
{
	for(int i = 0; i < gSim.problem.width; ++i)
	{
		if (gSim.board.cells[i][row].state != CELL_FILLED) return false;
	}

	return true;
}

void ClearRow(int row)
{
    // Update cached info
    for (int j = row; j >= 1; --j)
    {
        gSim.board.numEmptyCellsPerRow[j] = gSim.board.numEmptyCellsPerRow[j - 1];
    }
    gSim.board.numEmptyCellsPerRow[0] = gSim.problem.width;

    // Now do the clear
    for(int i = 0; i < gSim.problem.width; ++i)
	{
		for(int j = row; j >= 1; --j)
		{
			gSim.board.cells[i][j].state = gSim.board.cells[i][j-1].state;
            //gSim.board.cells[i][j].unitNum= gSim.board.cells[i][j - 1].unitNum;
		}

		gSim.board.cells[i][0].state = CELL_EMPTY;
	}
}

void LockCurrentUnit()
{
	Cell positions[MAX_UNIT_MEMBERS];
	Cell pivot;
	int numMembers = GetUnitPositions(gSim.board.currentUnitIndex, gSim.board.currentUnitPose, positions, pivot);

	XAssert(numMembers > 0);

	Bounds bounds;
	bounds.p0 = positions[0];
	bounds.p1 = positions[0];

	for(int i = 0; i < numMembers; ++i)
	{
        gSim.board.numEmptyCellsPerRow[positions[i].y] -= 1;

		gSim.board.cells[positions[i].x][positions[i].y].state = CELL_FILLED;
        //gSim.board.cells[positions[i].x][positions[i].y].unitNum = gSim.board.unitsSpawned;
		BoundsInflate(bounds, positions[i]);
	}

	int rowsCleared = 0;
	for(int i = bounds.p0.y; i <= bounds.p1.y; ++i)
	{
		if (CheckRowClear(i))
		{
			ClearRow(i);
			rowsCleared++;
		}
	}

	gSim.board.last_TileScore = CalcScore(UnitGet(gSim.board.currentUnitIndex).numMembers, rowsCleared, gSim.board.linesClearedPrev);
	gSim.board.totalScoreUnits += gSim.board.last_TileScore;
	gSim.board.linesClearedPrev = rowsCleared;

	gSim.board.lastWordScore = gSim.board.currentWordScore;
	gSim.board.currentWordScore = 0;
	gSim.board.lastWordCount = gSim.board.currentWordCount;
	gSim.board.currentWordCount = 0;
}

void HoneyMarkPoseAsUsed(const UnitPose &unitPose)
{
	int bits = 1 << unitPose.orientation;

	Cell position = GetUnitFirstPosition(gSim.board.currentUnitIndex, unitPose);

	int& currentBits = gSim.board.usedPositions[position.x][position.y];

	XAssert((currentBits & bits) == 0);

	currentBits |= bits;
}

void HoneyUnmarkPoseAsUsed(const UnitPose &unitPose)
{
	int bits = 1 << unitPose.orientation;

	Cell position = GetUnitFirstPosition(gSim.board.currentUnitIndex, unitPose);

	int& currentBits = gSim.board.usedPositions[position.x][position.y];

	XAssert((currentBits & bits) != 0);

	currentBits &= ~bits;
}

void LoadNextPiece()
{
	if (gSim.board.unitsSpawned == gSim.problem.sourceLength)
	{
		gSim.gameState = GAME_OVER;
	}

	bool loadUnitResult = BoardLoadUnit(gSim.board);
	if (loadUnitResult == false)
	{
		gSim.gameState = GAME_OVER;
	}
}

struct CommandSet
{
	Move move;
	const char *commands;
};

const CommandSet COMMANDS[] = 
{
	{MOVE_W, "p'!.03"},
	{MOVE_E, "bcefy2"},
	{MOVE_SW, "aghij4"},
	{MOVE_SE, "lmno 5"},
	{TURN_CW, "dqrvz1"},
	{TURN_CCW, "kstuwx"},
};

Move HoneyCommandToMove(char command)
{
	for (int i = 0; i < NUM_MOVE_TYPES; i++)
	{
		const CommandSet &set = COMMANDS[i];
		if (strchr(set.commands, tolower(command)) != NULL)
		{
			return set.move;
		}
	}

	XHalt();
}

char MoveToCommand(Move move)
{
	const CommandSet &set = COMMANDS[move];
	XAssert(set.move == move);
	return set.commands[0];
}

void AddMoveToCommand(char command)
{
	Solution &currentSolution = gSim.ouput.solutions[0];
	XAssert(currentSolution.numCommands >= 0 && currentSolution.numCommands < MAX_COMMANDS);
	currentSolution.pcommandBuffer[currentSolution.numCommands++] = command;
	//currentSolution.scoreWords += MoveScoreWords();
	int wordCount = 0;
	int wordScoredIndex = -1;
	int wordScore = MoveScoreWords(wordCount, wordScoredIndex);
	//if (wordScoredIndex != -1)
	//{
		gSim.board.currentWordIndex = wordScoredIndex;
	//}
	gSim.board.currentWordCount += wordCount;
	gSim.board.currentWordScore += wordScore;
	gSim.board.totalScoreWords += wordScore;
}

MoveResult HoneyMoveCommand(Move move, char command)
{
	//XAssert(gSim.gameState == GAME_PLAYING);
	if (gSim.gameState != GAME_PLAYING) return MOVERESULT_INVALID;

	AddMoveToCommand(command);

	UnitPose newPose = PoseMove(gSim.board.currentUnitPose, move);

	//XTrace("  move %d to %d,%d / %d", move, newPose.position.x, newPose.position.y, newPose.orientation);

	MoveResult result = ValidatePose(newPose);

	if (result == MOVERESULT_GAME_OVER)
	{
		LockCurrentUnit();
		LoadNextPiece();
	}
	else if (result == MOVERESULT_OK)
	{
		HoneyMarkPoseAsUsed(newPose);
		gSim.board.currentUnitPose = newPose;
	}
	else if (result == MOVERESULT_INVALID)
	{
		// just don't allow for now
	}
	else
	{
		XHalt();
	}

	return result;
}

MoveResult HoneyMove(Move move)
{
	return HoneyMoveCommand(move, MoveToCommand(move));
}

ParentPointer& GetParentPointerFromPose(UnitPose unitPose)
{
	Cell position = GetUnitFirstPosition(gSim.board.currentUnitIndex, unitPose);
	XAssert(position.x >= 0 && position.x < gSim.problem.width && position.y >= 0 && position.y < gSim.problem.height);
	return gSim.board.finalPositionsParents[position.x][position.y][unitPose.orientation];
}

void HoneyEnumerateFinalPositionsRecursive(UnitPose floodUnitPose)
{
	bool isGameOverPos = false;

	for (int i = 0; i < NUM_MOVE_TYPES; i++)
	{
		Move MOVE_ARRAY[NUM_MOVE_TYPES] = { MOVE_SW, MOVE_SE, MOVE_W, MOVE_E, TURN_CW, TURN_CCW };
		Move move = MOVE_ARRAY[i];
		//Move move = (Move)i;

		UnitPose newPose = PoseMove(floodUnitPose, move);

		MoveResult result = ValidatePose(newPose);

		if (result == MOVERESULT_OK)
		{
			ParentPointer& parentPointer = GetParentPointerFromPose(newPose);
			XAssert(parentPointer.moveToHere == MOVE_INVALID);
			parentPointer.poseParent = floodUnitPose;
			parentPointer.moveToHere = move;

			HoneyMarkPoseAsUsed(newPose);
			HoneyEnumerateFinalPositionsRecursive(newPose);
		}
		else if (result == MOVERESULT_GAME_OVER)
		{
			if (!isGameOverPos)
			{
				isGameOverPos = true;
				XAssert(gSim.board.numFinalPositions < MAX_FINAL_POSITIONS);
				//XTrace("%d %d %d", floodUnitPose.position.x, floodUnitPose.position.y, floodUnitPose.orientation);
				gSim.board.finalPositionList[gSim.board.numFinalPositions].poseFinal = floodUnitPose;
				gSim.board.finalPositionList[gSim.board.numFinalPositions].moveToEnd = move;
				gSim.board.finalPositionList[gSim.board.numFinalPositions].commandToEnd = MoveToCommand(move);
                gSim.board.finalPositionList[gSim.board.numFinalPositions].powerScore = 0;
				gSim.board.numFinalPositions++;
			}

			// keep checking for other ok moves
		}
		else if (result == MOVERESULT_INVALID)
		{
		}
		else
		{
			XHalt();
		}
	}
}

void HoneyEnumerateFinalPositions()
{
	if (gSim.gameState != GAME_PLAYING) return;

	if (USE_FINAL_POSITIONS_WITH_SCORE)
	{
		// todo: we could store the powerstep from the previous board move
		HoneyEnumerateFinalPositionsRecursiveWithScore(gSim.board.currentUnitPose, gSim.board.powerStepUsedBitField, POWERSTEP_INVALID, 0);
	}
	else
	{
		HoneyEnumerateFinalPositionsRecursive(gSim.board.currentUnitPose);
	}

	//XTrace("final positions %d", gSim.board.numFinalPositions);
}

Move PowerStepToMove(PowerStep powerStep)
{
	XAssert(powerStep >= POWERSTEP_MOVE_W && powerStep <= POWERSTEP_TURN_CCW);
	return (Move)(powerStep - POWERSTEP_MOVE_W);
}

MoveResult ValidatePowerStep(UnitPose startPose, PowerStep powerStep, UnitPose& stepEndPose)
{
	UnitPose curPose = startPose;
	stepEndPose = curPose;

	int stepsToClear = 0;
	MoveResult result = MOVERESULT_INVALID;

	XAssert(powerStep >= 0 && powerStep < NUM_POWERSTEPS);

	for (int i = 0; i < gSim.powerPhrases[powerStep].len; i++)
	{
		Move move = gSim.powerPhraseMoves[powerStep][i];

		curPose = PoseMove(curPose, move);

		result = ValidatePose(curPose);

		if (result == MOVERESULT_OK)
		{
			stepsToClear++;
			HoneyMarkPoseAsUsed(curPose);
			stepEndPose = curPose;
			continue;
		}
		else if (result == MOVERESULT_GAME_OVER)
		{
			if (i == gSim.powerPhrases[powerStep].len - 1)
			{
				// note stepEndPose is the last pose on the board
				break;
			}
			else
			{
				// has moves after game over
				result = MOVERESULT_INVALID;
				break;
			}
		}
		else if (result == MOVERESULT_INVALID)
		{
			break;
		}
		else
		{
			XHalt();
		}
	}

	curPose = startPose;

	for (int i = 0; i < stepsToClear; i++)
	{
		Move move = gSim.powerPhraseMoves[powerStep][i];
		curPose = PoseMove(curPose, move);
		HoneyUnmarkPoseAsUsed(curPose);
	}

	return result;
}
/*
int ScoreFromPowerStep(PowerStep powerStep, int powerStepUsedBitField)
{
	int score = gSim.powerStepLenScore[powerStep];

	if (powerStep < POWERSTEP_MOVE_W)
	{
		int bit = 1 << powerStep;
		if ((bit & powerStepUsedBitField) == 0)
		{
			score += 300;
		}
	}

	return score;
}

void HoneyMarkPowerStep(ParentPointer& parentPointer)
{
	UnitPose curPose = parentPointer.poseParent;
	PowerStep powerStep = parentPointer.powerStepToHere;

	XAssert(powerStep >= 0 && powerStep < NUM_POWERSTEPS);

	for (int i = 0; i < gSim.powerPhrases[powerStep].len; i++)
	{
		Move move = gSim.powerPhraseMoves[powerStep][i];

		curPose = PoseMove(curPose, move);

		XAssert(ValidatePose(curPose) == MOVERESULT_OK);

		HoneyMarkPoseAsUsed(curPose);
	}
}

void HoneyUnmarkPowerStep(ParentPointer& parentPointer)
{
	UnitPose curPose = parentPointer.poseParent;
	PowerStep powerStep = parentPointer.powerStepToHere;

	XAssert(powerStep >= 0 && powerStep < NUM_POWERSTEPS);

	for (int i = 0; i < gSim.powerPhrases[powerStep].len; i++)
	{
		Move move = gSim.powerPhraseMoves[powerStep][i];

		curPose = PoseMove(curPose, move);

		HoneyUnmarkPoseAsUsed(curPose);
	}
}
*/
void PowerStepMark(UnitPose startPose, PowerStep powerStep, int curScore)
{
	UnitPose curPose = startPose;

	XAssert(powerStep >= 0 && powerStep < NUM_POWERSTEPS);

	for (int i = 0; i < gSim.powerPhrases[powerStep].len; i++)
	{
		Move move = gSim.powerPhraseMoves[powerStep][i];
		char command = gSim.powerPhrases[powerStep].chars[i];

		UnitPose prevPose = curPose;
		curPose = PoseMove(curPose, move);

		MoveResult result = ValidatePose(curPose);

		if (result == MOVERESULT_OK)
		{
			ParentPointer& parentPointer = GetParentPointerFromPose(curPose);

			XAssert(parentPointer.moveToHere == MOVE_INVALID);

			parentPointer.poseParent = prevPose;
			parentPointer.moveToHere = move;
			parentPointer.commandToHere = command;

			HoneyMarkPoseAsUsed(curPose);
		}
		else if (result == MOVERESULT_GAME_OVER)
		{
			XAssert(gSim.board.numFinalPositions < MAX_FINAL_POSITIONS);
			//XTrace("%d %d %d", floodUnitPose.position.x, floodUnitPose.position.y, floodUnitPose.orientation);
			gSim.board.finalPositionList[gSim.board.numFinalPositions].poseFinal = prevPose;
			gSim.board.finalPositionList[gSim.board.numFinalPositions].moveToEnd = move;
			gSim.board.finalPositionList[gSim.board.numFinalPositions].commandToEnd = command;
            gSim.board.finalPositionList[gSim.board.numFinalPositions].powerScore = curScore;
			gSim.board.numFinalPositions++;
		}
		else
		{
			XHalt();
		}
	}
}

void PowerStepWalkFrom(UnitPose startPose, PowerStep powerStep, int stepIndex, int powerStepUsedBitField, PowerStep previousPowerStep, int curScore)
{
	UnitPose curPose = startPose;

	for (int i = 0; i < stepIndex; i++)
	{
		Move move = gSim.powerPhraseMoves[powerStep][i];
		curPose = PoseMove(curPose, move);
	}

	HoneyEnumerateFinalPositionsRecursiveWithScore(curPose, powerStepUsedBitField, previousPowerStep, curScore);
}

void PowerStepWalk(UnitPose startPose, PowerStep powerStep, int powerStepUsedBitField, PowerStep previousPowerStep, int curScore)
{
	// walk up tree (skipping tail of power step)
	for (int i = gSim.powerPhrases[powerStep].len - 1; i >= 1; i--)
	{
		PowerStepWalkFrom(startPose, powerStep, i, powerStepUsedBitField, previousPowerStep, curScore);
	}
}

void HoneyEnumerateFinalPositionsRecursiveWithScore(UnitPose floodUnitPose, int powerStepUsedBitField, PowerStep previousPowerStep, int curScore)
{
	int start = POWERSTEP_MOVE_W - gSim.numPowerPhrases;
	XAssert(start >= 0);
	
	PowerStep powerStepArray[NUM_POWERSTEPS];
	int powerStepArraySize = 0;

	// Load unused words
	for (int i = 0; i < gSim.numPowerPhrases; i++)
	{
		PowerStep powerStep = (PowerStep)gSim.sortedPowerPhrasesForUnused[i];
		XAssert(powerStep >= 0 && powerStep < NUM_POWERSTEPS);

		int bit = 1 << powerStep;
		if ((powerStepUsedBitField & bit) != 0)
		{
			continue;
		}

		PowerWord& powerPhrase = gSim.powerPhrases[powerStep];
		XAssert(powerPhrase.len > 0);

		powerStepArray[powerStepArraySize] = powerStep;
		powerStepArraySize++;
	}

	/*
	// Load prefix words
	if (previousPowerStep != POWERSTEP_INVALID)
	{
		PowerStep powerStepPartial;
		if (previousPowerStep <= POWERSTEP_PARTIAL_WORD_0)
		{
			powerStepPartial = previousPowerStep;
		}
		else
		{
			powerStepPartial = (PowerStep)(previousPowerStep - kMaxPowerPhrases);
		}
		
		PowerWord& powerPhrasePartial = gSim.powerPhrases[powerStepPartial];

		if (powerPhrasePartial.len > 0)
		{
			powerStepArray[powerStepArraySize] = powerStepPartial;
			powerStepArraySize++;
		}
	}
	*/

	// Load used words
	for (int i = 0; i < gSim.numPowerPhrases; i++)
	{
		PowerStep powerStep = (PowerStep)gSim.sortedPowerPhrasesForUnused[i];
		XAssert(powerStep >= 0 && powerStep < NUM_POWERSTEPS);

		int bit = 1 << powerStep;
		if ((powerStepUsedBitField & bit) == 0)
		{
			continue;
		}

		PowerWord& powerPhrase = gSim.powerPhrases[powerStep];
		XAssert(powerPhrase.len > 0);

		powerStepArray[powerStepArraySize] = powerStep;
		powerStepArraySize++;
	}

	// Load simple steps
	powerStepArray[powerStepArraySize] = POWERSTEP_MOVE_SW;
	powerStepArraySize++;
	powerStepArray[powerStepArraySize] = POWERSTEP_MOVE_SE;
	powerStepArraySize++;
	powerStepArray[powerStepArraySize] = POWERSTEP_MOVE_W;
	powerStepArraySize++;
	powerStepArray[powerStepArraySize] = POWERSTEP_MOVE_E;
	powerStepArraySize++;
	powerStepArray[powerStepArraySize] = POWERSTEP_TURN_CW;
	powerStepArraySize++;
	powerStepArray[powerStepArraySize] = POWERSTEP_TURN_CCW;
	powerStepArraySize++;

	XAssert(powerStepArraySize <= NUM_POWERSTEPS);

	for (int i = 0; i < powerStepArraySize; i++)
	{
		PowerStep powerStep = powerStepArray[i];

		UnitPose stepEndPose;
		MoveResult result = ValidatePowerStep(floodUnitPose, powerStep, stepEndPose);

		if (result == MOVERESULT_INVALID)
		{
			continue;
		}

        // Calculate the power phrase score similar to MoveScoreWords but faster. It's an approximation since we call this code a lot. We won't capture power phrases that cross unit moves
        int powerStepScore = 0;
        if (powerStep <= POWERSTEP_WORD_0)
        {
            powerStepScore += gSim.powerPhrases[powerStep].len * 2;
            if ((powerStepUsedBitField & (1 << powerStep)) == false)
            {
                powerStepScore += 300;
            }
        }

        int newScore = curScore + powerStepScore;

		PowerStepMark(floodUnitPose, powerStep, newScore);

		if (result == MOVERESULT_OK)
		{
			int powerStepUsedBitFieldNow = powerStepUsedBitField | (1 << powerStep);

			HoneyEnumerateFinalPositionsRecursiveWithScore(stepEndPose, powerStepUsedBitFieldNow, powerStep, newScore );
		}
		else if (result == MOVERESULT_GAME_OVER)
		{
			//XAssert(gSim.board.numFinalPositions < MAX_FINAL_POSITIONS);
			////XTrace("%d %d %d", floodUnitPose.position.x, floodUnitPose.position.y, floodUnitPose.orientation);
			//gSim.board.finalPositionList[gSim.board.numFinalPositions].poseFinal = prevPose;
			//gSim.board.finalPositionList[gSim.board.numFinalPositions].moveToEnd = move;
			//gSim.board.numFinalPositions++;
		}
		else
		{
			XHalt();
		}

		PowerStepWalk(floodUnitPose, powerStep, powerStepUsedBitField, previousPowerStep, newScore);
	}
}

void WalkMoveListRecursive(UnitPose unitPose)
{
	Cell position = GetUnitFirstPosition(gSim.board.currentUnitIndex, unitPose);
	XAssert(position.x >= 0 && position.x < gSim.problem.width && position.y >= 0 && position.y < gSim.problem.height);
	ParentPointer& parentPointer = gSim.board.finalPositionsParents[position.x][position.y][unitPose.orientation];

	if (parentPointer.moveToHere != MOVE_INVALID)
	{
		WalkMoveListRecursive(parentPointer.poseParent);

		AddMoveToCommand(parentPointer.commandToHere);
	}
}

void HoneyChooseEnumerateMove(int index)
{
	XAssert(index >= 0 && index < gSim.board.numFinalPositions);
	FinalMove& finalMove = gSim.board.finalPositionList[index];

	WalkMoveListRecursive(finalMove.poseFinal);

	gSim.board.currentUnitPose = finalMove.poseFinal;
	MoveResult result = HoneyMoveCommand(finalMove.moveToEnd, finalMove.commandToEnd);
	XAssert(result == MOVERESULT_GAME_OVER);
}

void InitOutput(int numSolutions)
{
	memset(&gSim.ouput, 0x00, sizeof(gSim.ouput));
	gSim.ouput.numSolutions = numSolutions;
	for (int i = 0; i < numSolutions; ++i)
	{
		gSim.ouput.solutions[i].pcommandBuffer = (char *)XMalloc(1024 * 1024);
	}
}

void ClearOutput(int outputIndex)
{
	XAssert(outputIndex >= 0 && outputIndex < gSim.ouput.numSolutions);
	gSim.ouput.solutions[outputIndex].numCommands = 0;
}

//
void DisposeOutput()
{
	for (int i = 0; i < gSim.ouput.numSolutions; ++i)
	{
		XFree(gSim.ouput.solutions[i].pcommandBuffer);
		gSim.ouput.solutions[i].pcommandBuffer = NULL;
	}

	gSim.ouput.numSolutions = 0;
}

void CopyOutput(int source, int target)
{
	XAssert(source >= 0 && source < gSim.ouput.numSolutions);
	XAssert(target >= 0 && target < gSim.ouput.numSolutions);
	if(source == target)
		return;

	gSim.ouput.solutions[target].numCommands = gSim.ouput.solutions[source].numCommands;
	gSim.ouput.solutions[target].problemId = gSim.ouput.solutions[source].problemId;
	//gSim.ouput.solutions[target].score = gSim.ouput.solutions[source].score;
	gSim.ouput.solutions[target].seed = gSim.ouput.solutions[source].seed;
	//gSim.ouput.solutions[target].tag = gSim.ouput.solutions[source].tag;

	// copy command buffer
	memcpy(gSim.ouput.solutions[target].pcommandBuffer, gSim.ouput.solutions[source].pcommandBuffer, gSim.ouput.solutions[target].numCommands);
}

void ClearPowerStepUsedBitField()
{
    gSim.board.powerStepUsedBitField = 0;
}

void HoneyStartGameSeed(int seedIndex)
{
	ClearOutput(0);
    ClearPowerStepUsedBitField();
	BoardLoad(gSim.board, seedIndex);
}

void HoneyStartGame(bool resetSeed)
{
	if ((gSim.gameIndex >= gSim.problem.numSourceSeeds) || resetSeed) 
	{
		gSim.gameIndex = 0;
	}

	HoneyStartGameSeed(gSim.gameIndex++);

}

bool HasFoundWord(int wordIndex)
{
	if (gSim.board.powerStepUsedBitField  & 1 << wordIndex) return true;
	return false;
}

void MarkHasFoundWord(int wordIndex)
{
	gSim.board.powerStepUsedBitField |= 1 << wordIndex;
}

bool ReverseCompare(const PowerWord& powerWord, const char* outputBuffer, int outputBufferLen)
{
	if (powerWord.len == 0) return false;
	if (outputBufferLen < powerWord.len) return false;
	for(int i = 0; i < powerWord.len; ++i)
	{
		int wordCharIndex = powerWord.len - i - 1;
		int bufferCharIndex = outputBufferLen - i - 1;

		if (powerWord.chars[wordCharIndex] == outputBuffer[bufferCharIndex] == false) return false;
	}

	return true;
}

int MoveScoreWords(int& wordCount, int &wordScoredIndex)
{
	wordScoredIndex = -1;
	//int score = 0;
	//for(int i = 0; i < numWords; ++i)
	//{
	//	if (revcompare(word, worldlen, bufferstart, bufferend) == true)
	//	{
	//		if (hasEverFoundWord(i) == false)
	//		{
	//			bonus = 300;
	//			score += bonus;
	//			hasFoundWord(i);
	//		}

	//		score += scoreword()
	//	}
	//}

	//return score;

	//Solution &currentSolution = gSim.ouput.solutions[0];
	int solutionIndex = 0;  // always zero now
	const char* outputBufferStart = gSim.ouput.solutions[solutionIndex].pcommandBuffer;
	int outputBufferLen = gSim.ouput.solutions[solutionIndex].numCommands;

	wordCount = 0;
	int score = 0;
	//for(int i = 0; i < gSim.numPowerPhrases; ++i)
	int start = POWERSTEP_MOVE_W - gSim.numPowerPhrases;
	XAssert(start >= 0);

	for (int i = start; i <= POWERSTEP_WORD_0; i++)
	{
		const PowerWord &word = gSim.powerPhrases[i];
		XAssert(word.len > 1);
		if (ReverseCompare(word, outputBufferStart, outputBufferLen))
		{
			if (HasFoundWord(i) == false)
			{
				const int WORD_BONUS = 300;
				score += WORD_BONUS;
				MarkHasFoundWord(i);
			}

			score += 2 * word.len;
			wordCount++;
			wordScoredIndex = i;
		}
	}

	return score;
}

int CalcScore(int unitSize, int linesCleared, int linesClearedPrev)
{
	int points = unitSize + 100 * (linesCleared + 1) * linesCleared / 2;

	int lineBonus = 0;
	if (linesClearedPrev > 1)
	{
		lineBonus = (linesClearedPrev - 1) * points / 10;
	}

	return points + lineBonus;
}

bool HasPosition(const Cell& test, int numPositions, Cell* positions)
{
	for (int i = 0; i < numPositions; i++)
	{
		if (test == positions[i])
		{
			return true;
		}
	}

	return false;
}

bool PositionsMatch(int numPositions, Cell* positionsA, Cell* positionsB)
{
	for (int i = 0; i < numPositions; i++)
	{
		if (!HasPosition(positionsA[i], numPositions, positionsB))
		{
			return false;
		}
	}

	return true;
}

int CountDownCommands(PowerWord& powerPhrase)
{
	int down = 0;
	for (int i = 0; i < powerPhrase.len; i++)
	{
		Move move = HoneyCommandToMove(powerPhrase.chars[i]);
		if (move == MOVE_SW || move == MOVE_SE)
		{
			down++;
		}
	}
	return down;
}

bool SortPowerPhrasesUsedFunc(int index1, int index2)
{
	XAssert(index1 >= 0 && index1 < NUM_POWERSTEPS);
	XAssert(index2 >= 0 && index2 < NUM_POWERSTEPS);
	PowerWord& powerPhrase1 = gSim.powerPhrases[index1];
	PowerWord& powerPhrase2 = gSim.powerPhrases[index2];

	XAssert(powerPhrase1.len > 0);
	XAssert(powerPhrase2.len > 0);

	int countDowns1 = CountDownCommands(powerPhrase1);
	int countDowns2 = CountDownCommands(powerPhrase2);

	float score1;
	float score2;

	if (countDowns1 == 0)
	{
		// no downs is really good
		score1 = 10000.0f - (float)powerPhrase1.len;
	}
	else
	{
		score1 = (float)powerPhrase1.len / (float)countDowns1;
	}

	if (countDowns2 == 0)
	{
		// no downs is really good
		score2 = 10000.0f - (float)powerPhrase2.len;
	}
	else
	{
		score2 = (float)powerPhrase2.len / (float)countDowns2;
	}

	if (score1 < score2)
	{
		return true;
	}
	else if (score2 < score1)
	{
		return false;
	}
	else
	{
		// tie breaker
		return index1 < index2;
	}
}

bool SortPowerPhrasesUnusedFunc(int index1, int index2)
{
	XAssert(index1 >= 0 && index1 < NUM_POWERSTEPS);
	XAssert(index2 >= 0 && index2 < NUM_POWERSTEPS);
	PowerWord& powerPhrase1 = gSim.powerPhrases[index1];
	PowerWord& powerPhrase2 = gSim.powerPhrases[index2];

	XAssert(powerPhrase1.len > 0);
	XAssert(powerPhrase2.len > 0);

	// longer first
	if (powerPhrase1.len > powerPhrase2.len)
	{
		return true;
	}
	else if (powerPhrase1.len < powerPhrase2.len)
	{
		return false;
	}
	else
	{
		// tie breaker
		return index1 < index2;
	}
}

void InitHoneySimSystems()
{
    HoneyProcessPowerWords();
}

void HoneyProcessPowerWords()
{
	int start = POWERSTEP_MOVE_W - gSim.numPowerPhrases;
	XAssert(start >= 0);

	int sortIndex = 0;
	for (int i = start; i < NUM_POWERSTEPS; i++)
	{
		PowerWord& powerPhrase = gSim.powerPhrases[i];

		if (i < POWERSTEP_MOVE_W)
		{
			gSim.sortedPowerPhrasesForUsed[sortIndex] = i;
			gSim.sortedPowerPhrasesForUnused[sortIndex] = i;
			sortIndex++;

			powerPhrase.len = strlen(powerPhrase.chars);

			XAssert(powerPhrase.len <= MAX_POWER_WORD_LEN);

			//gSim.powerStepLenScore[i] = 2 * powerPhrase.len;

			for (int charindex = 0; charindex < powerPhrase.len; charindex++)
			{
				gSim.powerPhraseMoves[i][charindex] = HoneyCommandToMove(powerPhrase.chars[charindex]);
			}

			if (_stricmp(powerPhrase.chars, "ia! ia!") == 0)
			{
				PowerStep powerStepPartial = (PowerStep)(i - kMaxPowerPhrases);
				strcpy(gSim.powerPhrases[powerStepPartial].chars, " ia!");
				gSim.powerPhrases[powerStepPartial].len = strlen(gSim.powerPhrases[powerStepPartial].chars);
				XAssert(powerPhrase.len <= MAX_POWER_WORD_LEN);

				for (int charindex = 0; charindex < powerPhrase.len; charindex++)
				{
					gSim.powerPhraseMoves[powerStepPartial][charindex] = HoneyCommandToMove(powerPhrase.chars[charindex]);
				}
			}
		}
		else
		{
			Move move = (Move)(i - POWERSTEP_MOVE_W);
			//gSim.powerStepLenScore[i] = 0;
			gSim.powerPhraseMoves[i][0] = move;
			powerPhrase.len = 1;
			powerPhrase.chars[1] = 0;

			switch (move)
			{
			case MOVE_W: powerPhrase.chars[0] = 'p'; break;
			case MOVE_E: powerPhrase.chars[0] = 'b'; break;
			case MOVE_SW: powerPhrase.chars[0] = 'a'; break;
			case MOVE_SE: powerPhrase.chars[0] = 'l'; break;
			case TURN_CW: powerPhrase.chars[0] = 'd'; break;
			case TURN_CCW: powerPhrase.chars[0] = 'k'; break;
			default: XHalt();
			}
		}
	}

	XAssert(sortIndex == gSim.numPowerPhrases);

	std::sort(gSim.sortedPowerPhrasesForUsed, gSim.sortedPowerPhrasesForUsed + gSim.numPowerPhrases, SortPowerPhrasesUsedFunc);
	std::sort(gSim.sortedPowerPhrasesForUnused, gSim.sortedPowerPhrasesForUnused + gSim.numPowerPhrases, SortPowerPhrasesUnusedFunc);
}

static void CalcUnitSymmetry()
{
	const int numRotations = 4;
	UnitPose unitPose[numRotations];

	for (int rot = 0; rot < numRotations; rot++)
	{
		unitPose[rot].position.x = 0;
		unitPose[rot].position.y = 0;
		unitPose[rot].orientation = rot;
	}

	for (int i = 0; i < gSim.problem.numUnits; i++)
	{
		Cell positions[numRotations][MAX_UNIT_MEMBERS];
		Cell pivot[numRotations];
		int numPositions[numRotations];

		for (int rot = 0; rot < numRotations; rot++)
		{
			numPositions[rot] = GetUnitPositions(i, unitPose[rot], positions[rot], pivot[rot]);
		}

		XAssert(numPositions[0] == numPositions[1] && numPositions[0] == numPositions[2] && numPositions[0] == numPositions[3]);

		bool match1 = PositionsMatch(numPositions[0], positions[0], positions[1]);
		bool match2 = PositionsMatch(numPositions[0], positions[0], positions[2]);
		bool match3 = PositionsMatch(numPositions[0], positions[0], positions[3]);

		Unit& unit = gSim.problem.units[i];

		if (!match1 && !match2 && !match3)
		{
			unit.symmetry = SYMMETRY_NONE;
		}
		else if (match1 && match2 && match3)
		{
			unit.symmetry = SYMMETRY_1;
		}
		else if (!match1 && match2 && !match3)
		{
			unit.symmetry = SYMMETRY_2;
		}
		else if (!match1 && !match2 && match3)
		{
			unit.symmetry = SYMMETRY_3;
		}
		else
		{
			XHalt();
		}

	}
}

void HoneyCacheProblemInfo()
{
	for(int i = 0; i < gSim.problem.numUnits; ++i)
	{
		CacheCellTransforms(i, gSim.problem.units[i]);
	}

	CalcUnitSymmetry();
}

void DumpOutputInit(HoneyOutputInfo& info, int problemIndex)
{
	char filename[FILENAME_MAX];
	sprintf(filename, "output_%d.txt", problemIndex);

	FILE *f = fopen(filename, "wb");

	HoneyInitOutputInfo(info, f);
}

void DumpOutputFinish(HoneyOutputInfo& info)
{
	fclose(info.f);
}

void HoneyInitOutputInfo(HoneyOutputInfo& info, FILE*f)
{
	info.f = f;
	info.numOutputs = -1;
}

void HoneyOutputHeader(HoneyOutputInfo& info)
{
	fprintf(info.f, "[\n");
	info.numOutputs = 0;
}

void HoneyOutputFooter(HoneyOutputInfo& info)
{
	if(info.numOutputs > 0)
	{
		fprintf(info.f, "  }\n");
	}

	fprintf(info.f, "]\n");
}

void HoneyOutputSolutionExt(HoneyOutputInfo& info, int seedIndex, int outputIndex, const char* aiName, const char* outMethod, int score)
{
	time_t rawtime;
	struct tm * timeinfo;
	char bufferTime[256];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(bufferTime, 256,"%c",timeinfo);

	char tag[256];
	sprintf(tag, "%s %s %s score@@%d", aiName, outMethod, bufferTime, score);

	HoneyOutputSolution(info, seedIndex, outputIndex, tag);
}

void HoneyOutputSolution(HoneyOutputInfo& info, int seedIndex, int outputIndex, const char* tag)
{
	if(info.numOutputs > 0)
	{
		fprintf(info.f, "  },\n");
	}

	info.numOutputs++;

	fprintf(info.f, "  { \"problemId\":%d,\n", gSim.problem.id);
	fprintf(info.f, "    \"seed\":%d,\n", gSim.problem.sourceSeeds[seedIndex]);
	fprintf(info.f, "    \"tag\":\"%s\",\n", tag);
	fprintf(info.f, "    \"solution\":\"");

	for(int j = 0; j < gSim.ouput.solutions[outputIndex].numCommands; ++j)
	{
		fprintf(info.f, "%c", gSim.ouput.solutions[outputIndex].pcommandBuffer[j]);
	}

	fprintf(info.f, "\"\n");
}

void AppendPowerPhrase(const char *phrase)
{
	XAssert(gSim.numPowerPhrases < kMaxPowerPhrases);
	int index = gSim.numPowerPhrases++;
	PowerStep powerStep = (PowerStep)(POWERSTEP_WORD_0 - index);
	strcpy(gSim.powerPhrases[powerStep].chars, phrase);
	gSim.powerPhrases[powerStep].len = strlen(phrase);
	for(int i = 0; i < gSim.powerPhrases[powerStep].len; ++i)
	{
		gSim.powerPhrases[powerStep].chars[i] = (char)tolower(gSim.powerPhrases[powerStep].chars[i]);
	}
}

void AddKnownPowerPhrases()
{
	AppendPowerPhrase("Ei!");
	AppendPowerPhrase("Ia! Ia!");
	AppendPowerPhrase("R'lyeh");
	AppendPowerPhrase("Yuggoth");
	AppendPowerPhrase("Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn."); // call of Cthulhu, plus Yiddish hint from tweet
	
	AppendPowerPhrase("Tsathoggua"); // tweet hint regarding formless spawns
	AppendPowerPhrase("yogsothoth");

	//AppendPowerPhrase("''''''''''''Cthulhu");
	//AppendPowerPhrase("''''''''''''''''''Cthulhu");
	//AppendPowerPhrase("''''''Cthulhu");
	//
	//AppendPowerPhrase("hhh''''''Cthulhu");
	//AppendPowerPhrase("''uu''''Cthulhu");
	//AppendPowerPhrase("''h''''Cthulhu");
	//AppendPowerPhrase("uuuu''''''Cthulhu");
	//AppendPowerPhrase("iiiiii''''''Cthulhu");

	//AppendPowerPhrase("tttt'Cthhhhhulhu");
	//AppendPowerPhrase("hhhh'Cthulhu");
	//AppendPowerPhrase("ccccc'Cthulhu");


	//gSim.powerPhrases[POWERSTEP_WORD_5].chars = "ppp"; // testing no down
	//gSim.powerPhrases[POWERSTEP_WORD_6].chars = "bbb"; // testing no down

	// The ones below have been confirmed to be NOT VALID
	//gSim.powerPhrases[POWERSTEP_WORD_4].chars = "cthulhu"; // NOT VALID
	//gSim.powerPhrases[POWERSTEP_WORD_5].chars = "pentagram"; // NOT VALID, from board problem layout
	//gSim.powerPhrases[POWERSTEP_WORD_6].chars = "bellingham"; // from tweet, bordering Vancouver
	//gSim.powerPhrases[POWERSTEP_WORD_6].chars = "Eyjafjallajokull";
	//gSim.powerPhrases[POWERSTEP_WORD_5].chars = "vancouver"; // unconfirmed, city mentioned in Tweet, plus problem 24's board layout
	//Also these ones also rejected: sharo, 
}
