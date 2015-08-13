#include "Precompile.h"
#include "GameApp.h"
#include "XUpcall.h"
#include "Sprite.h"
#include "Input.h"
#include "ResAsset.h"
#include "Font.h"
#include "AppController.h"
#include "Graphics.h"
#include "Draw3D.h"
#include "Coordinates.h"
#include "DrawMode.h"
#include "Particle.h"
#include "Texture.h"
#include "Game.h"
//#include "VR.h"
#include "GameEnum.h"
#include "Plane.h"
#include "OBoundingBox.h"
#include "Intersection.h"
#include "Sound.h"
#include "Structure.h"
#include "AssetUtils.h"
#include "HoneySim.h"
#include "HoneyAI.h"
#include "UIButton.h"
#include "HoneyGenes.h"

bool gAndroidLinkerTrickerAlwaysFalse = false;
int* gAndroidLinkerTrickerGlobal;


void AndroidLinkerTrickerNotCalled()
{
	// This makes the linker not strip some c++ junk that is needed by the engine library
	static AssetPtr FONT_DEBUG = AssetPtrFromPath("sys/DebugFont.font", ASSET_TYPE_FONT);
	XTrace("android linker tricker %s", (const char*)FONT_DEBUG->assetPath);
	//ovr_OnLoad(NULL); 
	gAndroidLinkerTrickerGlobal = new int[10];
	delete[] gAndroidLinkerTrickerGlobal;
	XHalt();
}

//bool IterateGameObjects(Obj *&iter)
//{
//	while(gGame.objects.IterateNext(iter))
//	{
//		if (!iter->isDead)
//		{
//			return true;
//		}
//	}
//
//	return false;
//}
//
//void ProcessDeleteQueue()
//{	
//	for (Obj *obj = NULL; gGame.objects.IterateNext(obj); )
//	{
//		if (!obj->isDead)
//		{
//			continue;
//		}
//
//		//GameObjectDispose(*obj);
//	}
//}
//
//typedef void (*GameObjectUpdateFn)(Obj &);
//typedef void (*GameObjectDrawFn)(Obj &);
//
//struct GameObjectFuncs	
//{
//	ObjType objectType;
//	GameObjectUpdateFn updateFn;
//	GameObjectDrawFn drawFn;
//};
//
//void DoNothing(Obj &obj) {}
//void DrawNothing(Obj& obj) {}
//
//GameObjectFuncs gGameObjectFuncs[] =
//{
//	{ GO_BOARD, DoNothing, DrawNothing },
//	{ GO_LOCKED_TILE, DoNothing, DrawNothing },
//	{ GO_PIECE,  DoNothing, DrawNothing },
//};

//GameObjectFuncs &GameObjectGetFuncs(Obj& obj)
//{
//	for (int i = 0; i < ARRAYSIZE(gGameObjectFuncs); i++)
//	{
//		GameObjectFuncs &funcs = gGameObjectFuncs[i];
//		if (funcs.objectType == obj.type)
//		{
//			return funcs;
//		}
//	}
//
//	XHalt();
//}

struct Game
{
	int   frame;
	float dt;
	float gameAge;

	//DataArray<Obj> objects;
	UIButton buttons[NUM_PROBLEMS];
	int numSeeds[NUM_PROBLEMS];
	int lastClickedButton;

} gGame;

GeneAI gGeneAI;

void LoadButtonClick(int button)
{
	gGame.lastClickedButton = button;
	switch(button)
	{
	case 0: HoneyLoadProblem("problems/problem_0.json"); break;
	case 1: HoneyLoadProblem("problems/problem_1.json"); break;
	case 2: HoneyLoadProblem("problems/problem_2.json"); break;
	case 3: HoneyLoadProblem("problems/problem_3.json"); break;
	case 4: HoneyLoadProblem("problems/problem_4.json"); break;
	case 5: HoneyLoadProblem("problems/problem_5.json"); break;
	case 6: HoneyLoadProblem("problems/problem_6.json"); break;
	case 7: HoneyLoadProblem("problems/problem_7.json"); break;
	case 8: HoneyLoadProblem("problems/problem_8.json"); break;
	case 9: HoneyLoadProblem("problems/problem_9.json"); break;
	case 10: HoneyLoadProblem("problems/problem_10.json"); break;
	case 11: HoneyLoadProblem("problems/problem_11.json"); break;
	case 12: HoneyLoadProblem("problems/problem_12.json"); break;
	case 13: HoneyLoadProblem("problems/problem_13.json"); break;
	case 14: HoneyLoadProblem("problems/problem_14.json"); break;
	case 15: HoneyLoadProblem("problems/problem_15.json"); break;
	case 16: HoneyLoadProblem("problems/problem_16.json"); break;
	case 17: HoneyLoadProblem("problems/problem_17.json"); break;
	case 18: HoneyLoadProblem("problems/problem_18.json"); break;
	case 19: HoneyLoadProblem("problems/problem_19.json"); break;
	case 20: HoneyLoadProblem("problems/problem_20.json"); break;
	case 21: HoneyLoadProblem("problems/problem_21.json"); break;
	case 22: HoneyLoadProblem("problems/problem_22.json"); break;
	case 23: HoneyLoadProblem("problems/problem_23.json"); break;
	case 24: HoneyLoadProblem("problems/problem_24.json"); break;
	default:
	XHalt();
	}

	gGame.numSeeds[button] = gSim.problem.numSourceSeeds;

	DisposeOutput();
	InitOutput(1);

	HoneyStartGame(true);
}

void ButtonInit(UIButton &button, const XString &str, RectF rect)
{
	//else if (aStyle == BUTTONSTYLE_GOLD)
	{
		button.mFont = FONT_MANATEE12;
		//button.mClickSound = SOUND_NEW_BUTTON_CLICK;
		button.mTextColor = gRgbaIWhite;
		button.mTextHighlightColor = gRgbaIWhite;
		button.mTextDisabledColor = ColorRgbaISet(80, 80, 80, 255);
		button.mImageNormal = TEXTURE_DIALOG_GOLD_BUTTON;
		button.mImageOver = TEXTURE_DIALOG_GOLD_BUTTON_OVER;
		button.mImageDisabled = TEXTURE_DIALOG_GOLD_BUTTON_DARK;
		button.mImageDown = TEXTURE_DIALOG_GOLD_BUTTON_DOWN;
		button.mTextDownOffset.y = 3.0f;
		button.mTextOffset.y = -3.0f;
	}

	button.mText = str;
	button.mScreenPos = V2(rect.x0, rect.y0);
	button.mScreenSize = RectSize(rect);
	button.mScreenSize.y -= 2.0f;
}

