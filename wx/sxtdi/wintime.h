/*  Buddhabrot
    https://github.com/Michaelangel007/buddhabrot
    http://en.wikipedia.org/wiki/User_talk:Michael.Pohoreski/Buddhabrot.cpp

    Optimized and cleaned up version by Michael Pohoreski
    Based on the original version by Evercat

        g++ -Wall -O2 buddhabrot.cpp -o buddhabrot
        buddhabrot 4000 3000 20000

   Released under the GNU Free Documentation License
   or the GNU Public License, whichever you prefer.
*/
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h> // Windows.h -> WinDef.h defines min() max()

/*
	typedef uint16_t WORD ;
	typedef uint32_t DWORD;

	typedef struct _FILETIME {
		DWORD dwLowDateTime;
		DWORD dwHighDateTime;
	} FILETIME;

	typedef struct _SYSTEMTIME {
		  WORD wYear;
		  WORD wMonth;
		  WORD wDayOfWeek;
		  WORD wDay;
		  WORD wHour;
		  WORD wMinute;
		  WORD wSecond;
		  WORD wMilliseconds;
	} SYSTEMTIME, *PSYSTEMTIME;

// *sigh* Microsoft has this in winsock2.h because they are too lazy to put it in the standard location ... !?!?
typedef struct timeval {
	long tv_sec;
	long tv_usec;
} timeval;
*/

// *sigh* no gettimeofday on Win32/Win64
int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	// FILETIME Jan 1 1970 00:00:00
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL); 

	SYSTEMTIME  nSystemTime;
	FILETIME    nFileTime;
	uint64_t    nTime;

	GetSystemTime( &nSystemTime );
	SystemTimeToFileTime( &nSystemTime, &nFileTime );
	nTime =  ((uint64_t)nFileTime.dwLowDateTime )      ;
	nTime += ((uint64_t)nFileTime.dwHighDateTime) << 32;

	tp->tv_sec  = (long) ((nTime - EPOCH) / 10000000L);
	tp->tv_usec = (long) (nSystemTime.wMilliseconds * 1000);
	return 0;
}
