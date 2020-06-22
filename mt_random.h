#ifndef _mt_random_h
#define _mt_random_h

#include <vector>
#include "stdint.h"

namespace hearts {

/* Period parameters */  
#define NN_SIZE 624
//#define MM 397
//#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
//#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
//#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

class mt_random {
 public:
	mt_random() :mti(NN+1) { /*mt.resize(NN);*/ } /* mti==N+1 means mt[N] is not initialized */
	mt_random(uint32_t seed)
	{ /*mt.resize(NN);*/ srand(seed); }
	void srand(uint32_t);
	void lsrand(uint32_t[], int);
	uint32_t rand_long();
	double rand_double();
	uint32_t ranged_long(uint32_t minimum, uint32_t maximum);
 private:
	static const int NN;
	static const int MM;
	static const uint32_t MATRIX_A;
	static const uint32_t UPPER_MASK;
	static const uint32_t LOWER_MASK;

	uint32_t mt[NN_SIZE];
	//std::vector<uint32_t> mt; /* the array for the state vector  */
	int mti;
};

} // namespace hearts

#endif
