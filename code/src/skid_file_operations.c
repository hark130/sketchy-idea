/*
 *	This library defines functionality to create, delete, and empty Linux files.
 */

#define SKID_DEBUG					// Enable DEBUGGING output

#include "skid_file_operations.h"	// bool, empty_file(), false, true
#ifndef ENOERR
#define ENOERR ((int)0)
#endif  /* ENOERR */


/**************************************************************************************************/
/********************************* PRIVATE FUNCTION DECLARATIONS **********************************/
/**************************************************************************************************/

/*
 *	Description:
 *		Closes *stream, if it's not NULL, using fclose() and sets it to NULL.  This is a
 *		"best effort" function but it will call DEBUG print macros if enabled and fclose() fails.
 *
 *	Args:
 *		stream: A pointer to a file pointer.
 */
void close_stream(FILE **stream);

/*
 *  Description:
 *		Is filename an actual file?  Any invalid input or errno values are treated as a "no".
 *
 *  Args:
 *      filename: Absolute or relative pathname to check.
 *
 *  Returns:
 *      True if filename exists as a file.  False otherwise.
 */
bool is_file(const char *filename);

/*
 *  Description:
 *      Validates the pathname arguments on behalf of this library.
 *
 *  Args:
 *      pathname: A non-NULL pointer to a non-empty string.
 *
 *  Returns:
 *      An errno value indicating the results of validation.  0 on successful validation.
 */
int validate_sfo_pathname(const char *pathname);

/*
 *	Description:
 *		Writes contents to stream and responds to errors.  This function does not close stream.
 *
 *	Args:
 *		contents: The contents to write to stream.
 *		stream: Open FILE pointer to write to.
 *
 *	Returns:
 *		0, on success.  On failure, an errno value (or -1 for an unspecified error).
 */
int write_stream(const char *contents, FILE *stream);

/**************************************************************************************************/
/********************************** PUBLIC FUNCTION DEFINITIONS ***********************************/
/**************************************************************************************************/


int create_file(const char *filename, const char *contents, bool overwrite)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution
	FILE *fp = NULL;      // File pointer to filename

	// INPUT VALIDATION
	result = validate_sfo_pathname(filename);
	if (ENOERR == result)
	{
		if (true == is_file(filename) && false == overwrite)
		{
			result = EEXIST;
		}
	}

	// CREATE IT
	// Open it
	if (ENOERR == result)
	{
		fp = fopen(filename, "w");  // Truncate file to zero length or create text file for writing.
		if (!fp)
		{
			result = errno;
			PRINT_ERROR(The call to fopen() failed);
			PRINT_ERRNO(errnum);
		}
	}
	// Write to it
	if (ENOERR == result)
	{
		if (contents && *contents)
		{
			result = write_stream(contents, fp);
		}
	}

	// DONE
	close_stream(&fp);  // Best effort
	return result;
}


int delete_file(const char *filename)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution

	// INPUT VALIDATION
	result = validate_sfo_pathname(filename);

	// DELETE IT
	if (ENOERR == result)
	{
		if (unlink(filename))
		{
			result = errno;
			PRINT_ERROR(The call to unlink() failed);
			PRINT_ERRNO(errnum);
		}
	}

	// DONE
	return result;
}


int empty_file(const char *filename)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // Results of execution

	// INPUT VALIDATION
	// Let create_file() handle input validation

	// CREATE IT
	result = create_file(filename, NULL, true);

	// DONE
	return result;
}


/**************************************************************************************************/
/********************************** PRIVATE FUNCTION DEFINITIONS **********************************/
/**************************************************************************************************/


void close_stream(FILE **stream)
{
	// LOCAL VARIABLES
	int errnum = ENOERR;      // Store errno values here

	// INPUT VALIDATION
	if (stream && *stream)
	{
		if (fclose(*stream))
		{
			errnum = errno;
			PRINT_ERROR(The call to fclose() failed);
			PRINT_ERRNO(errnum);
		}
		*stream = NULL;  // Succeed or fail, we tried...
	}

	// DONE
	return;
}


bool is_file(const char *filename)
{
	// LOCAL VARIABLES
	bool is_a_file = false;  // Results of execution
	int errnum = ENOERR;     // Store errno values here

	// INPUT VALIDATION
	is_a_file = is_regular_file(filename, &errnum);

	// DONE
	return is_a_file;
}


int validate_sfo_pathname(const char *pathname)
{
	// LOCAL VARIABLES
	int result = ENOERR;  // The results of validation

	// VALIDATE IT
	// pathname
	if (!pathname)
	{
		result = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received a null pathname pointer);
	}
	else if (!(*pathname))
	{
		result = EINVAL;  // Invalid argument
		PRINT_ERROR(Invalid Argument - Received an empty pathname);
	}

	// DONE
	return result;
}


int write_stream(const char *contents, FILE *stream)
{
	// LOCAL VARIABLES
	int result = ENOERR;         // The results of validation
	size_t nmemb = 0;            // Number of characters in contents
	size_t num_items_wrote = 0;  // The number of items written by fwrite()

	// INPUT VALIDATION
	if (!contents || !(*contents) || !stream)
	{
		result = EINVAL;  // NULL pointer or empty buffer
	}
	else
	{
		nmemb = strlen(contents);
	}

	// WRITE IT
	if (ENOERR == result)
	{
		num_items_wrote = fwrite(contents, sizeof(*contents), nmemb, stream);
		result = errno;  // Just in case
		if (0 == num_items_wrote)
		{
			PRINT_ERROR(The call to fwrite() failed);
			PRINT_ERRNO(result);
			result = -1;  // Failure!
		}
		else if (num_items_wrote < nmemb)
		{
			PRINT_WARNG(fwrite() performed a short write);
			PRINT_ERRNO(result);
			result = -1;  // Failure?
		}
		else if (num_items_wrote != nmemb)
		{
			PRINT_WARNG(fwrite() performed an OVER-write?!);
			PRINT_ERRNO(result);
			result = -1;  // Failure?!
		}
	}

	// DONE
	return result;
}