void GameInit()
{
	AddKnownPowerPhrases();

    InitHoneySimSystems();

	LoadButtonClick(0);
	//HoneyLoadProblem("problems/problem_0.json");
	//HoneyStartGame(true);

	for(int i = 0; i < NUM_PROBLEMS; ++i)
	{
		XString str = XFormatString("Problem %d", i);
		RectF rect = RectSetCenterSize(V2(950, i * 30.0f + 25), V2(100, 30));
		ButtonInit(gGame.buttons[i], str, rect);
	}
}

void GameUpdate()
{
	for(int i = 0; i < NUM_PROBLEMS; ++i)
	{
		gGame.buttons[i].Update();
		if (gGame.buttons[i].mClickedLastUpdate)
		{
			LoadButtonClick(i);
		}
	}
}

float CellScale()
{
	float scale = 1 / (Max(gSim.problem.width, gSim.problem.height) / 12.0f);
	return scale;
}
Vec2 CellSize(float scale)
{
	return V2(56.0f, 50.0f) * scale;//CellScale();
}

Vec2 CellToWorld(Cell cell, float scale)
{
	Vec2 size = CellSize(scale);
	Vec2 pos = MulEl2(V2(cell.x+.5f, cell.y+.5f), size);
	if (!IsEven(cell.y)) pos.x += size.x/2.0f;
	pos.x += 2;
	pos.y += 4;

	return pos;
}

#include "HoneyAI.h"
//AIFn aiFn = AIGreed2;

int gCurrentAlg = 0;
bool gShowRemainingUnits = false;
const int kRemainingUnitsPerRow = 12;
const float kRemainingPartsScale = 0.25f;

void DrawUnit(UnitPose unitPose, uint32 index, float scale)
{
	Cell positions[MAX_UNIT_MEMBERS];
	Cell pivot;
	int numMembers = GetUnitPositions(index, unitPose, positions, pivot);
	for(int i = 0; i < numMembers; ++i)
	{
		Vec2 pos = CellToWorld(positions[i], scale);
		DrawImageAlignedScale(TEXTURE_HEX_ACTIVE, RectSetCenterSize(pos, V20), TA_CENTER_MIDDLE, scale);
	}

	Vec2 pivotPos = CellToWorld(pivot, scale);
	DrawImageAlignedScale(TEXTURE_PIVOT, RectSetCenterSize(pivotPos, V20), TA_CENTER_MIDDLE, scale);
}

void AlgorithmToString(const Gene& gene, XString& desc)
{
	switch(gene.cmd)
	{
	case kGeneSetAI:
		if (gene.canPerturbContext)
		{
			desc = XFormatString("%d SetAI: %s (%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f)", gene.len, ALGORITMS[gene.data].str,
				gene.funcContext.context1,
				gene.funcContext.context2,
				gene.funcContext.context3,
				gene.funcContext.context4,
				gene.funcContext.context5,
				gene.funcContext.context6,
				gene.funcContext.context7,
				gene.funcContext.context8,
				gene.funcContext.context9
				);
		}
		else
		{
			desc = XFormatString("%d SetAI: %s", gene.len, ALGORITMS[gene.data].str);
		}
		break;
	case kGeneSetWord:
		desc = XFormatString("0 SetAI: PowerWorder");
		break;
	}
}

Board gVisualizationBoard;
Board gVisualizationBoardScratch;
int gVisualizationTurnedOnMode = 0; // 0: off, 1: recursive, 2: recursive with score

