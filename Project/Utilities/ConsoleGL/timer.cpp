#include "timer.h"
#include "glutils.h"

#ifdef WIN32
	#include <time.h>
#else
	#include <sys/time.h>
	#include <stdlib.h>
#endif

// Constructor
Timer::Timer(void)
{
	frames = 0;
	frametime = 0;
	fTime = 0.0f;

	// Randomize timer
	srand((unsigned)time(NULL));

	#ifdef WIN32
		if (!QueryPerformanceFrequency((LARGE_INTEGER *) &tFrequency))
		{
			// Aucun compteur de performances disponible
			// Il faut utiliser W98 OSR1
			tResolution = 0.001f;
			tPerformanceTimerEnabled = 0;
			//exit(-1);	// I usually exit in this situation
		}
		else
	#endif
	{ 
		// Compteur de performances disponibles
		// On va utiliser celui-ci plutôt qu'un compteur multimédia
		tPerformanceTimerEnabled = 1;

		// Détermine le temps sur 32 bits (au lieu des 64 de l'horloge)
		// pour avoir une précision de l'ordre de la microseconde
		unsigned int lowpart = (unsigned int)tFrequency.LowPart;
		unsigned int highpart = (unsigned int)tFrequency.HighPart;
		lowshift = 0;

		while (highpart || (lowpart > 2000000.0))
		{
			++lowshift;
			lowpart >>= 1;
			lowpart |= (highpart & 1) << 31;
			highpart >>= 1;
		}
		tResolution = 1.0/(double)lowpart;
	}
}

Timer::~Timer(void)
{
}

// Remise à jour des timers et compteurs
// (appelé à chaque image)
int Timer::Refresh()
{
	#ifdef WIN32
		LARGE_INTEGER	count;
		unsigned int	temp, t2;
		if (tPerformanceTimerEnabled)
		{
			// On utilise le compteur de performances
			QueryPerformanceCounter((LARGE_INTEGER *) &count);

			temp = ((unsigned int) count.LowPart >> lowshift) |
				((unsigned int) count.HighPart << (32 - lowshift));

			// Détermine si on est à la 1e image,
			// ou si le temps revient en arrière (sait-on jamais !)
			if (frames == 0 || ((temp <= oldTime) && ((oldTime - temp) < 0x10000000)))
			{
				oldTime = temp;
			}
			else
			{
				t2 = temp - oldTime;
				frametime = (double) t2 * tResolution;
				fTime += frametime;

				oldTime = temp;

				if (fTime == flastTime)
				{
					++sametimecount;

					if (sametimecount > 100000)
					{
						fTime += 1.0;
						sametimecount = 0;
					}
				}
				else sametimecount = 0;

				flastTime = fTime;
			}
			++frames;
		}
		else
	#endif
	{
		// On y va à la main....
		if (frames == 0)
		{
			flastTime = fTime = GetTime() * tResolution;
			frametime = 0;
		}
		else
		{
			flastTime = fTime;
			fTime = GetTime() * tResolution;
			frametime = fTime - flastTime;
		}
	}

	return ++frames;
}
