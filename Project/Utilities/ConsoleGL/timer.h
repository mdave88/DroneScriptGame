#ifndef __TIMER_H__
#define __TIMER_H__

#ifndef WIN32
	typedef struct			/**< Large integer structure. */
	{
		unsigned short LowPart;
		long HighPart;
	} LARGE_INTEGER;
#else
	#include <wtypes.h>
#endif

/**
 * Timer class.
 * The timer is used in every operations which has connections with time.
 * It is used for animations, fps, etc.
 */
class Timer
{
	public:
		Timer(void);		/**< Default constructor. */
		~Timer(void);		/**< Default destructor. */

		/**
		 * Timer initialization. The timer must be initialized manually by
		 * calling this function.
		 */
		void Init(void);

		/**
		 * Refresh the timer. When this function is called, the timer gets
		 * the current time and store new values into public class variables.
		 * @return the number of calls of the function since the timer has
		 *         been created (if the engine calls this function each frames,
		 *         the returned value is the number of frames since the
		 *         beginning of the main loop)
		 */
		int Refresh(void);

		double			fTime;			/**< Time since windows started. */
		double			flastTime;		/**< Last recorded time. */
		double			frametime;		/**< Time elapsed in the last frame. */
		int				frames;			/**< Number of frames. */

		LARGE_INTEGER	tFrequency;
		double			tResolution;
		
	private:
		int				lowshift;
		unsigned char	tPerformanceTimerEnabled;

		unsigned int	oldTime;
		int				sametimecount;	/**< Counter for frames with the same time. */
};

#endif	/* __TIMER_H__ */