void GameDraw() 
{
	XSetCursor(CURSOR_ARROW);
	gGraphicsInterface->RenderTargetClear(gRgbaIBlue);
	//BackgroundDraw();
	//GameObjectsDraw();

	float scale = CellScale();

	for(int j = 0; j < gSim.problem.height; ++j)
	{
		for(int i = 0; i < gSim.problem.width; ++i)
		{
			Vec2 pos = CellToWorld(V2I(i,j), scale);

			//if (gSim.board.cells[i][j].state == CELL_FILLED && gSim.board.cells[i][j].unitNum == gSim.board.unitsSpawned - 1)
			//{
			//	DrawImageAlignedScale(TEXTURE_HEX_LAST_FILLED, RectSetCenterSize(pos, V20), TA_CENTER_MIDDLE, scale);
			//}
			//else 
			if (gSim.board.cells[i][j].state == CELL_FILLED)
			{
				DrawImageAlignedScale(TEXTURE_HEX_FILLED, RectSetCenterSize(pos, V20), TA_CENTER_MIDDLE, scale);
			}
			else
			{
				DrawImageAlignedScale(TEXTURE_HEX, RectSetCenterSize(pos, V20), TA_CENTER_MIDDLE, scale);
			}
		}
	}

    if (gVisualizationTurnedOnMode)
    {
        for (int i = 0; i < gVisualizationBoard.numFinalPositions; i++)
        {
            Cell positions[MAX_UNIT_MEMBERS];
            Cell pivot;
            Bounds bounds;
            int numMembers = GetUnitPositionsWithBounds(gVisualizationBoard.currentUnitIndex, gVisualizationBoard.finalPositionList[i].poseFinal, positions, pivot, bounds);

            for (int j = 0; j < numMembers; ++j)
            {
                Vec2 pos = CellToWorld(positions[j], scale);
                DrawImageAlignedScale(TEXTURE_POTENTIAL_MOVE, RectSetCenterSize(pos, V20), TA_CENTER_MIDDLE, scale);
            }
        }
    }


	if (gSim.gameState == GAME_OVER)
	{
		RectF rectGameOver = RectSet(0, 0, (15) * 56.0f, (10) * 50.0f);
		DrawStringAligned("Game Over", FONT_MANATEE18, rectGameOver, gRgbaIWhite, TA_CENTER_MIDDLE);
	}
	else
	{
		DrawUnit(gSim.board.currentUnitPose, gSim.board.currentUnitIndex, scale);
	}

	if(gShowRemainingUnits)
	{
		GameLCG unitIndexLCG;
		GameLCGInit(unitIndexLCG, gSim.board.lcg.state);
		int remainingCount = 0;
		UnitPose remainingUnitPose;
		remainingUnitPose.orientation = 0;
		for(int i = gSim.board.unitsSpawned; i < gSim.problem.sourceLength; ++i)
		{
			uint32 unitIndex = GameLCGNext(unitIndexLCG) % gSim.problem.numUnits;
			Cell pos = V2I(remainingCount%kRemainingUnitsPerRow*5 + 2, remainingCount/kRemainingUnitsPerRow*5 + 2);
			remainingUnitPose.position = pos;
			DrawUnit(remainingUnitPose, unitIndex, kRemainingPartsScale);
			remainingCount++;
		}
	}

	for(int i = 0; i < NUM_PROBLEMS; ++i)
	{
		int totalUnits = 0;
		int totalWords = 0;
		for(int n = 0; n < gGame.numSeeds[i]; ++n)
		{
			totalUnits += gSim.scoreUnits[i][n];
			totalWords += gSim.scoreWords[i][n];
		}

		if (totalWords+totalUnits > 0)
		{
			RectF rect = RectSet(gGame.buttons[i].mScreenPos, gGame.buttons[i].mScreenPos+gGame.buttons[i].mScreenSize);
			rect.x1 = rect.x0;
			rect.x0 -= 200;
			int averageUnits = totalUnits / gGame.numSeeds[i];
			int averageWords = totalWords / gGame.numSeeds[i];
			XString scoreStr = XFormatString("%d+%d=%d", averageUnits, averageWords, averageUnits+averageWords);
			DrawStringAligned(scoreStr, FONT_MANATEE12, rect, gRgbaIWhite, TA_RIGHT_MIDDLE);
		}
		gGame.buttons[i].Draw();
	}

	if (gSim.board.last_TileScore > 0)
	{
		XString tileScore = XFormatString("+%d", gSim.board.last_TileScore);
		RectF rectLastScore = RectSet(200, 700, 300, 730);
		DrawStringAligned(tileScore, FONT_MANATEE18, rectLastScore, gRgbaIWhite, TA_LEFT_MIDDLE);
	}

	if (gSim.board.lastWordScore > 0)
	{
		XString tileScore = XFormatString("+%d", gSim.board.lastWordScore);
		RectF rectLastScore = RectSet(550, 700, 600, 730);
		DrawStringAligned(tileScore, FONT_MANATEE18, rectLastScore, gRgbaIWhite, TA_LEFT_MIDDLE);

		XString worldCount = XFormatString("(%d)", gSim.board.lastWordCount);
		RectF rectLastCount = RectSet(620, 700, 620, 730);
		DrawStringAligned(worldCount, FONT_MANATEE18, rectLastCount, gRgbaIWhite, TA_LEFT_MIDDLE);
	}

	if (gSim.board.currentWordIndex != -1 && gSim.board.currentWordIndex != 0)
	{
		const char *word = gSim.powerPhrases[gSim.board.currentWordIndex].chars;
		RectF rectTotalScoreW = RectSet(700, 700, 900, 730);
		DrawStringAligned(word, FONT_MANATEE18, rectTotalScoreW, gRgbaIWhite, TA_LEFT_MIDDLE);
	}

	XString totalScoreU = XFormatString("Score Units: %d", gSim.board.totalScoreUnits);
	RectF rectTotalScoreU = RectSet(10, 700, 160, 730);
	DrawStringAligned(totalScoreU, FONT_MANATEE18, rectTotalScoreU, gRgbaIWhite, TA_LEFT_MIDDLE);

	XString totalScoreW = XFormatString("Score Words: %d", gSim.board.totalScoreWords);
	RectF rectTotalScoreW = RectSet(350, 700, 500, 730);
	DrawStringAligned(totalScoreW, FONT_MANATEE18, rectTotalScoreW, gRgbaIWhite, TA_LEFT_MIDDLE);

	XString totalScoreTotal = XFormatString("Score Total: %d", gSim.board.totalScoreWords + gSim.board.totalScoreUnits);
	RectF rectTotalScoreT = RectSet(10, 730, 160, 760);
	DrawStringAligned(totalScoreTotal, FONT_MANATEE18, rectTotalScoreT, gRgbaIWhite, TA_LEFT_MIDDLE);

	RectF rectAlg = RectSet(300, 730, 500, 760);
	DrawStringAligned(ALGORITMS[gCurrentAlg].str, FONT_MANATEE18, rectAlg, gRgbaIWhite, TA_LEFT_MIDDLE);
	
	XString piecesLeftStr = XFormatString("Units Left: %d", gSim.problem.sourceLength-gSim.board.unitsSpawned+1);
	RectF rectPieceLeft = RectSet(600, 730, 800, 760);
	DrawStringAligned(piecesLeftStr, FONT_MANATEE18, rectPieceLeft, gRgbaIWhite, TA_LEFT_MIDDLE);

	if(gGeneAI.numSolutions > 0)
	{
		// print latest GA plan
		float y = 0.0f;

		RectF rect = RectSet(0, y, 100, y+20);
		GeneSolution* pBest = &gGeneAI.solutions.front();
		XString best = XFormatString("Best: %d", pBest->score);
		DrawStringAligned(best, FONT_MANATEE12, rect, gRgbaIWhite, TA_LEFT_MIDDLE);
		y += 20.0f;

		int numGenes = pBest->sequence.size();
		for(int i = 0; i < numGenes; ++i, y += 20.0f)
		{
			Gene gene = pBest->sequence[i];
			XString desc;
            AlgorithmToString(gene, desc);
            RectF rect = RectSet(0, y, 100, y+20);
			DrawStringAligned(desc, FONT_MANATEE12, rect, gRgbaIWhite, TA_LEFT_MIDDLE);
		}
	}

	for(int i = 0; i < NUM_ALGORITHMS; ++i)
	{
		RectF rectAlg = RectSet(600.0f, i*20.0f, 700.0f, (i+1)*20.0f);
		XString str = XFormatString("%s: %d", ALGORITMS[i].str, ALGORITMS[i].usedInBestContextCount);
		DrawStringAligned(str, FONT_MANATEE12, rectAlg, gRgbaIWhite, TA_LEFT_MIDDLE);
	}
}

void ClampAlgIndex(int &currentAlg)
{
	if (currentAlg < 0) currentAlg += NUM_ALGORITHMS;
	if (currentAlg >= NUM_ALGORITHMS) currentAlg -= NUM_ALGORITHMS;
}

