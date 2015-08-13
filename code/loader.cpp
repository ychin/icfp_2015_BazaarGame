#include "Precompile.h"
#include "HoneySim.h"
#include "rapidjson/reader.h"
#include <iostream>
#include <fstream>
#include <streambuf>

using namespace rapidjson;
using namespace std;

enum ReadState
{
	READSTATE_NONE,
	READSTATE_ID,
	READSTATE_UNITS,
	READSTATE_MEMBERS,
	READSTATE_MEMBERS_X,
	READSTATE_MEMBERS_Y,
	READSTATE_PIVOT,
	READSTATE_PIVOT_X,
	READSTATE_PIVOT_Y,
	READSTATE_WIDTH,
	READSTATE_HEIGHT,
	READSTATE_FILLED,
	READSTATE_FILLED_X,
	READSTATE_FILLED_Y,
	READSTATE_SOURCE_LENGTH,
	READSTATE_SOURCE_SEEDS,
};

ReadState gReadState = READSTATE_NONE;

struct MyHandler {
    bool Null() { /*cout << "Null()" << endl;*/ return true; }
    bool Bool(bool b) { /*cout << "Bool(" << boolalpha << b << ")" << endl;*/ return true; }
    bool Int(int i) { /*cout << "Int(" << i << ")" << endl;*/ return true; }
    bool Uint(unsigned u)
	{
		switch (gReadState)
		{
		case READSTATE_ID: 
			gSim.problem.id = u;
			gReadState = READSTATE_NONE;
			break;

		case READSTATE_MEMBERS_X:
			{
				Unit& unit = gSim.problem.units[gSim.problem.numUnits];
				unit.members[unit.numMembers].x = u;
				gReadState = READSTATE_MEMBERS;
				break;
			}

		case READSTATE_MEMBERS_Y:
			{
				Unit& unit = gSim.problem.units[gSim.problem.numUnits];
				unit.members[unit.numMembers].y = u;
				gReadState = READSTATE_MEMBERS;
				break;
			}

		case READSTATE_PIVOT_X:
			{
				Unit& unit = gSim.problem.units[gSim.problem.numUnits];
				unit.pivot.x = u;
				gReadState = READSTATE_PIVOT;
				break;
			}

		case READSTATE_PIVOT_Y:
			{
				Unit& unit = gSim.problem.units[gSim.problem.numUnits];
				unit.pivot.y = u;
				gReadState = READSTATE_PIVOT;
				break;
			}

		case READSTATE_FILLED_X:
			gSim.problem.filled[gSim.problem.numFilled].x = u;
			gReadState = READSTATE_FILLED;
			break;

		case READSTATE_FILLED_Y:
			gSim.problem.filled[gSim.problem.numFilled].y = u;
			gReadState = READSTATE_FILLED;
			break;

		case READSTATE_WIDTH:
			gSim.problem.width = u;
			gReadState = READSTATE_NONE;
			break;

		case READSTATE_HEIGHT:
			gSim.problem.height = u;
			gReadState = READSTATE_NONE;
			break;

		//case READSTATE_FILLED:
		case READSTATE_SOURCE_LENGTH:
			gSim.problem.sourceLength = u;
			gReadState = READSTATE_NONE;
			break;

		case READSTATE_SOURCE_SEEDS:
			XAssert(gSim.problem.numSourceSeeds < MAX_SOURCE_SEEDS);
			gSim.problem.sourceSeeds[gSim.problem.numSourceSeeds] = u;
			gSim.problem.numSourceSeeds++;
			break;

		default: XAssert(0);
		}
		return true;
	}
    bool Int64(int64_t i) { /*cout << "Int64(" << i << ")" << endl;*/ return true; }
    bool Uint64(uint64_t u) { /*cout << "Uint64(" << u << ")" << endl;*/ return true; }
    bool Double(double d) { /*cout << "Double(" << d << ")" << endl;*/ return true; }
    bool String(const char* str, SizeType length, bool copy) { 
        /*cout << "String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;*/
        return true;
    }
    bool StartObject()
	{ 
		return true; 
	}
    bool Key(const char* str, SizeType length, bool copy) 
	{
		if (_stricmp(str, "id") == 0)
		{
			gReadState = READSTATE_ID;
		}
		else if (_stricmp(str, "units") == 0)
		{
			gReadState = READSTATE_UNITS;
		}
		else if (_stricmp(str, "width") == 0)
		{
			gReadState = READSTATE_WIDTH;
		}
		else if (_stricmp(str, "height") == 0)
		{
			gReadState = READSTATE_HEIGHT;
		}
		else if (_stricmp(str, "filled") == 0)
		{
			gReadState = READSTATE_FILLED;
		}
		else if (_stricmp(str, "sourceLength") == 0)
		{
			gReadState = READSTATE_SOURCE_LENGTH;
		}
		else if (_stricmp(str, "sourceSeeds") == 0)
		{
			gReadState = READSTATE_SOURCE_SEEDS;
		}
		else if (_stricmp(str, "members") == 0)
		{
			XAssert(gReadState == READSTATE_UNITS);
			gReadState = READSTATE_MEMBERS;
		}
		else if (_stricmp(str, "pivot") == 0)
		{
			XAssert(gReadState == READSTATE_UNITS);
			gReadState = READSTATE_PIVOT;
		}
		else if (_stricmp(str, "x") == 0)
		{
			if (gReadState == READSTATE_MEMBERS)
			{
				gReadState = READSTATE_MEMBERS_X;
			}
			else if (gReadState == READSTATE_PIVOT)
			{
				gReadState = READSTATE_PIVOT_X;
			}
			else if (gReadState == READSTATE_FILLED)
			{
				gReadState = READSTATE_FILLED_X;
			}
			else
			{
				XAssert(0);
			}
		}
		else if (_stricmp(str, "y") == 0)
		{
			if (gReadState == READSTATE_MEMBERS)
			{
				gReadState = READSTATE_MEMBERS_Y;
			}
			else if (gReadState == READSTATE_PIVOT)
			{
				gReadState = READSTATE_PIVOT_Y;
			}
			else if (gReadState == READSTATE_FILLED)
			{
				gReadState = READSTATE_FILLED_Y;
			}
			else
			{
				XAssert(0);
			}
		}
		else
		{
			XAssert(0);
		}
        /*cout << "Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;*/
        return true;
    }
    bool EndObject(SizeType memberCount)
	{ 
		if (gReadState == READSTATE_PIVOT_X || 
			gReadState == READSTATE_PIVOT_Y)
		{
			gReadState = READSTATE_PIVOT;
		}
		else if (gReadState == READSTATE_FILLED_X ||
				 gReadState == READSTATE_FILLED_Y)
		{
			gReadState = READSTATE_FILLED;
		}
		else if (gReadState == READSTATE_PIVOT)
		{
			gReadState = READSTATE_UNITS;
		}
		else if (gReadState == READSTATE_MEMBERS_X || 
				 gReadState == READSTATE_MEMBERS_Y)
		{
			gReadState = READSTATE_MEMBERS;
		}
		else if (gReadState == READSTATE_MEMBERS)
		{
			Unit& unit = gSim.problem.units[gSim.problem.numUnits];
			unit.numMembers++;
		}
		else if (gReadState == READSTATE_UNITS)
		{
			gSim.problem.numUnits++;
		}
		else if (gReadState == READSTATE_FILLED)
		{
			gSim.problem.numFilled++;
		}
		else if (gReadState == READSTATE_NONE)
		{
			// ok
		}
		else
		{
			XAssert(0);
		}
		return true;
	}
    bool StartArray() 
	{
		return true; 
	}
    bool EndArray(SizeType elementCount) 
	{ 
		if (gReadState == READSTATE_SOURCE_SEEDS)
		{
			gReadState = READSTATE_NONE;
		}
		else if (gReadState == READSTATE_FILLED)
		{
			gReadState = READSTATE_NONE;
		}
		else if (gReadState == READSTATE_MEMBERS)
		{
			gReadState = READSTATE_UNITS;
		}
		else if (gReadState == READSTATE_UNITS)
		{
			gReadState = READSTATE_NONE;
		}
		else
		{
			XAssert(0);
		}
		return true; 
	}
};

void HoneyLoadProblem(const char* filename)
{

    //const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
	memset(&gSim.problem, 0x00, sizeof(gSim.problem));

	std::ifstream json_file(filename);
	std::string json((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());

	if (json.length() == 0)
	{
		XTrace("failed to open file '%s'", filename);
		printf("failed to open file '%s'", filename);
	}

    MyHandler handler;
    Reader reader;
    StringStream ss(json.c_str());
    ParseResult result = reader.Parse(ss, handler);

	XAssert(!result.IsError());

	HoneyCacheProblemInfo();
}
