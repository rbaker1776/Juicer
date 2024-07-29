#ifndef JUICER_H_78A29C6E15DC
#define JUICER_H_78A29C6E15DC


#if defined(__GUNC__) || defined(__clang__)
	#define forceinline __attribute__((always_inline)) inline
	#define noinline __attribute__((noinline))
#elif defined(_MSC_VER)
	#define forceinline __forceinline
	#define noinline __declspec(noinline)
#else
	#define forceinline inline
	#define noinline
#endif


#if defined(__GNUC__) || defined(__clang__)
	#define restrict __restrict__
#elif defined (_MSC_VER)
	#define restrict _restrict
#else
	#define restrict
#endif


#if defined(__GUNC__) || defined(__clang__)
	#define pure __attribute__((pure))
#elif defined(_MSC_VER)
	#define pure __declspec(noalias) // not quite the name as pure but similar
#else
	#define pure
#endif


#if defined(__GNUC__) || defined(__clang__)
	#define constfunc __attribute__((const))
#else
	#define constfunc
#endif


#define MANUAL 0
#define BUILTIN 1

#define POPCOUNT BUILTIN
#define LSB BUILTIN

#define DEBUG false
#define VERBOSE true

#define VERSION 1


#endif // JUICER_H_78A29C6E15DC
