#ifndef QSORT_H
#define QSORT_H
/* ------------------- */
/* FUNCTION PROTOTYPES */
/* ------------------- */
# undef F
# if defined(__STDC__) || defined(__PROTO__)
#	define	F( P )	P
# else
#	define	F( P )	()
# endif


void	qsort F((void *, size_t, size_t,
			int (*)(const void *, const void *)));
			
#endif QSORT_H
