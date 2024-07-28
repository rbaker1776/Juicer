#ifndef JUICER_H_78A29C6E15DC
#define JUICER_H_78A29C6E15DC


#if defined(__GUNC__) || defined(__clang__)
	#define force_inline __attribute__((always_inline)) inline
	#define no_inline __attribute__((noinline))
#elif defined(_MSC_VER)
	#define force_inline __forceinline
	#define no_inline __declspec(noinline)
#else
	#define force_inline inline
	#define no_inline
#endif


#if defined(__GNUC__) || defined(__clang__)
	#define restrict __restrict__
#elif defined (_MSC_VER)
	#define restrict _restrict
else
	#define restrict
#endif

#define MANUAL 0
#define BUILTIN 1

#define POPCOUNT BUILTIN
#define LSB BUILTIN

#define DEBUG false
#define VERBOSE true

#define VERSION 1


#endif // JUICER_H_78A29C6E15DC
