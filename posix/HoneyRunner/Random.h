// Random.h

#ifndef RANDOMH
#define RANDOMH

#define MTWISTER_N 624

struct Random
{
	Random();

	uint32 mt[MTWISTER_N];
	uint32 mti;
};

extern Random gRNG;

uint32 RandomNext(Random &rng);
void RandomSeed(Random &rng, uint32 seed);
int RandomInt(Random& rng, int maxPlusOne);
int RandomIntRange(Random& rng, int min, int max);
float RandomUnit(Random& rng);
float RandomFloat(Random& rng, float limit);
float RandomRange(Random& rng, float min, float max);

inline bool RandomPercentChance(int percent)	{ return (RandomIntRange(gRNG, 0, 99) < percent); }
inline int RandomIntRange(int min, int max)		{ return RandomIntRange(gRNG, min, max); }
inline int RandomInt(int maxPlusOne)			{ return RandomInt(gRNG, maxPlusOne); }
inline float RandomUnit()						{ return RandomUnit(gRNG); }
inline float RandomFloat(float limit)			{ return RandomFloat(gRNG, limit); }
inline float RandomRange(float min, float max)	{ return RandomRange(gRNG, min, max); }

void ShuffleInts(int *data, int count);

struct RandomLC
{
	int seed;
};

RandomLC RandomLCSet(int seed);
uint32 RandomLCNext(RandomLC &rng);
float RandomLCUnit(RandomLC& rng);

struct RandomPCG
{
	uint64 state;
	uint64 inc;
};

RandomPCG RandomPCGSet(uint64 seed, uint64 sequence);


#endif // RANDOMH