bool GameInputHandle(const InputData& input)
{
	if (input.message == XINPUT_CHAR && input.key == '`' && input.modifiers == 0)
	{
		gShowRemainingUnits = true;
	}
	else
	{
		gShowRemainingUnits = false;
	}

	if (input.message == XINPUT_CHAR && input.key == 'n' && input.modifiers == 0)
	{
		gCurrentAlg--;
		ClampAlgIndex(gCurrentAlg);
	}

	if (input.message == XINPUT_CHAR && input.key == 'm' && input.modifiers == 0)
	{
		gCurrentAlg++;
		ClampAlgIndex(gCurrentAlg);
	}

/* 
	if (input.message == XINPUT_CHAR && input.key == 'o' && input.modifiers == 0)
	{
		DumpOutput(gGame.lastClickedButton, ALGORITMS[gCurrentAlg].str, "o-output");
	}
	*/

	if (input.message == XINPUT_CHAR && input.key == ']' && input.modifiers == 0)
	{
		Path path;
		AIRandom(path);
		for(int i = 0; i < path.numMoves; ++i)
		{
			HoneyMove(path.moves[i]);
		}
	}

	if (input.message == XINPUT_CHAR && input.key == 'v' && input.modifiers == 0)
	{
        memcpy(&gVisualizationBoardScratch, &gSim.board, sizeof(Board));

        gVisualizationTurnedOnMode = (gVisualizationTurnedOnMode + 1) % 3;

        if (gSim.gameState == GAME_PLAYING)
        {
            if (gVisualizationTurnedOnMode == 1)
		        HoneyEnumerateFinalPositionsRecursive(gSim.board.currentUnitPose);
            else if (gVisualizationTurnedOnMode == 2)
		        HoneyEnumerateFinalPositionsRecursiveWithScore(gSim.board.currentUnitPose, gSim.board.powerStepUsedBitField, POWERSTEP_INVALID, 0);
        }

        memcpy(&gVisualizationBoard, &gSim.board, sizeof(Board));
        memcpy(&gSim.board, &gVisualizationBoardScratch, sizeof(Board));

    }

	if (input.message == XINPUT_CHAR && input.key == ' ' && input.modifiers == 0)
	{
		HoneyStartGame(false);
	}

	if (input.message == XINPUT_CHAR && input.key == '1' && input.modifiers == 0)
	{
		if (gSim.gameState == GAME_PLAYING)
		{
			HoneyEnumerateFinalPositions();
			XAssert(gSim.board.numFinalPositions > 0);
			int index = RandomInt(gSim.board.numFinalPositions);
			//int index = 0;
			HoneyChooseEnumerateMove(index);
		}
	}

	if (input.message == XINPUT_CHAR && input.key == '2' && input.modifiers == 0)
	{
		if (gSim.gameState == GAME_PLAYING)
		{
			RunAI(gCurrentAlg);
		}
	}

	if (input.message == XINPUT_CHAR && input.key == '3' && input.modifiers == 0)
	{
		//const char* solution = "iiiiiiimimiiiiiimmimiiiimimimmimimimimmeemmimimiimmmmimmimiimimimmimmimeeemmmimimmimeeemiimiimimimiiiipimiimimmmmeemimeemimimimmmmemimmimmmiiimmmiiipiimiiippiimmmeemimiipimmimmipppimmimeemeemimiieemimmmm";
		//const char* solution = "aadddlllllalllllllbbaallalalalallallaaddddddaaddddalalaaalaaaallllllllalllaaalaaalaaaallldddllllbbballalalbaapppdddlalaaallbbbllaappaal";
		//const char* solution = "iiiiiiiimmiiiiiimimmiiiimimimmimimimimmimimimeemimeeeemimimimimiiiiiimmeemimimimimiimimimmeemimimimmeeeemimimimmiiiiiipmiimimimeeemmimimmemimimimiiiiiimeeemimimimimeeemimimimmiiiimemimimmiiiipimeeemimimmiiiippmeeeeemimimimiiiimmimimeemimimeeeemimimiiiipmeeemmimmiimimmmimimeemimimimmeeemimiiiiipmiiiimmeeemimimiiiipmmiipmmimmiippimemimeeeemimmiipppmeeeeemimimmiimipmeeeemimimiimmeeeeemimmeemimmeeeemimiiippmiippmiiimmiimimmmmmeeeemimmiippimmimimeemimimimmeemimimimmeemimimimiimimimeeemmimimmmiiiiipimeemimimimmiiiimimmiiiiiiiimiimimimimeeemmimimimmiiiiiimimmemimimimimmimimimeemimiiiiiiiimiiiimimimiimimimmimmimimimimmeeeemimimimimmmimimimimeemimimimimmmemimimmiiiiiiimiimimimmiiiiiimeeeeemimimimimmimimimmmmemimimmeeeemimimimmiimimimmiiiiiipmeeeeemimimimimmiiiiimmemimimimimmmmimimmeeeemimimimimeeemimimimmiimimimeeemmimimmiiiiiiimimiiiiiimimmiiiiiiiimmimimimimiiiimimimeemimimimimmeeemimimimimiiiiiiimiiiimimmemimimimmeemimimimeeemmimimmiiiiiimmiiiipmmiiimmmimimeemimimeeemmimmiiiippmiiiimiiippimiimimeemimimeeeemimimiiiipmeemimimiimiimimmimeeemimimmippipmmiimemimmipimeeeemimmeemimiippimeeeeemimimmmimmmeeeemimimiiipimmiipmemimmeeeemimimiipipimmipppimeeemimmpppmmpmeeeeemimmemm";
		//const char* solution = "aaaaaaaaaaaalaaaaaaaaaaaaaallaaaaaaaaaaaaalbbblaaaaaaaaaaaaalbbbblaaaaaaaaaaabbbbbbbbbbbbkpppaaaaaaaaaaabbbblblaaaaaaaaaabbbbbbbblbbbkaaaaaaaaaabbbbbbbblbbbkaaaaaaaaapppdbbbbbkbbbbbbbbbkblaaaaaaaaaaabbbbbbblbllaaaaaaaaaabbbbbbblblblaaaaaaaaaaabbbbbbbbblblbaaaaaaaaaaaaalaaaaaaaaaaabbbbbbbdaaaaaaaaaaabbbbbbbkaaaaaaaaaaaabblbaaaaaapppdbbbbbbbbbkpppppkbbbkaaaaaaaaaaaaalbaaaaaaaapppdpapalaaaaaaaaaaaaalaaaaaappppdbbbbbbbbbkppppkpkaaaaaaaaaaaaabbbbbblbbbbaaaaaaaaaaabbbbbllbaaaaaaaaaaabbdaaaaaaaaaaabbbbbblaaaaaaaaaaaaaaaaaaaaaabbbbblblbllaaaaaaaaaaaaablaaaaaaaaaaaaabbbblaaaaaaaaaaaabbbbblaaaappppppdpppdbbbbbbbbbbbbbkpppkbbbbbbbkpppppppppkbkaaaaaaaaaabbbblbbbbblbaaaaaaaaabbbbbbbdaaaaaaaaaabbbblbblbaaaaaaaaaaaabbbbbbbblaaaaaaaabbbbbbbdaaaaaaaaabbbbbbkpppppaaaaaaaaaapaaaaappppdbbbbbbbbbkppppkkaaaaaaaallaaaaaaaaaablaaaaappppdbbbbbbbbbkppppkbbbkaaaaaaaaaabbbbllaappppppdbbbbbbbbbkppkbbbbbbbbbbkpppppppkaaaaaaaaabbblbbbbbkaaaaaappppdbbbbbbbbbkppppkbbbbbbbbbkaaaaaaaaaaabbbbbbbbdaaaaaaaaaaaaalbbbbbbbbblaaaaaaaaaaaabbbbblbbbaaaaaaaaaaaaalbbbbblaaaaaaaaaaabbdaaaaaaaaaaaaapaaaaaaaaaaaalabbbbbblaaaaaaaaaaapdbbkbbbbbbbdaaaaaaaaaaapdbbkbbbbbbdaaaaaaaaaaapdbbkbbbbbdaaaaaaaaaaaalbbbbbaaaaaaaaaappdbaaaaaaaaaappdaaaaaaaaaabbblbkaaaaaaaaaabbblkaaaaaapppdbbbbbbbbbkpppppkbkaaaaaaaaaaaaabbbbbbbllaaappppppdpppdbbbbdpaaappppppdpppdbbbbdaaaaaaaaaaabbbbblbbbbbbbaaaaaaaaaalbbbbblbbblaaaaaaaaaaaaabbbbbbbbbbllaaaappppdpppppdbbbbbbbbbbbbbkpppkbbbbbbbkpppppppppkpkbbbbbdbbbbkaaaaaaaaaaabbbbbbbbbblaaaaaaaaapppdbbbbbbbbkpppkbbbbbbbbbdppppdaaaaaaaaaaaaabblbbbblaaaaaaaaaaaaabblbbbbblaaaaaaaappdbbbbbbbkpppkbbbbbbbbbbbbbaaaaaaaaappdbbbbbbkpppkbbbbbbbbbbbbaaaaaaapppdbbbbbkpkbbbbbbbbdppppdaaaaaaaaaaabbbbbbbbkbbbbaaaaaaaaaaaablbbbbbbbaaaaappppdpdbbbbbbkbbkppkbbbbbbbbbkaaaaaaaaaappdbbbkbbbbkbbbdaaaaaaaaaappdbbbkbbbbbkbbdaaaaaaaaaaapdbbbkbbbbkbbdaaaaaaaabbbbbbbbbkaaaaaaaaaaappdbbbbkbbbbkpppaaaaaaaaaappdbbbkbbbbkpppaaaaaaabbbbbbbblaaaaapppppdppdbbbbbbkbbbbkppkbbbbbbbbdppppdaaaappppdpdbbbbkbbbkpkbbbbbbbdpppdaaaaaaaaaapdpppppaaaaaaaaaaapaaaaaaaalbbbbkpaaaaaaapppdppppaaaaaaaaaapdppppaaaaaaaaaldppkaaaaaaalladppppaaaaaaaapdappaaaaaaaapdapaaaaaaaalblblapaaaaappppdbbddaaaaaaaabbbbbbbklaaa";
		//const char* solution = "aaaaaaaalaaaaaaaaablaaaaaaapkllbbbbbbbaaaaaaalaaaaaapdbbblalbbdbaaaaaaapdbbllaaaaaaaapkbbblllaaaaaaapkbbbllbkpkbblaaaaaaaaaaaaaaablblaaaaaabbbllaaaaapdbbbbblbldaaaaaapdpdbbbbbbldaaaaaaablaaaaaadldblbaaaaaaabbbbbkpppaaaaaapkbbblalaaappdbbblblaaaaalkppaaaaaldpadaaaappdbbbblldaaaldppdlbbbbdppadaaaapdpdbbbdbbbldaaaappdbbbdppdpdbbbbbklappppdpdbbbdpdbbbdbbblllaapppdbbbbbbbldaaapappdbbappaapadlaaaabbappaapadlaaaapkbbbbblbbkbkpaaaabbbbbbdldaaaapdpdpdbbbbbbaaaaapaaaaapaaaaaappkbbbbbkppppkbbbbaaaaabaappapdabbaaappkbbbblbbbbkppkaappaadbdaaapppadbbbbbbldlaaaapapklaaappdabbbbbbblaaaaaaaaaabblaaaaldppdlaaaablbbdbbaaaaabblbkbaaaappdbbbbbbbdppppppdpdbbbbbbbblaaaaaabbbbkaaaaaaaaaaaaapdpdpdbbbbbbbklaaaaaalbdpdaaappdbbbbbbbdlaaapppkpkbbbbbbbbkppppppkbbbbbbbkkaaaaaapdbbbbbdppppdpdbbbbbbdaaaaaaaaaaaalbbdaaaaabdpppdbbddaaaaappdabbbaaaaappdbaaaaabbbblaaaaapadpdaaaaaapdaaaaaaaaaaalbblaaaalbdaaaaaabbbllaaaappdlbbdblaaaaappdbbbbbblbaaaaaapkbbbbbbklaaaaaalaaaaaapdpdaaaaaalaaaaaabblaaaappkabbblbklbkkaaaaaaaaaaaaapkbbbblbbbbklaaaaaapdbbbldpppdbbbblbbbdpppaaaaaapdabbblaaaaaabbbbbllaaaaaaaabbbdppppdaaaaaaaaaaaaaaaabbbblbbbaaaaaallbbdapdpaaaaaapaaaaaaalbllbbblaaaaaalbllbbbaaaaaallbbdppdaaaaaalbbbbbbdppppppdaaaaappdbbblbblaaaaaapdbbblbbkpkbaaaaapdbbbbbbbkpppppkpkaaaaaapdpdbblbbbbbbkpppppkbbbbbaaaaaabblbbaaaalbdldadpddaaaaaaapdbbbbbbbbdaaaaaappdbbbblaaaaaaapdabbdlaaaaaaapdbdpa";
		//const char* solution = "YuggothIa! Ia!R'lyehEi!R'lyehR'lyehR'lyebbbbbdaYuggothYuggothYuggothYuggothppppppdbR'dbbbbbbbbbdpppppppppdbbbbbbbbbbEi!YuggothYuggothYuggothYuggothppppppdbR'lyebbbbbbaYuggothYuggothYuggothYuggothppppppdbR'dbbbbbdpppppdbbbbbbEi!YuggothYuggothYuggothIa! Ia!ppaYuggothYuggothYuggothIa! IppaaYuggothYuggothYuggothR'lyehpapR'lyedppaaYuggothYuggothYuggothR'lyehpppppppdbbbEbbbbbbbdppppppppppppaYuggothYuggothYuggothR'lyebbbbdpppppppppdppaaYuggothYuggothYuggothR'lyehpppdpdbbbbbbbbaYuggothYuggothYuggothR'lyehpppdaaYuggothYuggothYuggothR'lyehR'dbbbbEi!YuggothYuggothYuggothR'lyehEi!dbdpdbdpdbbbYuggothYuggothYuggothppppppdbbbEbEibldpdEi!YuggothYuggothYuggothpaaYuggothYuggothYuggotEiR'lyebbbdpppppdppppakkppddpaYuggothYuggothYuggotEiR'lyehpdpdbbbbbaYuggothYuggothYuggotEiR'lyedppdppppppaaYuggothYuggothYuggotEiR'lyedppdppkkbkbbbbbEi!YuggothYuggothYuggotEiEi!lbaYuggothYuggothYuggotEiapaaYuggothYuggothYuggotppppppdbbbbbdppppppdbbbbbbbbbbbbkppppkEi!YuggothYuggothIa! IppppdpaYuggothYuggothIa! ppdppapYuggothYuggothIppppppdbbbbbbbEbEiEiEiEbYuggothYuggothIppppppdbbbbbbbEbbEiaEi!YuggothYuggothIa! IpdpYuggothYuggothR'lyebbbbkpppppapadapYuggothYuggothR'lyehlYuggothYuggothaaapppppdaYuggothYuggothaappdppaaYuggothYuggothaapdbbEbEikbbbdaYuggothYuggothaapdbbEbEbEi!YuggothYuggothaaapdaYuggothYuggothaadaaYuggothYuggothppppppdbbbbbbbbbbbkppppkpppppppkbbbbbbbbbbbbdpppaaYuggothYuggotEiaabEdbbkkapaaYuggothYuggotEialbddaYuggothYuggothaaapppppdaYuggothYuggothaappdppaaYuggothYuggotpppppppdbbbbbbdpppppdbbbbbbbdpppppppdbbbbbbbbbR'lyehEildaYuggothYuggothaaapdaYuggothYuggothaakklaYuggothYuggotpppppppdbbbbbbdpppppdbbbbbbbdpppppppdbbbbbbbbbR'lyeladaYuggothYuggotpppppppdbbbbbbdpppppdbbbbbbbdpppppppdbbbbbbbbbEbdppppdbbEilaYuggothYuggothppppppkbbbbbbbblaYuggothYuggotEiappappppdpaYuggothYuggotEiappppdppaaYuggothYuggotEiappapdaYuggothYuggotEiadppaaYuggothYuggppppppdbbbbbbbbdppppppppdbbbbbbbbbEbEiEi!YuggothYuggotEkpppppppdpdaYuggothYuggpppppaaYuggothYuggppdbbbbdpppppdpdbbbbbbbblddbaYuggothYuggotEkpppkbbbkbkaYuggothEi!R'lyelaYuggothaR'lyehappdapppaYuggothaR'lyehappdaaYuggothaR'lyehdddppakklaYuggothapppdbbbdpppppdbbbbbbbEbEbdppppppppppppdbbbbbbbbEbEilaYuggothppppppdbbbbbbbbblkklaYuggothapppdbbbdapaYuggothppkaEi!YuggothblaYuggotppppaR'lR'lpaYuggotppppappkaEi!YuggotppppdbbR'pppdbbbbbbbbR'lyeEdlaaYuggotppppdbbR'pppdbbbbbbbbEbbdppppppppppppdbbbbbbbEbbEiaaYuggppppppdbbbbbbbbbEbEiaaYuggotppppdbbR'lyedpppdbbdppaEi!aaEi!Ei!ppppaabbbbbbblaEi!R'lyebEbbEkppppppppppappEi!R'lyebEbbEklaEi!R'lyebEdppppppdppapEi!R'lyebEi!Ei!aaR'lykpaEi!aaEbEdpddddblkkaEi!aaEi!EbbEbEidlaEi!pppppdbbbbbbbblaYuggodpppppaaaYuggodpppdpapklaYuggothalapappaapppdbdpdpappakppdpdddpappaadddpakkkpalaaa";
		//const char* solution = "Ia! Ia!aEiEi!laIa! Ia!aEiEiEi!yogsothothR'lyehEi!pppkbbbbbbbkaTsathogguaR'lyebdpppdaYuggothYuggothbbayogsothothEi!palayogsothothR'lyehppppapdayogsothothR'lyehappdayogsothothpppppdbbbbbEbEdlayogsothothakpppppkbbbbbEbkppppppkayogsothothpppdbbbEdlayogsothothlayogsothotEibddddpppappppdbbdddayogsothothR'lyebEdppppppppaayogsothothaddpayogsothothR'lyeEbEbkpppkayogsothothR'lyehppppdpdayogsothotEibEibayogsothothR'lyeEbkpppappdbdpayogsothotEikpppppkbbbbbbEidaayogsothotbbdlayogsothothR'ppppdbbbbbdaayogsothothR'lyehpkppapayogsothothR'lyehEi!daTsathoggupakppdayogsothothppakaaYuggothR'lyehppdpdbbbbbklayogsothothapppkEbbkppkbbbkpppayogsothothR'lyehpkblayogsothothEi!ppkEbbbkpppppppdbdpdbdpayogsothothR'lyehR'lyebkpkbayogsothothR'lyebbdppR'lyehyogsothothEi!papppkbEbEbkpppkaIa! Ia!aEibbbEiEi!yogsothothEi!layogsothothR'lyehpppppdpdbbbbbbbbkpkbblaIa! Ia!aEiEibEi!yogsothothR'lyedpppppppayogsothothR'lyehR'lyebbkppppppdbdpayogsothothR'lyehR'lyehyogsothothapkEbbkppkkayogsothothR'lyehEibbdayogsothothEibdddbdppapppdppayogsothothR'lyehappddayogsothothppppdbbbbEibdbayogsothothEbdaayogsothothR'lyedbayogsothothappppkEibbbbayogsothothR'lyehpapppdayogsothothEi!apppayogsothothEbEiaaTsathogguaR'lyekEi!pdayogsothothpapkEi!yogsothothEbbdabayogsothothR'lyekapppppppdbbbbbdpppppayogsothothR'lyehEi!pppdayogsothothYuggotppddayogsothothpppppkEi!yogsothothppppkEbbbbkppppkbbbkpppayogsothothppkEbbkppkbkpaYuggotEidEbbbdpppppppppddayogsothothbEiblayogsothothR'lyedpR'lyedpppdbbdppayogsothothEi!ppayogsothothppdEbEibbEbbkpppayogsothothppppdpdbbbbbbbEiaaYuggothppppkbEikpkbbbbbbbEi!Ia! Ia!abbEilayogsothotpppppakaayogsothotpppppkEibbbbblayogsothothR'lyehppppdayogsothothEi!layogsothothpppppdbbR'lyebbbbbdppppppppdpdayogsothothR'lyehppppdbbbbbbEkppkppppaIa! Ia!aEibbbEibbbbEi!yogsothothR'lyehEi!pkbbbbbkayogsothothR'lyehppkbEibayogsothothR'lyehpppppdpdayogsothothR'lyehppppdayogsothothpakbbkaayogsothothR'lyehEbEbkpppppppayogsothothR'lyehkppaayogsothothEikpppkbbbbEidaYuggothR'lyehppdpdbbbdayogsothothappkEi!yogsothothaayogsothothYuggotbbbkayogsothothEi!papaayogsothothEidbdaayogsothothbbbdpR'lyehYuggothR'lyeEbbdpppppapppdpdayogsothothEibdbdaaTsathogguaayogsothothR'lyebdpR'pppppaayogsothothR'lyehppdbbbbbkppkbbbayogsothothR'lyehpaayogsothothpapkEi!yogsothotpR'lyeR'lyebdppppppppkayogsothothR'lyedpppppayogsothotpdR'lyebbbdppppppdbbbbbda";
		const char* solution = "yuggothia! ia!r'lyehei!r'lyehr'lyehr'lyebbbbbdayuggothyuggothyuggothyuggothppppppdbr'dbbbbbbbbbdpppppppppdbbbbbbbbbbei!yuggothyuggothyuggothyuggothppppppdbr'lyebbbbbbayuggothyuggothyuggothyuggothppppppdbr'dbbbbbdpppppdbbbbbbei!yuggothyuggothyuggothia! ia!ppayuggothyuggothyuggothia! ippaayuggothyuggothyuggothr'lyehpapr'lyedppaayuggothyuggothyuggothr'lyehpppppppdbbbebbbbbbbdppppppppppppayuggothyuggothyuggothr'lyebbbbdpppppppppdppaayuggothyuggothyuggothr'lyehpppdpdbbbbbbbbayuggothyuggothyuggothr'lyehpppdaayuggothyuggothyuggothr'lyehr'dbbbbei!yuggothyuggothyuggothr'lyehei!dbdpdbdpdbbbyuggothyuggothyuggothppppppdbbbebeibldpdei!yuggothyuggothyuggothpaayuggothyuggothyuggoteir'lyebbbdpppppdppppakkppddpayuggothyuggothyuggoteir'lyehpdpdbbbbbayuggothyuggothyuggoteir'lyedppdppppppaayuggothyuggothyuggoteir'lyedppdppkkbkbbbbbei!yuggothyuggothyuggoteiei!lbayuggothyuggothyuggoteiapaayuggothyuggothyuggotppppppdbbbbbdppppppdbbbbbbbbbbbbkppppkei!yuggothyuggothia! ippppdpayuggothyuggothia! ppdppapyuggothyuggothippppppdbbbbbbbebeieieiebyuggothyuggothippppppdbbbbbbbebbeiaei!yuggothyuggothia! ipdpyuggothyuggothr'lyebbbbkpppppapadapyuggothyuggothr'lyehlyuggothyuggothaaapppppdayuggothyuggothaappdppaayuggothyuggothaapdbbebeikbbbdayuggothyuggothaapdbbebebei!yuggothyuggothaaapdayuggothyuggothaadaayuggothyuggothppppppdbbbbbbbbbbbkppppkpppppppkbbbbbbbbbbbbdpppaayuggothyuggoteiaabedbbkkapaayuggothyuggoteialbddayuggothyuggothaaapppppdayuggothyuggothaappdppaayuggothyuggotpppppppdbbbbbbdpppppdbbbbbbbdpppppppdbbbbbbbbbr'lyeheildayuggothyuggothaaapdayuggothyuggothaakklayuggothyuggotpppppppdbbbbbbdpppppdbbbbbbbdpppppppdbbbbbbbbbr'lyeladayuggothyuggotpppppppdbbbbbbdpppppdbbbbbbbdpppppppdbbbbbbbbbebdppppdbbeilayuggothyuggothppppppkbbbbbbbblayuggothyuggoteiappappppdpayuggothyuggoteiappppdppaayuggothyuggoteiappapdayuggothyuggoteiadppaayuggothyuggppppppdbbbbbbbbdppppppppdbbbbbbbbbebeiei!yuggothyuggotekpppppppdpdayuggothyuggpppppaayuggothyuggppdbbbbdpppppdpdbbbbbbbblddbayuggothyuggotekpppkbbbkbkayuggothei!r'lyelayuggothar'lyehappdapppayuggothar'lyehappdaayuggothar'lyehdddppakklayuggothapppdbbbdpppppdbbbbbbbebebdppppppppppppdbbbbbbbbebeilayuggothppppppdbbbbbbbbblkklayuggothapppdbbbdapayuggothppkaei!yuggothblayuggotppppar'lr'lpayuggotppppappkaei!yuggotppppdbbr'pppdbbbbbbbbr'lyeedlaayuggotppppdbbr'pppdbbbbbbbbebbdppppppppppppdbbbbbbbebbeiaayuggppppppdbbbbbbbbbebeiaayuggotppppdbbr'lyedpppdbbdppaei!aaei!ei!ppppaabbbbbbblaei!r'lyebebbekppppppppppappei!r'lyebebbeklaei!r'lyebedppppppdppapei!r'lyebei!ei!aar'lykpaei!aaebedpddddblkkaei!aaei!ebbebeidlaei!pppppdbbbbbbbblayuggodpppppaaayuggodpppdpapklayuggothalapappaapppdbdpdpappakppdpdddpappaadddpakkkpalaaa";
		static int index = 0;
		if (solution[index] == 0)
		{
			index = 0;
		}
		else
		{
			Move move = HoneyCommandToMove(solution[index]);
			HoneyMoveCommand(move, solution[index]);
			index++;
		}
	}

	if (input.message == XINPUT_CHAR && input.key == '9' && input.modifiers == 0)
	{
		DisposeOutput();
		InitOutput(1);

		HoneyOutputInfo output;
		DumpOutputInit(output, gGame.lastClickedButton);
		HoneyOutputHeader(output);

		for(int n = 0; n < gGame.numSeeds[gGame.lastClickedButton]; ++n)
		{
			HoneyStartGameSeed(n);
			while(1)
			{
				if (gSim.gameState == GAME_PLAYING)
				{
					RunAI(gCurrentAlg);
				}
				else
				{
					gSim.scoreUnits[gGame.lastClickedButton][n] = gSim.board.totalScoreUnits;
					gSim.scoreWords[gGame.lastClickedButton][n] = gSim.board.totalScoreWords;
					break;
				}
			}

			HoneyOutputSolutionExt(output, n, 0, ALGORITMS[gCurrentAlg].str, "9-output", gSim.board.totalScoreUnits + gSim.board.totalScoreWords);
		}

		HoneyOutputFooter(output);
		DumpOutputFinish(output);
	}

	int reps = 500;
	if (input.message == XINPUT_CHAR && input.key == '=' && input.modifiers == 0)
	{
		for(int i = 0; i < NUM_PROBLEMS-1 /*NUM_PROBLEMS*/; ++i)
		{
			LoadButtonClick(i);

			HoneyOutputInfo output;
			DumpOutputInit(output, gGame.lastClickedButton);
			HoneyOutputHeader(output);

			AIGenes(&gGeneAI, output, gSim.scoreUnits[i], gSim.scoreWords[i], reps);

			HoneyOutputFooter(output);
			DumpOutputFinish(output);
		}
	}

	if (input.message == XINPUT_CHAR && input.key == '-' && input.modifiers == 0)
	{
		for(int i = gSim.problem.id; i < gSim.problem.id + 1 /*NUM_PROBLEMS*/; ++i)
		{
			LoadButtonClick(i);

			HoneyOutputInfo output;
			DumpOutputInit(output, gGame.lastClickedButton);
			HoneyOutputHeader(output);

			AIGenes(&gGeneAI, output, gSim.scoreUnits[i], gSim.scoreWords[i], reps);

			HoneyOutputFooter(output);
			DumpOutputFinish(output);
		}
	}

	if (input.message == XINPUT_CHAR && input.key == '0' && input.modifiers == 0)
	{
		for(int i = 0; i < NUM_PROBLEMS - 1; ++i)
		{
			LoadButtonClick(i);

			HoneyOutputInfo output;
			DumpOutputInit(output, gGame.lastClickedButton);
			HoneyOutputHeader(output);

			for(int n = 0; n < gGame.numSeeds[i]; ++n)
			{
				HoneyStartGame(n == 0);
				while(1)
				{
					if (gSim.gameState == GAME_PLAYING)
					{
						RunAI(gCurrentAlg);
					}
					else
					{
						gSim.scoreUnits[i][n] = gSim.board.totalScoreUnits;
						gSim.scoreWords[i][n] = gSim.board.totalScoreWords;
						break;
					}
				}

				HoneyOutputSolutionExt(output, n, 0, ALGORITMS[gCurrentAlg].str, "0-output", gSim.board.totalScoreUnits + gSim.board.totalScoreWords);
			}

			HoneyOutputFooter(output);
			DumpOutputFinish(output);
		}
	}

	if (input.message == XINPUT_CHAR && input.key == 'a' && input.modifiers == 0)
	{
		HoneyMove(MOVE_W);
	}

	if (input.message == XINPUT_CHAR && input.key == 'd' && input.modifiers == 0)
	{
		HoneyMove(MOVE_E);
	}

	if (input.message == XINPUT_CHAR && input.key == 'q' && input.modifiers == 0)
	{
		HoneyMove(TURN_CCW);
	}

	if (input.message == XINPUT_CHAR && input.key == 'e' && input.modifiers == 0)
	{
		HoneyMove(TURN_CW);
	}

	if (input.message == XINPUT_CHAR && input.key == 'z' && input.modifiers == 0)
	{
		HoneyMove(MOVE_SW);
	}

	if (input.message == XINPUT_CHAR && input.key == 'c' && input.modifiers == 0)
	{
		HoneyMove(MOVE_SE);
	}

	//if (input.message == XINPUT_CHAR && input.key == 'p' && input.modifiers == 0)
	//{
	//	gGame.paused = !gGame.paused;
	//	gGame.singleStep = false;
	//	return true;
	//}

	//if (input.message == XINPUT_CHAR && input.key == 'o' && input.modifiers == 0)
	//{
	//	gGame.paused = true;
	//	gGame.singleStep = true;
	//	return true;
	//}

	if(input.message == XINPUT_CHAR && input.key == '5' && input.modifiers == 0)
	{
		GeneInit(&gGeneAI, 0);
	}
	
	if(input.message == XINPUT_KEYDOWN && input.key == '6')
	{
        if (input.modifiers == 1) // shift
        {
            for (int i = 0; i < 500; ++i)
		        GeneticStep(&gGeneAI);
        }
        else
        {
            GeneticStep(&gGeneAI);
        }
	}

	return false;
}

