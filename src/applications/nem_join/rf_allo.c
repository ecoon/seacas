#include <stdlib.h>
#include <stdio.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif


/*#include "rf_allo.h"*/

#ifdef DEBUG
    extern int Proc;
#endif

   static double *smalloc (int n, char *filename, int lineno);

/******************************************************************************
 *
 *                    Dynamic Allocation of Multidimensional Arrays
 *-----------------------------------------------------------------------------
 *
 * Example Usage:
 *
 *     typedef	struct
 *       {	int	bus1;
 *              int	bus2;
 *              int	dest;
 *      }       POINT;
 *
 *      POINT    **points, corner;
 *
 *      points = (POINT **) array_alloc (2, x, y, sizeof(POINT));
 *                               ^ ^ ^
 *                               | | |
 *         number of dimensions--+ | |
 *                                 | |
 *          first dimension max----+ |
 *                                   |
 *         second dimension max------+
 *
 *         (points may be now be used as if it were declared
 *          POINT points[x][y])
 *
 *          This particular version is limited to dimensions of 3 or less.
 *
 *      corner = points[2][3]; (refer to the structure as you would any array)
 *
 *      free (points); (frees the entire structure in one fell swoop)
 *
 *****************************************************************************/
/******************************************************************************
* 	The following section is a commented section containing
*	an example main code:
*******************************************************************************
*double *array_alloc();
*main()
*{
*  int ***temp;
*   int *temp2;
*   int i, j, k;
*   int il, jl, kl;
*
*   malloc_debug(2);
*   il = 2;
*   jl = 3;
*   kl = 3;
*   temp = (int ***) array_alloc(3,il,jl,kl,sizeof(int));
*   for (i=0; i<il; i++) {
*      for (j=0; j<jl; j++) {
*         for (k=0; k<kl; k++) temp[i][j][k] = 1;
*      }
*   }
*
*   temp2 = (int *) malloc(10*sizeof(int));
*   for (i=0; i<10; i++) temp2[i] = 0;
*
*   for (i=0; i<il; i++) {
*      for (j=0; j<jl; j++) {
*         for (k=0; k<kl; k++) (void) printf(" %d\n", temp[i][j][k]);
*      }
*   }
*   malloc_verify();
*}
******************************************************************************/

/*****************************************************************************/
/*****************************************************************************/
#ifdef __STDC__

double *array_alloc (char *file, int lineno, int numdim, ...)

#else

double *
array_alloc (va_alist)
va_dcl

#endif
/*****************************************************************************/

{
   char *yo = "array_alloc";
   int	i, j;
   struct dim {
      long	index;	/* Number of elements in the dimension	*/
      long	total;	/* Total number of elements 		*/
      long	size;	/* Size of a single element in bytes	*/
      long	off;	/* offset from beginning of array	*/
   }	dim[3];		/* Info about each dimension 		*/
#ifndef __STDC__
   char *file;		/* Filename of source code from call.   */
   int lineno;		/* Line number of call.                 */
   int numdim;		/* Number of dimensions			*/
#endif
   long total;		/* Total size of the array		*/
   double *dfield;	/* ptr to avoid lint complaints		*/
   char *field;		/* The multi-dimensional array		*/
   char **ptr;		/* Pointer offset			*/
   char *data;		/* Data offset				*/
   va_list va;		/* Current pointer in the argument list	*/

#ifdef __STDC__
   va_start(va, numdim);
#else
   va_start(va);
   file = va_arg(va, char *);
   lineno = va_arg(va, int);
   numdim = va_arg(va, int);
#endif

   if (numdim <= 0) {
     fprintf (stderr, "%s (%s: %d) ERROR: number of dimensions, %d, is <=0\n",
              yo, file, lineno, numdim);
	exit(1);
   } else if (numdim > 3) {
     fprintf (stderr, "%s (%s: %d) ERROR: number of dimensions, %d, is > 3\n",
              yo, file, lineno,  numdim);
	exit(1);
   }

   dim[0].index = va_arg(va, int);


   if (dim[0].index <= 0) {
#ifdef DEBUG
    fprintf(stderr, "WARNING, %s (%s: %d) called with first "
            "dimension <= 0, %d; will return NULL\n",
            yo, file, lineno, dim[0].index);
#endif
      return((double *) NULL);
   }

   dim[0].total = dim[0].index;
   dim[0].size = sizeof(void *);
   dim[0].off = 0;
   for (i=1; i<numdim; i++) {
      dim[i].index = va_arg(va, int);
      if (dim[i].index <= 0) {
         fprintf(stderr, "WARNING: %s (%s: %d) called with dimension %d <= 0, "
                 "%ld; will return NULL\n",
                 yo, file, lineno, i+1, dim[i].index);
	 return((double *) NULL);
      }
      dim[i].total = dim[i-1].total * dim[i].index;
      dim[i].size = sizeof(void *);
      dim[i].off = dim[i-1].off + dim[i-1].total * dim[i-1].size;
   }
   dim[numdim-1].size = va_arg(va, int);
   va_end(va);

   /* Round up the last offset value so data is properly aligned. */
   dim[numdim-1].off = dim[numdim-1].size *
       ((dim[numdim-1].off+dim[numdim-1].size-1)/dim[numdim-1].size);

   total = dim[numdim-1].off + dim[numdim-1].total * dim[numdim-1].size;

   dfield = (double *) smalloc((int) total, file, lineno);
   field = (char *) dfield;

   for (i=0; i<numdim - 1; i++) {
      ptr = (char **) (field + dim[i].off);
      data = (char *) (field + dim[i+1].off);
      for (j=0; j<dim[i].total; j++) {
         ptr[j] = data + j * dim[i+1].size * dim[i+1].index;
      }
   }

   return(dfield);
}

/* Safe version of malloc.  Does not initialize memory .*/

/* Modified by Scott Hutchinson (1421) 20 January 1993 */

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

static double *smalloc (int n, char *filename, int lineno)

{
  char *yo = "smalloc";
  double *pntr;           /* return value */

  if (n < 0) {
    fprintf(stderr, "%s (from %s,%d) ERROR: Non-positive argument. (%d)\n",
            yo, filename, lineno, n);
    exit(-1);
  }
  else if (n == 0)
    pntr = NULL;
  else
    pntr = (double *) malloc((unsigned) n);

  if (pntr == NULL && n != 0) {
    fprintf(stderr, "%s (from %s,%d) Out of space - number of bytes "
            "requested = %d\n", yo, filename, lineno, n);
    exit(0);
  }

  return pntr;

}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

void safe_free (void **ptr)
{
/*
 *  This version of free calls the system's free function
 *  with maximum error checking. It also doesn't call free if ptr is
 *  the NULL pointer.
 */

  if (*ptr != NULL) {

    free(*ptr);

    /*
     *  Set the value of ptr to NULL, so that further references
     *  to it will be flagged.
     */

    *ptr = NULL;
  }
}  /* safe_free */

/*****************************************************************************/
/*                       END of rf_allo.c                                    */
/*****************************************************************************/