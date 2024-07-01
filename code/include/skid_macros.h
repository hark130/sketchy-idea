/*
 *	Defines common use macros for use with SKETCHY IDEA (SKID).
 */

#ifndef __SKID_MACROS__
#define __SKID_MACROS__

#define SKID_BAD_FD (signed int)-1  // Use this to standardize "invalid" file descriptors
#define SKID_MAX_SZ (~(size_t)0)    // Library's value for the maximum size_t value
#ifndef ENOERR
#define ENOERR ((int)0)  // Success value for errno
#endif  /* ENOERR */

// You may use these macros with SKID mode_t arguments.
/* We translated the chown(2) macros so you don't have to! */
// Owner Permissions
#define SKID_MODE_OWNER_R S_IRUSR  // Read by owner
#define SKID_MODE_OWNER_W S_IWUSR  // Write by owner
#define SKID_MODE_OWNER_X S_IXUSR  // Execute/search by owner
// Group Permissions
#define SKID_MODE_GROUP_R S_IRGRP  // Read by group
#define SKID_MODE_GROUP_W S_IWGRP  // Write by group
#define SKID_MODE_GROUP_X S_IXGRP  // Execute/search by group
// Other Permissions
#define SKID_MODE_OTHER_R S_IROTH  // Read by others
#define SKID_MODE_OTHER_W S_IWOTH  // Write by others
#define SKID_MODE_OTHER_X S_IXOTH  // Execute/search by others
// Special Permissions
#define SKID_MODE_SET_UID S_ISUID  // set-user-ID
#define SKID_MODE_SET_GID S_ISGID  // set-user-ID
#define SKID_MODE_STICKYB S_ISVTX  // sticky bit (see: unlink(2), restricted deletion flag)

#endif  /* __SKID_MACROS__ */
