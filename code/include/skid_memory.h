#ifndef __SKID_MEMORY__
#define __SKID_MEMORY__

/*
 *	Description:
 *		Allocate a zeroized array in heap memory.
 *
 *	Args:
 *		num_elem: The number of elements in the array.
 *		size_elem: The size of each element in the array.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated memory of total size num_elem * size_elem that has been zeroized, on success.
 *		Caller is responsible for freeing the return value with free_skid_mem().
 *		NULL on error (check errnum for details).
 */
void *alloc_skid_mem(size_t num_elem, size_t size_elem, int *errnum);

/*
 *	Description:
 *		Determine the length of source, allocate a zeroized array in heap memory, and copy it.
 *		It is the caller's responsibility to free the return value using free_skid_string().
 *
 *	Args:
 *		source: A string to copy.
 *		errnum: [Out] Storage location for errno values encountered.
 *
 *	Returns:
 *		Heap-allocated memory containing a copy of source on succes.  NULL on error (check
 *		errnum for details).
 */
char *copy_skid_string(const char *source, int *errnum);

/*
 *	Description:
 *		Free skid-allocated heap memory and set the original pointer to NULL.
 *
 *	Args:
 *		old_mem: Pointer to the heap-allocated memory's storage location.
 *
 *	Returns:
 *		0 on success, errno on error.
 */
int free_skid_mem(void **old_mem);

/*
 *	Description:
 *		Free skid-allocated string and set the original pointer to NULL.
 *
 *	Args:
 *		old_string: Pointer to the heap-allocated string's storage location.
 *
 *	Returns:
 *		0 on success, errno on error.
 */
int free_skid_string(char **old_string);

#endif  /* __SKID_MEMORY__ */
