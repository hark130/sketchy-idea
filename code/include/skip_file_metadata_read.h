#ifndef SKIP_FILE_METADATA_READ
#define SKIP_FILE_METADATA_READ

#include <stdbool.h>  // bool, false, true

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
ino_t get_serial_num(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
mode_t get_file_perms(const char *filename, int *errnum);

/*
 *  Description:
 *      
 *  Args:
 *      
 *  Returns:
 *      
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
nlink_t get_hard_link_num(const char *filename, int *errnum);

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
gid_t get_group(const char *filename, int *errnum);

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
 *      
 *  Args:
 *      
 *  Returns:
 *      
 */
off_t get_size(const char *filename, int *errnum);

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
time_t get_mod_time(const char *filename, int *errnum);

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
blksize_t get_block_size(const char *filename, int *errnum);

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
 *      Answers the question, "Is filename a regular file?".  Updates errnum with errno values.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if filename is a regular file.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_regular_file(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is pathname a directory?".  Updates errnum with errno values.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if pathname is a directory.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_directory(const char *pathname, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a character device?".  Updates errnum with errno values.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if filename is a character device.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_character_device(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a block device?".  Updates errnum with errno values.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if filename is a block device.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_block_device(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a named pipe?".  Updates errnum with errno values.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if filename is a named pipe.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_named_pipe(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a symbolic link?".  Updates errnum with errno values.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if filename is a symbolic link.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_sym_link(const char *filename, int *errnum);

/*
 *  Description:
 *      Answers the question, "Is filename a socket?".  Updates errnum with errno values.
 *  Args:
 *      filename: Absolute or relative filename to check.
 *      errnum: [Out] Stores the first errno value encountered here.  Set to 0 on success.
 *  Returns:
 *      True if filename is a socket.  False otherwise.  Also, returns False if an error
 *      occurred (bad input or otherwise).
 */
bool is_socket(const char *filename, int *errnum);

#endif  /* SKIP_FILE_METADATA_READ */
