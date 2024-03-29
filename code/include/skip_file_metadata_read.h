#ifndef __SKIP_FILE_METADATA_READ__
#define __SKIP_FILE_METADATA_READ__

#include <errno.h>      // errno
#include <stdbool.h>    // bool, false, true
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 *  Description:
 *		Fetches the access time (atime) for pathname by reading the st_atim member from the
 *		stat struct.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to fetch the access time for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		The access time, on success.  0 on error, and errnum is set.
 */
time_t get_access_time(const char *pathname, int *errnum);

/*
 *  Description:
 *		Extracts the st_blocks field from the stat struct which indicates the number of blocks
 *		allocated to filename, in 512-byte units.  (This may be smaller than st_size/512 when
 *		the file has holes.)
 *      
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *      
 *  Returns:
 *      On success, the st_blksize value for filename.  On failure, this function
 *		returns 0 and sets errnum with an errno value.  Some filenames legitimately have a block
 *		count of zero so the only indication of error is a non-zero value in errnum.
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
 *		Fetches the status change time (ctime) for pathname by reading the st_ctim member from the
 *		stat struct.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to fetch the status change time for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		The status change time, on success.  0 on error, and errnum is set.
 */
time_t get_change_time(const char *pathname, int *errnum);

/*
 *  Description:
 *		Fetches the device ID of the device denoted by a character or block special file by
 *		reading the st_rdev member from the stat struct.  This means that non-character and
 *		non-block files will likely return 0.  This function uses stat() so the file
 *		type of a symbolic link will report back as the file it is linked to.  If positively
 *		identifying symbolic links is important, use is_sym_link() and lstat() instead.
 *      
 *  Args:
 *      pathname: Absolute or relative pathname to fetch its container's device ID for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *      
 *  Returns:
 *		Returns the st_rdev value on success.  Error conditions are indicated by non-zero values
 *		in errnum.  Use major(3) and minor(3) to decompose the return value into major and
 *		minor numbers.
 */
dev_t get_container_device_id(const char *pathname, int *errnum);

/*
 *  Description:
 *		Fetches the device ID of the device containing pathname by reading the st_dev member
 *		from the stat struct.  This function uses stat() so the file type of a symbolic link
 *		will report back as the file it is linked to.  If positively identifying symbolic
 *		links is important, use is_sym_link() and lstat() instead.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to fetch device ID for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		Returns the st_dev value on success.  Error conditions are indicated by non-zero values
 *		in errnum.  Use major(3) and minor(3) to decompose the return value into major and
 *		minor numbers.
 */
dev_t get_file_device_id(const char *pathname, int *errnum);

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
 *		Fetches the ID of pathname's group by reading the st_gid member from the stat struct.
 *		This function uses stat() so the file type of a symbolic link will report back as the
 *		file it is linked to.  If positively identifying symbolic links is important,
 *		use is_sym_link() and lstat() instead.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to the group's ID for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		Returns the owner's UID on success.  Error conditions are indicated by non-zero values
 *		in errnum.
 */
gid_t get_group(const char *pathname, int *errnum);

/*
 *  Description:
 *		Gets the number of pathname's hard links by reading the stat struct's st_nlink member.
 *		This function uses stat() so the file type of a symbolic link will report back as the
 *		file it is linked to.  If positively identifying symbolic links is important,
 *		use is_sym_link() and lstat() instead.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to count the hard links of.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		Number of hard links to pathname on success.  Error conditions are indicated by non-zero
 *		values in errnum.
 */
nlink_t get_hard_link_num(const char *pathname, int *errnum);

/*
 *  Description:
 *		Fetches the modification time (mtime) for pathname by reading the st_mtim member from the
 *		stat struct.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to fetch the modification time for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		The modification time, on success.  0 on error, and errnum is set.
 */
time_t get_mod_time(const char *pathname, int *errnum);

/*
 *  Description:
 *		Fetches the user ID of pathname's owner by reading the st_uid member from the stat struct.
 *		This function uses stat() so the file type of a symbolic link will report back as the
 *		file it is linked to.  If positively identifying symbolic links is important,
 *		use is_sym_link() and lstat() instead.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to the owner's user ID for.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		Returns the owner's UID on success.  Error conditions are indicated by non-zero values
 *		in errnum.
 */
uid_t get_owner(const char *pathname, int *errnum);

/*
 *  Description:
 *		Fetches the pathname's inode number by reading the st_ino member from the stat struct.
 *		This function uses stat() so the file type of a symbolic link will report back as the
 *		file it is linked to.  If positively identifying symbolic links is important,
 *		use is_sym_link() and lstat() instead.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to read the inode number of.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		Pathname's inode number on success.  Error conditions are indicated by non-zero values
 *		in errnum.
 */
ino_t get_serial_num(const char *pathname, int *errnum);

/*
 *  Description:
 *		Fetches pathname's size, in bytes, by reading the st_size member from the stat struct.
 *		This function uses stat() so the file type of a symbolic link will report back as the
 *		file it is linked to.  If positively identifying symbolic links is important,
 *		use is_sym_link() and lstat() instead.
 *
 *  Args:
 *      pathname: Absolute or relative pathname to the fetch the size of.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *
 *  Returns:
 *		Pathname's size, in bytes, on success.  Error conditions are indicated by non-zero values
 *		in errnum.
 */
off_t get_size(const char *pathname, int *errnum);


/*
 *  Description:
 *		Format the time_val into a standardized, human-readble string in output.
 *
 *  Args:
 *      output: [Out] Buffer to format the time into.
 *		output_size: The size, in bytes, of output.
 *		time_val: The time value to format.
 *
 *  Returns:
 *		0 on success.  Returns errno value or -1 on failure.
 */
int format_time(char *output, size_t output_size, time_t time_val);


/*
 *  Description:
 *      Answers the question, "Is filename a block device?".  Updates errnum with errno values.
 *		Calls stat(filename), reads the mode field from the stat struct, and checks it against
 *		the "block device" macro.
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
 *		Calls stat(filename), reads the mode field from the stat struct, and checks it against
 *		the "character device" macro.
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
 *		Calls stat(filename), reads the mode field from the stat struct, and checks it against
 *		the "directory" macro.
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
 *		Calls stat(filename), reads the mode field from the stat struct, and checks it against
 *		the "FIFO" macro.
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
 *		Calls stat(filename), reads the mode field from the stat struct, and checks it against
 *		the "socket" macro.
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
