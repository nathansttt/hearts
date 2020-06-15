// $Id: fpUtil.h 522 2009-09-22 22:48:07Z nathanst $

#include <stdint.h>
#include <assert.h>
#include <string.h>
#ifndef UINT32_MAX
#define UINT32_MAX             (4294967295U)
#endif
#ifndef UINT_MAX
#define UINT_MAX             (4294967295U)
#endif

#ifndef fpUtil_H
#define fpUtil_H

//// Somehow DBL_MAX is not defined under Linux?
//#ifndef OS_MAC
//static const double DBL_MAX = 1.79769313486231500e+308;    // DBL_MAX for non Mac OS
//#endif

// Floating point comparisons 
static const double TOLERANCE_D = 0.000001;    // floating point tolerance
static const float  TOLERANCE_F = 0.00005;    // floating point tolerance

inline bool fless(double a, double b) { return (a < b - TOLERANCE_D); }
inline bool fgreater(double a, double b) { return (a > b + TOLERANCE_D); }
inline bool fequal(double a, double b)
{ return (a > b - TOLERANCE_D) && (a < b+TOLERANCE_D); }

inline bool fless(float a, float b) { return (a < b - TOLERANCE_F); }
inline bool fgreater(float a, float b) { return (a > b + TOLERANCE_F); }
inline bool fequal(float a, float b)
{ return (a > b - TOLERANCE_F) && (a < b+TOLERANCE_F); }


#endif
