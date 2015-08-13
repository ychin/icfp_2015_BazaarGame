// HoneySim.h

#ifndef HONEYSIM_H
#define HONEYSIM_H

typedef Vec2I Cell;

const int MAX_UNIT_MEMBERS = 256;
const int MAX_WIDTH  = 512; 
const int MAX_HEIGHT = 512;
const int MAX_CELLS_PER_UNITS = 256;
const int MAX_CELLS_PER_BOARD = MAX_HEIGHT * MAX_WIDTH;
const int MAX_SOURCE_SEEDS = 256;
const int NUM_ORIENTATIONS = 6;
const int MAX_FINAL_POSITIONS = MAX_CELLS_PER_BOARD * 6;
const int NUM_PROBLEMS = 25;
const int MAX_POWER_WORD_LEN = 51;
const int kMaxPowerPhrases = 18;
const int NUM_PARITIES = 2;

enum Symmetry
{
	SYMMETRY_NONE,
	SYMMETRY_1, // any rotation
	SYMMETRY_2, // any double rotation
	SYMMETRY_3, // any triple rotation
};

enum PowerStep
{
	POWERSTEP_INVALID = -1,

	POWERSTEP_PARTIAL_WORD_17,
	POWERSTEP_PARTIAL_WORD_16,
	POWERSTEP_PARTIAL_WORD_15,
	POWERSTEP_PARTIAL_WORD_14,
	POWERSTEP_PARTIAL_WORD_13,
	POWERSTEP_PARTIAL_WORD_12,
	POWERSTEP_PARTIAL_WORD_11,
	POWERSTEP_PARTIAL_WORD_10,
	POWERSTEP_PARTIAL_WORD_9,
	POWERSTEP_PARTIAL_WORD_8,
	POWERSTEP_PARTIAL_WORD_7,
	POWERSTEP_PARTIAL_WORD_6,
	POWERSTEP_PARTIAL_WORD_5,
	POWERSTEP_PARTIAL_WORD_4,
	POWERSTEP_PARTIAL_WORD_3,
	POWERSTEP_PARTIAL_WORD_2,
	POWERSTEP_PARTIAL_WORD_1,
	POWERSTEP_PARTIAL_WORD_0,

	POWERSTEP_WORD_17,
	POWERSTEP_WORD_16,
	POWERSTEP_WORD_15,
	POWERSTEP_WORD_14,
	POWERSTEP_WORD_13,
	POWERSTEP_WORD_12,
	POWERSTEP_WORD_11,
	POWERSTEP_WORD_10,
	POWERSTEP_WORD_9,
	POWERSTEP_WORD_8,
	POWERSTEP_WORD_7,
	POWERSTEP_WORD_6,
	POWERSTEP_WORD_5,
	POWERSTEP_WORD_4,
	POWERSTEP_WORD_3,
	POWERSTEP_WORD_2,
	POWERSTEP_WORD_1,
	POWERSTEP_WORD_0,
	POWERSTEP_MOVE_W,
	POWERSTEP_MOVE_E,
	POWERSTEP_MOVE_SW,
	POWERSTEP_MOVE_SE,
	POWERSTEP_TURN_CW,
	POWERSTEP_TURN_CCW,

	NUM_POWERSTEPS,
};

struct Bounds
{
	Cell p0;
	Cell p1;
};

enum MoveResult
{
	MOVERESULT_OK,
	MOVERESULT_GAME_OVER,
	MOVERESULT_INVALID,
};

typedef Vec3I Axial;

struct Unit
{
	Cell members[MAX_CELLS_PER_UNITS];
	int numMembers;
	Cell pivot;
	Symmetry symmetry;

	// pre-computed cell offsets
	Axial *offsetCache[NUM_ORIENTATIONS];
	Axial offsetCacheData[NUM_ORIENTATIONS*MAX_CELLS_PER_UNITS];

	Cell cellCache[NUM_ORIENTATIONS][NUM_PARITIES][MAX_UNIT_MEMBERS];
	Cell pivotCache[NUM_ORIENTATIONS][NUM_PARITIES];
};

struct Problem
{
	int id;
	Unit units[MAX_UNIT_MEMBERS];
	int numUnits;
	int width;
	int height;
	Cell filled[MAX_CELLS_PER_BOARD];
	int numFilled;
	int sourceLength;
	int sourceSeeds[MAX_SOURCE_SEEDS];
	int numSourceSeeds;
};

struct GameLCG
{
	uint32 state;
};

//typedef char Move;
//enum MoveItems
enum Move
{
	MOVE_INVALID = -1,

	MOVE_W,
	MOVE_E,
	MOVE_SW,
	MOVE_SE,

	TURN_CW,
	TURN_CCW,

	NUM_MOVE_TYPES
};

//struct Obj
//{
//	ObjType type;
//	Cell pos;
//	bool isDead;
//};

struct Command
{
	char character;
};

const int MAX_COMMANDS = 1024 * 1024;

struct Solution
{
	int problemId;
	int seed;
	//char tag[64];
	char *pcommandBuffer;
	int numCommands;
	//int score;
};

struct Output
{
	Solution solutions[MAX_SOURCE_SEEDS];
	int numSolutions;
};

enum BoardCellState 
{
	CELL_EMPTY,
	CELL_FILLED,
};

