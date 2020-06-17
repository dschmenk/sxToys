#ifdef _MSC_VER
#if _MSC_VER < 1700
typedef unsigned char  uint8_t;
typedef signed   char  int8_t;
typedef unsigned short uint16_t;
typedef signed   short int16_t;
#endif
#include <windows.h>
#define ENABLE_HIGH_RES_TIMER() timeBeginPeriod(1)
#define DISABLE_HIGH_RES_TIMER() timeEndPeriod(1)
#else
#include <sys/time.h>
#define ENABLE_HIGH_RES_TIMER()
#define DISABLE_HIGH_RES_TIMER()
#endif
#include "sxutil.h"
#include "aip.h"
#include "fits.h"
