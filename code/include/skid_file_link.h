#ifndef __SKID_FILE_LINK__
#define __SKID_FILE_LINK__

/*
 *  Description:
 *      Create a new hard link to an existing file by calling link().
 *
 *	Notes:
 *      If hard_link exists, it will not be overwritten.
 *      This new name may be used exactly as the old one for any operation;
 *      both names refer to the same file (and so have the same permissions and ownership)
 *      and it is impossible to tell which name was the "original".
 *
 *  Args:
 *      source: Absolute or relative pathname to create a hard link for.
 *      hard_link: The pathname, relative or absolute, to the source's hard link.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.
 */
int create_hard_link(const char *source, const char *hard_link);

/*
 *	Description:
 *		Create a new symbolic link to an existing file by calling symlink().
 *
 *	Notes:
 *		Symbolic links are interpreted at run time as if the contents of the link had been
 *      substituted into the path being followed to find a file or directory.
 *      Symbolic links may contain ..  path components, which (if used at the start of the link)
 *      refer to the parent directories of that in which the link resides.  A symbolic link
 *      (also known as a soft link) may point to an existing file or to a nonexistent one;
 *      the latter case is known as a dangling link.  The permissions of a symbolic link are
 *      irrelevant; the ownership is ignored when following the link, but is checked when
 *      removal or renaming of the link is requested and the link is in a directory with the
 *      sticky bit (S_ISVTX) set.  If sym_link exists, it will not be overwritten.
 *
 *  Args:
 *      dest: Absolute or relative pathname to create a symbolic link for.
 *		sym_link: The pathname, relative or absolute, to the symbolic link that points at dest.
 *
 *  Returns:
 *      ENOERR, on success.  On failure, an errno value.
 */
int create_sym_link(const char *dest, const char *sym_link);

#endif  /* __SKID_FILE_LINK__ */