struct BoardCell
{
	BoardCellState state;
    //int unitNum; // for debugging purposes
};

struct UnitPose
{
	Cell position;
	int orientation; // 0..5
};

struct ParentPointer
{
	UnitPose poseParent;
	Move moveToHere;
	char commandToHere;
	//PowerStep powerStepToHere;
	//int powerScore;
};

struct FinalMove
{
	UnitPose poseFinal;
	//UnitPose lastParent;
	Move moveToEnd;
	char commandToEnd;
	//PowerStep powerStepToEnd;
	int powerScore;
};

struct Board 
{
	BoardCell cells[MAX_WIDTH][MAX_HEIGHT];
	int usedPositions[MAX_WIDTH][MAX_HEIGHT];
    int numEmptyCellsPerRow[MAX_HEIGHT];
	int powerStepUsedBitField;

	int currentUnitIndex;
	UnitPose currentUnitPose;

	FinalMove finalPositionList[MAX_FINAL_POSITIONS];
	int numFinalPositions;
	ParentPointer finalPositionsParents[MAX_WIDTH][MAX_HEIGHT][NUM_ORIENTATIONS];

	int linesClearedPrev;

	GameLCG lcg;
	int unitsSpawned;

	int last_TileScore;
	int totalScoreUnits;

	int lastWordScore;
	int lastWordCount;
	int currentWordScore;
	int currentWordCount;
	int totalScoreWords;
	int currentWordIndex;
};

enum GameState
{
	GAME_PLAYING,
	GAME_OVER,
};

struct PowerWord
{
	char chars[MAX_POWER_WORD_LEN + 1];
	int len;
};

struct Sim
{
	Board board;
	int gameIndex;
	GameState gameState;

	Problem problem;
	Output ouput;

	int scoreUnits[NUM_PROBLEMS][MAX_SOURCE_SEEDS];
	int scoreWords[NUM_PROBLEMS][MAX_SOURCE_SEEDS];

	PowerWord powerPhrases[NUM_POWERSTEPS];
	int numPowerPhrases;
	Move powerPhraseMoves[NUM_POWERSTEPS][MAX_POWER_WORD_LEN];
	//int powerStepLenScore[NUM_POWERSTEPS];
	int sortedPowerPhrasesForUsed[NUM_POWERSTEPS];
	int sortedPowerPhrasesForUnused[NUM_POWERSTEPS];
};

extern Sim gSim;

void HoneyLoadProblem(const char* filename);
void HoneyEnumerateFinalPositions();
void HoneyEnumerateFinalPositionsRecursiveWithScore(UnitPose floodUnitPose, int powerStepUsedBitField, PowerStep previousPowerStep, int curScore);
void HoneyEnumerateFinalPositionsRecursive(UnitPose floodUnitPose);
void HoneyChooseEnumerateMove(int index);
void HoneyStartGame(bool resetSeed);
void HoneyStartGameSeed(int seedIndex);
void HoneyMarkPoseAsUsed(const UnitPose &unitPose);
MoveResult HoneyMoveCommand(Move move, char command);
MoveResult HoneyMove(Move move);

void GameLCGInit(GameLCG &rng, uint32 seed);
uint32 GameLCGNext(GameLCG &rng);

bool IsEven(int x);
const Unit &UnitGet(int unitIndex);
int GetUnitPositions(int unitIndex, const UnitPose &unitPose, Cell positions[], Cell &pivot);
int GetUnitPositionsWithBounds(int unitIndex, const UnitPose &unitPose, Cell positions[], Cell &pivot, Bounds& bounds);
bool CheckRowClear(int row);
int CalcScore(int unitSize, int linesCleared, int linesClearedPrev);
//void DumpOutput(int problemIndex, const char* aiStr, const char* method);
Move HoneyCommandToMove(char ch);
void InitHoneySimSystems();
void HoneyProcessPowerWords();
void HoneyCacheProblemInfo();
void DisposeOutput();
void CopyOutput(int source, int target);
void BoundsInflate(Bounds &bounds, const Cell &pos);
void InitOutput(int numSolutions);
bool OnBoard(Cell pos);
Cell GetUnitFirstPosition(int unitIndex, const UnitPose &unitPose);

struct HoneyOutputInfo {
	FILE *f;
	int numOutputs;
};

void DumpOutputInit(HoneyOutputInfo& info, int problemIndex);
void DumpOutputFinish(HoneyOutputInfo& info);

void HoneyInitOutputInfo(HoneyOutputInfo& info, FILE*f);
void HoneyOutputHeader(HoneyOutputInfo& info);
void HoneyOutputFooter(HoneyOutputInfo& info);

void HoneyOutputSolutionExt(HoneyOutputInfo& info, int seedIndex, int outputIndex, const char* aiName, const char* outMethod, int score);
void HoneyOutputSolution(HoneyOutputInfo& info, int seedIndex, int outputIndex, const char* tag);
void AppendPowerPhrase(const char *phrase);
void AddKnownPowerPhrases();
int MoveScoreWords(int& worldCount, int &wordScoreIndex);
Axial CellToAxial(Cell cell);
UnitPose PoseMove(UnitPose pose, Move move);
MoveResult ValidatePose(UnitPose newPose);

#endif //HONEYSIM_H
