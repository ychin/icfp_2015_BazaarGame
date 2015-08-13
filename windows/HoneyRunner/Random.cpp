// Random.cpp

#include "Precompile.h"
#include "Random.h"

Random gRNG;

// this is a mesenne twister RNG

/* Period parameters */  
#define MTWISTER_M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

Random::Random() 
{
	mti = MTWISTER_N + 1;
}

void RandomSeed(Random &rng, uint32 seed)
{
	rng.mt[0]= seed & 0xffffffffUL;
	for (rng.mti=1; rng.mti<MTWISTER_N; rng.mti++) {
		rng.mt[rng.mti] = 
			(1812433253UL * (rng.mt[rng.mti-1] ^ (rng.mt[rng.mti-1] >> 30)) + rng.mti); 
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		rng.mt[rng.mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}

/* generates a random number on [0,0xffffffff]-interval */
uint32 RandomNext(Random &rng)
{
	unsigned int y;
	static unsigned int mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (rng.mti >= MTWISTER_N) { /* generate MTWISTER_N words at one time */
		int kk;

		if (rng.mti == MTWISTER_N+1)   /* if init_genrand() has not been called, */
			RandomSeed(rng, 5489UL); /* a default initial seed is used */

		for (kk=0;kk<MTWISTER_N-MTWISTER_M;kk++) {
			y = (rng.mt[kk]&UPPER_MASK)|(rng.mt[kk+1]&LOWER_MASK);
			rng.mt[kk] = rng.mt[kk+MTWISTER_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<MTWISTER_N-1;kk++) {
			y = (rng.mt[kk]&UPPER_MASK)|(rng.mt[kk+1]&LOWER_MASK);
			rng.mt[kk] = rng.mt[kk+(MTWISTER_M-MTWISTER_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (rng.mt[MTWISTER_N-1]&UPPER_MASK)|(rng.mt[0]&LOWER_MASK);
		rng.mt[MTWISTER_N-1] = rng.mt[MTWISTER_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		rng.mti = 0;
	}

	y = rng.mt[rng.mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

int RandomInt(
	Random& rng, 
	int maxPlusOne)
{
	XAssert(maxPlusOne > 0);
	return RandomNext(rng) % maxPlusOne;
}

int RandomIntRange(
	Random& rng, 
	int min, 
	int max)
{
	XAssert(min <= max);
	return min + RandomInt(rng, max - min + 1);
}

float RandomUnit(
	Random& rng)
{	
	int retval = (RandomNext(rng) & 0x007FFFFF) | 0x3f800000;
	return *(float *)&retval - 1.0f;
};

float RandomFloat(
	Random& rng, 
	float limit)
{
	return RandomUnit(rng) * limit;
};

float RandomRange(
	Random& rng, 
	float min,
	float max)
{
	return RandomUnit(rng) * (max - min) + min;
};

//////////////////////
//////////////////////

RandomLC RandomLCSet(int seed)
{
	RandomLC retval;
	retval.seed = seed;
	return retval;
}

uint32 RandomLCNext(RandomLC &rng)
{
	//int m = 0xFFFFFFFF;
	int a = 1103515245;
	int c = 12345;
	rng.seed = (a * rng.seed + c);
	return rng.seed;
}

float RandomLCUnit(RandomLC& rng)
{	
	int retval = (RandomLCNext(rng) & 0x007FFFFF) | 0x3f800000;
	return *(float *)&retval - 1.0f;
};


//////////////////////////
//////////////////////////

uint32 RandomPCGNext(RandomPCG &rng)
{
	uint64 oldstate = rng.state;
	rng.state = oldstate * 6364136223846793005ULL + rng.inc;
	uint32 xorshifted = uint32(((oldstate >> 18u) ^ oldstate) >> 27u);
	uint32 rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((0-rot) & 31));
}

float RandomPCGUnit(RandomPCG& rng)
{	
	int retval = (RandomPCGNext(rng) & 0x007FFFFF) | 0x3f800000;
	return *(float *)&retval - 1.0f;
};

RandomPCG RandomPCGSet(uint64 seed, uint64 sequence)
{
	RandomPCG rng = {};
	rng.state = 0;
	rng.inc = (sequence << 1) | 1;
	RandomPCGNext(rng);
	rng.state = seed;
	RandomPCGNext(rng);
	return rng;
}

//////////////////////////
//////////////////////////

template <typename T>
void Shuffle(T *data, int count) 
{
	for(int i = 0; i < count - 1; ++i)
	{
		int idx = RandomIntRange(i, count - 1);
		T temp = data[i];
		data[i] = data[idx];
		data[idx] = temp;
	}
}

void ShuffleInts(int *data, int count)
{
	Shuffle(data, count);
}


