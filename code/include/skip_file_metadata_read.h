#ifndef __SKIP_FILE_METADATA_READ__
#define __SKIP_FILE_METADATA_READ__

#include <errno.h>      // errno
#include <stdbool.h>    // bool, false, true
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
time_t get_access_time(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
blkcnt_t get_block_count(const char *filename, int *errnum);

/*
 *  Description:
 *		Extracts the st_blksize field from the stat struct which gives the "preferred" blocksize
 *		for efficient file system I/O.
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *      
 *  Returns:
 *      On success, the st_blksize value for filename.  On failure, this function
 *		returns 0 and sets errnum with an errno value.
 */
blksize_t get_block_size(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
time_t get_change_time(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
dev_t get_container_device_id(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
dev_t get_file_device_id(const char *filename, int *errnum);

/*
 *  Description:
 *		Use library macros to extract just the permission values from the stat struct
 *		st_mode member.  This function uses stat() so the file type of a symbolic link will
 *		report back as the file it is linked to.  If positively identifying symbolic links is
 *		important, use is_sym_link() and lstat() instead.
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *      
 *  Returns:
 *		Just the file permission bits from filename's mode on success.  Returns 0 on failure
 *		and updates errnum.
 */
mode_t get_file_perms(const char *filename, int *errnum);

/*
 *  Description:
 *		Uses the file type bit field mask to extract the file type bits from the stat struct
 *		st_mode member.  This function uses stat() so the file type of a symbolic link will
 *		report back as the file it is linked to.  If positively identifying symbolic links is
 *		important, use is_sym_link().
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      On success, a value comparable to any of the file type mask values:
 *		S_IFBLK, S_IFCHR, S_IFDIR, S_IFIFO, S_IFREG, S_IFSOCK, S_IFLNK.  On failure, this function
 *		returns 0 and sets errnum with an errno value.
 */
mode_t get_file_type(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
gid_t get_group(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
nlink_t get_hard_link_num(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
time_t get_mod_time(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
uid_t get_owner(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
ino_t get_serial_num(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
off_t get_size(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a block device?".  Updates errnum with errno values.
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      True if filename is a block device.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_block_device(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a character device?".  Updates errnum with errno values.
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      True if filename is a character device.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_character_device(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is pathname a directory?".  Updates errnum with errno values.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      True if pathname is a directory.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_directory(const char *pathname, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a named pipe?".  Updates errnum with errno values.
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      True if filename is a named pipe.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_named_pipe(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a regular file?".  Updates errnum with errno values.
 *		Calls stat(filename), reads the mode field from the stat struct, and checks it against
 *		the "regular file" macro.  This function may return true for symbolic links because
 *		stat() follows symbolic links.  If that is a problem, also call is_sym_link().
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      True if filename is a regular file.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_regular_file(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a socket?".  Updates errnum with errno values.
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      True if filename is a socket.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_socket(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a symbolic link?".  Updates errnum with errno values.
 *		Calls lstat(filename), reads the mode field from the stat struct, and checks it against
 *		the "symbolic link" macro.
 *
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *      True if filename is a symbolic link.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_sym_link(const char *filename, int *errnum);

#endif  /* __SKIP_FILE_METADATA_READ__ */