void GameDestroy()
{
	DisposeOutput();

	for(int i = 0; i < NUM_PROBLEMS; ++i)
	{
		gGame.buttons[i].mText = "";
	}
	//gGame.objects.DataArrayDispose();

}

/////////////////////////////////////////
/////////////////////////////////////////

class AppInterfaceGameApp : public AppInterface
{
public:
	virtual void UpcallStartForReal()						{ GameInit(); }
	virtual void UpcallDisposeForApp()						{ GameDestroy(); }
	virtual void UpcallUIDraw()								{ GameDraw(); }
	virtual void UpcallUpdateSimulation(int dtMs)			{ GameUpdate(); }
	virtual bool UpcallInputHandle(const InputData& input)	{ return GameInputHandle(input); }
	//virtual bool UpcallNetMessage(const NetworkMessage &message)  { JoustHandleMessage((NetworkMessage *)&message); return true; }
};

AppInterfaceGameApp gGameAppUpcallInterface;

void GameSetUpcallInterface()
{
	XUpcallSetInterface(&gGameAppUpcallInterface);

	gAppInterface->mPackFileName = "GameApp.xpack";
	//gAppInterface->mPackFileName = "JoustXPack.zip";
	gAppInterface->mSaveFileDirectory = "Honey";
	gAppInterface->mTitle = "Honey";
	gAppInterface->mAppWidth = 1024;
	gAppInterface->mAppHeight = 768;
	//gAppInterface->mHasZBuffer = true;
	//gAppInterface->mCreateMipMaps = true;
	//gAppInterface->mMultisamples = 2;
	//gAppInterface->mVRApp = false;
}
