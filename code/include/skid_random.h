#ifndef __SKID_RANDOM__
#define __SKID_RANDOM__

/*
 *  Description:
 *      Randomize a number between 1 and stop, inclusive.
 *
 *  Args:
 *      stop: End of the range.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      On success, a number between 1 and stop, inclusive, is returned.  On error, 0 is
 *		returned and errnum is set.  ERANGE is used to indicate stop is an invalid value
 *		(e.g., 0, 1).
 */
unsigned int randomize_number(unsigned int stop, int *errnum);

/*
 *  Description:
 *      Randomize a number between 1 and stop, inclusive.
 *
 *  Args:
 *		start: The beginning of the range.
 *      stop: End of the range.
 *      errnum: [Out] Storage location for errno values encountered.
 *
 *  Returns:
 *      On success, a number between 1 and stop, inclusive, is returned.  On error, 0 is
 *		returned and errnum is set.  ERANGE is used to indicate that start and/or stop are
 *		invalid values (e.g., start > stop, start == stop).
 */
unsigned int randomize_range(unsigned int start, unsigned int stop, int *errnum);

#endif  /* __SKID_RANDOM__ */
