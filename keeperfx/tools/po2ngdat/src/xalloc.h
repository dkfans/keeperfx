/*
 * xalloc.h
 *
 *  Created on: 30-09-2012
 *      Author: tomasz
 */

#ifndef _XALLOC_H
#define _XALLOC_H

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Allocate an object of type T dynamically, with error checking.  */
/* extern T *XMALLOC (typename T); */
#define XMALLOC(T) \
  ((T *) malloc (sizeof (T)))

# define XNMALLOC(N,T) \
   ((T *) malloc (N*sizeof(T)))

/* extern T *XCALLOC (size_t nmemb, typename T); */
#define XCALLOC(N,T) \
  ((T *) calloc (N, sizeof (T)))

# define xmalloc(N) \
  malloc (N)

# define xmalloca(N) \
  malloc (N)

# define freea(N) \
  free (N)

# define xrealloc(P,N) \
  realloc(P,N)

# define xstrdup(P) \
  strdup(P)

#ifdef __cplusplus
}
#endif


#endif /* _XALLOC_H */
