/*
 *	Manually test devops_code create_path_tree() function.
 *	It's a complicated feature that needs to be verified before it's used to unit test and
 *	manually test skid_dir_operations functions.
 *	The commands below will create a unique directory in /tmp populated with files and
 *	sub-directories as indicated.
 *
 *	Copy/paste the following...

# Devops Code create_path_tree() test 1 - 0 files 0 sub-dirs 0 depth
./code/dist/test_devops_code_create_path_tree.bin "/tmp/test_devops_code-c_p_t_1-$(date +"%Y%m%d_%H%M%S")" 0 0 0
# Devops Code create_path_tree() test 2 - 1 file 0 sub-dirs 0 depth
./code/dist/test_devops_code_create_path_tree.bin "/tmp/test_devops_code-c_p_t_2-$(date +"%Y%m%d_%H%M%S")" 1 0 0
# Devops Code create_path_tree() test 3 - 1 file 1 sub-dir 1 deep
./code/dist/test_devops_code_create_path_tree.bin "/tmp/test_devops_code-c_p_t_3-$(date +"%Y%m%d_%H%M%S")" 1 1 1
# Devops Code create_path_tree() test 4 - 2 files 2 sub-dirs 2 levels deep
./code/dist/test_devops_code_create_path_tree.bin "/tmp/test_devops_code-c_p_t_4-$(date +"%Y%m%d_%H%M%S")" 2 2 2

 *
 */

#define SKID_DEBUG			// Enable DEBUG logging

#include <errno.h>			// EINVAL
#include <inttypes.h>		// strtoumax()
#include <stdio.h>			// fprintf()
#include <stdlib.h>			// exit()
#include "devops_code.h"	// create_path_tree(), free_path_tree()
#include "skid_debug.h"		// FPRINTF_ERR(), PRINT_ERRNO(), PRINT_ERROR()

/*
 *	Description:
 *		Convert a string to an unsigned integer.
 *
 *	Args:
 *		string: A non-empty string to convert.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Converted value on success.  On error, 0 and errnum is set.
 */
unsigned int convert_str_to_uint(const char *string, int *errnum);

/*
 *	Description:
 *		Print an array of strings in a standardized fashion.
 *
 *	Args:
 *		str_arr: A NULL-terminated array of strings to print.
 *		context: Context about the array of strings (e.g., "Files created by...")
 *
 *	Returns:
 *		0 on success, errno on failure.
 */
int print_str_array(char **str_arr, const char *context);


int main(int argc, char *argv[])
{
	// LOCAL VARIABLES
	int exit_code = 0;            // Store errno and/or results here
	char **tree_arr = NULL;       // Return value of create_path_tree()
	char *dirname = NULL;         // argv[1]; Name of the top level directory to create
	unsigned int num_files = 0;   // argv[2]; Number of files to create in each dir
	unsigned int tree_width = 0;  // argv[3]; Number of directories to create
	unsigned int tree_depth = 0;  // argv[4]; Number of directory levels to create

	// INPUT VALIDATION
	if (argc != 5)
	{
	   fprintf(stderr, "Usage: %s <dir_to_create> <num_files> <tree_width> <tree_depth>\n",
	   	       argv[0]);
	   exit_code = EINVAL;
	}
	else
	{
		// Store dirname
		dirname = argv[1];
	}

	// CREATE PATH TREE
	// Convert num_files
	if (!exit_code)
	{
		num_files = convert_str_to_uint(argv[2], &exit_code);
	}
	// Convert tree_width
	if (!exit_code)
	{
		tree_width = convert_str_to_uint(argv[3], &exit_code);
	}
	// Convert tree_depth
	if (!exit_code)
	{
		tree_depth = convert_str_to_uint(argv[4], &exit_code);
	}
	// Call create_path_tree()
	if (!exit_code)
	{
		tree_arr = create_path_tree(dirname, num_files, tree_width, tree_depth, &exit_code);
		// Let the call know what happened
		if (exit_code)
		{
			PRINT_ERROR(The call to create_path_tree() failed);
			PRINT_ERRNO(exit_code);
			FPRINTF_ERR("Command line arguments received:\n\t%s %s %s %s %s\n",
				        argv[0], argv[1], argv[2], argv[3], argv[4])
			FPRINTF_ERR("Attempted create_path_tree(%s, %u, %u, %u, %p)\n",
				        dirname, num_files, tree_width, tree_depth, &exit_code);
		}
		else
		{
			exit_code = print_str_array(tree_arr, "Paths created by create_path_tree()");
		}
	}

	// CLEANUP
	if (tree_arr)
	{
		exit_code = free_path_tree(&tree_arr);
	}

	// DONE
	exit(exit_code);
}


unsigned int convert_str_to_uint(const char *string, int *errnum)
{
	// LOCAL VARIABLES
	unsigned int converted_val = 0;  // Converted value
	int result = 0;                  // Store errno value

	// INPUT VALIDATION
	if (!string || !(*string) || !errnum)
	{
		result = EINVAL;  // Bad input
	}

	// CONVERT IT
	if (!result)
	{
		converted_val = strtoumax(string, NULL, 10);
		if (UINTMAX_MAX == converted_val)
		{
			result = errno;  // ERANGE or valid?
		}
	}

	// DONE
	if (result)
	{
		converted_val = 0;
	}
	if (errnum)
	{
		*errnum = result;
	}
	return converted_val;
}


int print_str_array(char **str_arr, const char *context)
{
	// LOCAL VARIABLES
	char **arr_ptr = str_arr;  // Iterating pointer
	int result = 0;            // Store errno value

	// INPUT VALIDATION
	if (NULL == arr_ptr)
	{
		result = EINVAL;  // NULL pointer
	}

	// PRINT IT
	if (!result)
	{
		printf("%s:\n", context);
		do
		{
			printf("\t%s\n", *arr_ptr);
			if (*arr_ptr)
			{
				arr_ptr++;  // Next string
			}
		}
		while (NULL != *arr_ptr);
	}

	// DONE
	return result;
}
