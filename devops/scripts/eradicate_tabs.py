"""Eradicate tabs by replacing them with spaces.

Starting at the current working directory and walking down, replace all tabs with spaces.
"""

# Standard Imports
from pathlib import Path
import os
import sys
# Third Party Imports
from hobo.disk_operations import find_path_to_dir, list_files, modify_file
from hobo.misc import print_exception
from hobo.validation import validate_string
# Local Imports


def eradicate_tabs(replacement: str = '    ') -> None:
    """Replace all tabs with the replacement string.

    Start at the current working directory and work your way down.
    """
    # LOCAL VARIABLES
    repo_dir = ''   # Path to the repo
    file_list = []  # List of files
    # Repo directories to do replacements
    dir_list = [os.path.join('code', 'include'), os.path.join('code', 'src')]

    # INPUT VALIDATION
    validate_string(validate_this=replacement, param_name='replacement', can_be_empty=True)

    # ERADICATE THEM
    repo_dir = find_path_to_dir(dir_to_find='sketchy-idea')
    for dir_entry in dir_list:
        # file_list = list_files(dirname=os.path.join(repo_dir, dir_entry), include_dirname=True)
        file_list = [filename for filename in list_files(dirname=os.path.join(repo_dir, dir_entry),
                                                         include_dirname=True)
                     if filename.endswith('.c') or filename.endswith('.h')]
        for file_entry in file_list:
            try:
                modify_file(file_path=Path(file_entry), regex='[\\t]', replace_str=replacement,
                            must_find_match=True)
            except ValueError as err:
                if err.args[0].startswith('Expected '):
                    pass  # hobo.disk_operations.modify_file() has a BUG?!


def main() -> int:
    """Entry point for this module.

    Returns:
        0 on success, 1 on exception.
    """
    # LOCAL VARIABLES
    exit_code = 0  # Return value

    # ERADICATE THEM
    try:
        eradicate_tabs()
    except Exception as err:
        print_exception(err)
        exit_code = 1

    # DONE
    return exit_code


if __name__ == '__main__':
    sys.exit(main())
