Show
====

Overview
--------

The ``show`` application lets users view the names of files and directories on a disk with information about the files. Files can be copied, moved, viewed, and edited (in your system's default editor). The application is run using the show command. The output is similar to the ``ls`` command.

The screen is devided into three parts. The first is the command bar which displays the functions available, their hotkeys, and aditional prompts when requested. The commands are described in more detail in the `commands <#commands>`__ and `global commands <#global-commands>`__ sections.

The second part is the middle of the screen between the top and bottom rows. This displays information for the files and directories and is known as the display window. The top three rows of the display window are header lines which identify the information being displayed in each column, the number of objects in the current directory, the total space used by the files in the current directory, the remaining disk space on the mounted partition the directory resides, and the current path. Each line gives the object's permissions, hard links, the owner and group information, size of the object, date and time of the last modification, as well as the name of each file or directory.

The third part is known as the function bar, each function can be accessed by its numeric function key. The functions are described in more deatail in the `functions <#functions>`__ section.

Command line arguments
----------------------

The following options are shared with ``ls``:

``-@``
  Displays extended atributes keys and sizes (macOS only).

``-a``, ``--all``
  Does not ignore files or directories starting with ``.``. The
  ``.`` and ``..`` objects are never hidden due to their
  navigational functions in ``show``.

``--author``
  Prints the author of the file in the output.

``--block-size``\ =SIZE
  Scales sizes by SIZE when printing them; e.g., '--block-size=M'

``-B``, ``--ignore-backups``
  Hides files ending in ``~``, often used as backup files.

``--color``\ [=COLOR]
  Sets colorizing of files and directories. Can be set to ``always``
  or ``never``.

``-d``, ``--directory``
  Displays only directories, not files.

``-f``
  Does not sort the output, this is equivilent to running ``show``
  with ``-aU``.

``--full-time``
  Sets the Time/Date format to ``full-iso``.

``-g``
  Only displays the group of each object. Can be paird with
  ``--author``.

``-G``, ``--no-group``
  Omits the group of each object in the output.

``-h``, ``--human-readable``
  Displays the file sizes in a human readable format. For example,
  1024 Bytes will be represented as 1M.

``--si``
  As above, but uses powers of 1000. (1000 = 1m)

``-r``, ``--reverse``
  Reverse the order of sorting. Can be paird with ``-S`` or ``-t``

``-s``, ``--size``
  Displays file sizes in blocks.

``-S``
  Sorts files and directories by size, largest first.

``--time-style``\ =[TIME_STYLE]
  Time/Date format. Can be set to ``full-iso``, ``long-iso``,
  ``iso``, or ``locale``.

``-t``
  Sorts by modification time, newset first.

``-U``
  Displays output in the order they're found.

``-Z``, ``--context``
  Shows the secuirty context of each file, if one is defined.

``-1``
  Only shows the file name on display.

``--help``
  Displays help message, then exits.

``--version``
  Displays program version, then exits.

The following options are specific to ``show``:

``--theme``\ =[THEME]
  Color themes. Passing this argument without an option will display
  available themes.

``--no-danger``
  Disables the danger color scheme when running with elevated
  privileges.

``--marked``\ =[MARKED]
  Displays information about marked files. Can be set to ``always``,
  ``never``, or ``auto``.

``--no-sf``
  Doesn't display files using ``sf`` (if installed) and defaults to
  the user's PAGER setting.

``--show-on-enter``
  Sets the enter key to launch the show command instead of moving the
  selection down by one.

``--running``
  Checks for parent instances of ``show``, this is useful when using
  the Run Command option.

``--settings-menu``
  Launch ``show`` directly into the settings menu.

``--edit-themes``
  Launches ``show`` directly into the theme editor. (colors)

``--skip-to-first``
  Sets the highlighted item to the first object that isn't ``.`` or ``..``

Commands
--------

Several commands are provided which can be used on a single file or
multiple files. These commands provide a lot of the functions
supported by standard Unix commands, but in a convenient user
interface. The file to be operated on is selected by navigating the
cursor on the line of the desired file. The highlighed (first
character) of the command is used to invoke its function. Multiple
files can be selected using the "Mark/Unmark" function. The following
is a list of valid commands and a brief description of each.

``Copy``
  Copy file or marked files to a specified destination.

``Delete``
  Delete file or marked files.

``Edit``
  Invoke the system's text editor.

``Hidden``
  Toggle the visibility of hidden objects.

``Link``
  Create a link between files or directories.

``Modify``
  Change the owner/group or permissions of files.

``Quit``
  Quit current directory.

``Rename``
  Rename/move file or marked files.

``Show``
  Show file or directory.

``Touch``
  Change file timestamps.

``eXec``
  Runs files marked as executable.

``hUnt``
  Searches and marks files matching a string.

Each of the commands are described in the following paragraphs. If
the command has any additional propts, they are also described here.

``Copy``
  Copy the selected file, or marked files to another file location.
  When using this command, only the destination must be specified.
  The destination file name is requested by one of the following
  prompts. An empty response to the prompt or the *ESC* key is used
  to abort the command. If there are no marked files, a single file
  copy is requested; otherwise, a multiple file copy is requested.

  ``Copy file to:``

  ``Copy multiple files to:``

  If the destination file already exists, confirmation to replace
  the file is requested by the following prompt.

  ``Replace file [<file name>]? (Yes/No)``

``Delete``
  Delete the selected file, or marked files. As this is a
  destructive process, confirmation is requested by one of the
  following prompts. If there are no marked files, a single file
  delete is requested; otherwise, a multiple delete is requested.

  ``Delete file? (Yes/No)``

  ``Delete file [<file name>]? (Yes/No/All/Stop)``

  For single files, a "Y" will delete files and anything else will
  abort the delete file operation. For multiple files, confirmation
  for all files is requested. A "Y" will delete the prompted file,
  an "N" will not delete the file, an "A" will delete all the marked
  files without further confirmation, and an "S" will stop the
  multiple delete command.

``Edit``
  This invokes the default text editor. Please consult the ``man``
  pages of your preferred editor for more information. The edit
  command utilizes the ``VISUAL`` or ``EDITOR`` environment
  variable. If this has not been set, the following message is
  displayed.

  ``Please export a VISUAL environment variable to define the utility program name.``

  To resolve this, set a variable to your preferred editor in your
  shell's user preference file.

  ``export VISUAL=/usr/bin/vi``

``Hidden``
  This toggles the display of hidden files and directories. This
  allows adjustment of the ``-a`` option whilst in the application.
  If the currently selected object is hidden by this toggle, the
  cursor is returned to the top of the directory.

``Link``
  Creates a link to the selected file. The type of link is requested
  by the following prompt.

  ``Link Type - Hard, Symbolic (enter = S)``

  Selecting "H" will bring up the following prompt.

  ``Hard link to:``

  ``show`` will then create a hard link to the selected file at the target
  specified.

  Selecting "S" will bring up the following prompt.

  ``Symbolic link to:``

  After a location is specified, the following prompt is shown to ask how to
  link to the target file.

  ``Link Location - Absolute, Relative (enter = R)`` 

  Selecting "A" will link to the file directly relevent to the root directory
  (/).

  Selecting "R" will instruct ``show`` to link to the target file relative to the
  destination.

``Modify``
  Modify the owner/group properties or changes the permissions of an
  object, or multiple objects. When invoked, the following prompt is
  displayed.

  ``Modify: Owner/Group, Permissions``

  Selecting "O" will bring up two prompts.

  ``Set Owner:``

  ``Set Group (owner):``

  If the names of the owner or group is invalid, an error will be displayed to
  the user. If the group prompt is left blank, then it will use the value of 
  the owner prompt.

  Selecting "P" will bring up the following prompt.

  ``Modify Permissions:``

  The syntax is a 3 or 4 digit octect. See the ``man`` pages for
  ``chmod`` for further information.

``Quit``
  Closes the current directory currently displayed. The application
  will return you to the previous directory you were viewing. If
  there are no previous directory, a blank screen showing the global
  commands is displayed.

``Rename``
  Rename the selected file, or marked files to a new file name. The
  new file name is requested by one of the following prompts. After
  a file is renamed, the new file information line is into the list
  (providing the file has been renamed to the same directory) and
  the old information line is removed. Files can only be moved to
  locations on the same mounted partition. An empty response to the
  prompt or the *ESC* key will abort the command. If there are no
  marked files, a single file rename is requested; otherwise a
  multiple file rename is requested.

  ``Rename file to:``

  ``Rename multiple files to:``

``Show``
  Will either display the contents of a directory, or open the
  contents of a file. The show file command utilizes the ``PAGER``
  environment variable. If this has not been set, the following
  message is displayed.

  ``Please export a PAGER environment variable to define the utility program name.``

  To resolve this, set a variable to your preferred pager in your
  shell's user preference file.

  ``export PAGER=/usr/bin/less``

``Touch``
  Sets the timestamp of the selected file(s). When selected, the following
  prompt will be shown.

  ``Set Time - Accessed, Both, Modified (enter = B)``

  By default, ``show`` will modify both the access and modified times. When one
  of the options is selected, one of the following prompts are shown.

  ``Set Access Time:``
  ``Set Modifed Time:``
  ``Set Time:``

  Set the desired time in one of the following formats:
  ``YYYY-MM-DD HH:MM:SS``, ``HH:MM:SS``

``hUnt``
  Hunts the selected file, or marked files containing a regex
  string. When used with a single file, the selected file will be
  marked if the string matches. When using multiple files, any files
  not matching the string will be unselected. Case sensitivity is
  requested by the following prompt, afterwards the user is asked to
  input the string to search. To abort at this prompt, the *ESC* key
  must be used.

  ``Case Sensitive, Yes/No/ESC (enter = no)``

  Following this selection, one of the following prompts will be
  displayed.

  ``Match Case - Enter string:``

  ``Ignore Case - Enter string:``

``eXec``
  Will execute the selected file if it has the execute flag set and
  the current user running the ``show`` process has permission to.
  Arguments are requested by the following prompt. Unlike other
  commands, an empty response will execute the file without
  arguments. To abort at this prompt, the *ESC* key must be used.

  ``Args to pass to <file>:``

  The following error is displayed if the file does not have an
  executable flag, or the user does not have sufficient privileges
  to run.

  ``Error: Permission denied``


Creating parent directories
^^^^^^^^^^^^^^^^^^^^^^^^^^^

A number of the commands above will display the following prompt to create
parent directories if they are not present.

  ``Directory [/path/to/directory] does not exist. Create it? Yes/No (enter = no)``

  Selecting "Y" will instruct ``show`` to create the missing parent directories
  required to complete the command.

  Selecting "N" will abort the command due to the required parent directories
  not being available. An error message will be shown.

Functions
---------

In order to select objects to be used by the commands described in
the previous section, the cursor must be moved to the line of the
desired object. The functions to move the cursor and the list of
files in the display window are described here. A list of the valid
functions and their associated function keys is given list.

``F1``, ``PgDn``
  Page Down

``F2``, ``PgUp``
  Page Up

``F3``
  Top of List

``F4``
  Bottom of List

``F5``
  Refresh Directory

``F6``
  Mark/Unmark File

``F7``
  Mark All Files

``F8``
  Unmark All Files

``F9``
  Sort List

``F10``
  Block Mark

``HOME``
  Top of Display

``END``
  Bottom of Display

``Down``, ``RETURN``
  Down one line (``RETURN`` can be repurposed to be the ``Show`` command using the ``--show-on-enter`` argument)

``Up``
  Up one line

``Right``
  Right one column

``Left``
  Left one column

``ESC``
  Global Commands

The display functions with their associated key assignments are
described here.

``Page Down``
  ``F1, PgDn``: Scroll the display window down or forward a page on
  the list of files. The cursor is left in the same relative row of
  the window unless the end of the list is reached. If the last file
  of the list is already displayed in the window, the list is not
  scrolled, but the cursor is placed on the last file in the list.

``Page Up``
  ``F2, PgUp``: Scroll the display window up or backward a page on
  the list of files. The cursor is left in the same relative row of
  the window unless the beginning of the list is reached. If the
  first file of the list is already displayed in the window, the
  list is not scrolled, but the cursor is placed on the first file
  in the list.

``Top of List``
  ``F3``: Display the beginning of the list of files in the window
  and place the cursor on the first file in the list.

``Bottom of List``
  ``F4``: Display the end of the list of files in the display window
  and place the cursor on the last file of the list.

``Refresh Directory``
  ``F5``: Rereads the directory. This function is useful to update
  the list of files after several new files have been created or
  updated outside of the application.

``Mark/Unmark File``
  ``F6``: Toggle the file mark on the current file. The file mark is
  indicated with an "*" in front of the file name.

``Mark All Files``
  ``F7``: Set the file mark on all the files but not directories in
  the list.

``Unmark All Files``
  ``F8``: Remove the file marks from all files in the list.

``Sort List``
  ``F9``: Normally, the file list is sorted by file name
  alphabetically. This function allows the files to be listed based
  on another sorting criteria which is requested by the following
  promt.

  ``Sort list by - Date & time, Name, Size``

  The option is selected by using the first letter of the option
  name.

  ``Date & time``: Sort the list on date and time so the newest
  files are at the top of the list.

  ``Name``: Sort the list on the file name.

  ``Size``: Sort the list on file size so the largest are at the top
  of the list.

  Using *SHIFT* whilst selecting an option performs that action in
  reverse order.

``Block Mark``
  ``F10``: Marks all files between two points. Files marked will be
  indicated with an ``*`` in front of them.

``Top of Display``
  ``HOME``: Move the cursor to the first file on the current
  display.

``Bottom of Display``
  ``END``: Move the cursor to the last file on the current display.

``Down One Line``
  ``Down Arrow, Return``: Move the cursor down one line to the next
  file in the display. If the cursor is on the bottom row of the
  window, the window is scrolled down one line. If the present line
  is the last file in the list, the cursor is not repositioned.

``Up One Line``
  ``Up Arrow``: Move the cursor up one line to the next file in the
  display. If the cursor is on the top row of the window, the window
  is scrolled up one line. If the present line is the first file in
  the list, the cursor is not repositioned.

``Right one column``
  ``Right Arrow``: Moves the display area one column. This occurs
  when an entry rolls off the edge of the display. Scrolling will
  stop at the end of the longest entry.

``Left one column``
  ``Left Arrow``: Moves the display area one column.

``Global Commands``
  ``ESC``: Invoke the `global commands <#global-commands>`__
  described in the next section. This allows another directory to be
  displayed without terminating the current display.

Global Commands
---------------

When a file group display is terminated with the Quit command, one of
the following commands can be used to display another group of files,
invoke the editor for a fire, or terminate the application
completely. The first character of the command is used to invoke the
desired function. The command line is shown below.

``cOlors, Config, Edit file, Help, Make dir, Quit, Run, Show dir, Touch file``

These commands are desctibed below.

``cOlors``
  Launches an inbuilt color configuration utility which cusomizes
  the colors for the various display areas in all the utilities.
  Further information can be found in the `colors <#colors>`__
  section.

``Config``
  Launches `show`'s configuration menu. From here, all aspects of
  ``show`` can be configured, and settings saved so they will persist
  between sessions. Further information can be found in the
  `configuring show <#configuring-show>`__ section.

``Edit file``
  Invoke the default text editor to edit the specified file. The
  file name is requested by the following prompt. An empty response
  is used to abort this command.

  ``Edit File - Enter pathname:``

``Help``
  Launches the ``man`` pages for ``show``.

``Make dir``
  Make a new directory. The directory name is requested by the
  following prompt. An empty response is used to abort this command.

  ``Make Directory - Enter pathname:``

``Quit``
  Terminate ``show``.

``Run``
  Invoke your shell. The ``show`` application is still resident, so
  the "exit" command will return to the application.

``Show dir``
  Invoke the application to display another directory. The directory
  name is requrested by the following prompt. An empty response is
  used to abort this command.

  ``Show Directory - Enter pathname:``

``Touch file``
  Updates the timestamp of a specified file requested by the following prompt.
  If the file doesn't exist, it will be created.

  ``Touch File - Enter pathname:``

  The following prompt is shown to ask if the time should be set to a specific
  date.

  ``Set Time? Yes/No (enter = N)``

  Selecting No will set the file's access and modification time to the current
  time.

Configuring show
----------------

``show`` features an inbuilt configuration menu where the user can tweak
the default settings. It is accessed from the global menu.

The following screen is displayed.

::

   SHOW Settings Menu - Quit, Revert, Save

      [ ] Display file colors
      <-> Show marked file info: <never> <always> <auto>
      <-> Sorting mode: <name> <date> <size> <unsorted>
      [ ] Reverse sorting order
      <-> Time style: <locale> <iso> <long-iso> <full-iso>
      [ ] Show hidden files
      [ ] Hide backup files
      [ ] Use 3rd party pager over SF
      [ ] Use SI units
      [ ] Human readable sizes
      [ ] Enter key acts like Show
      < > Owner Column: <owner> <group> <author>
      [ ] Skip to the first object

There are three types of configuration items, each denoted with a different
symbol:

``[ ]``
Indicates a toggle switch, when active, the switch will display ``[*]``.
To toggle a value, press *SPACE* when the cursor is highlighed over a specific item.

``<->``
Indicates a single value option, the active item will be highlighted. To change
the value, use either the arrow keys, or *SPACE* to toggle through each available
option.

``< >``
Indicates a multi value option, the active items will be highlighted. To change
their values, use the arrow keys to highlight the desired option and press
*SPACE* to toggle its activation status.

The following commands can be used within this menu.

``Quit``
Applies changes and either returns to the previous screen.

``Revert``
Reverts settings to their original value from when the settings menu was
invoked.

``Save``
Saves settings for future sessions.

Colors
======

The inbuilt Colors utility is used to customize the colors used in
the applications. It is accessed from the global menu of the ``show``
utility.

The following screen is displayed after launch.

::

   Color number, Load, Quit, Save, Toggle, Use

       Command lines                            !-Default
       Display lines                            ?-Default Bold
       Error messages                           0-Black
       Information lines                        1-Red
       Heading lines                            2-Green
       Danger lines                             3-Brown
       Selected block lines                     4-Blue
       Highlight                                5-Magenta
       Text input                               6-Cyan
       Directories                              7-Light Gray
       Symbolic links                           8-Dark Gray
       Orphened symbolic links                  9-Light Red
       Executable files                         A-Light Green
       Set user identification                  B-Yellow
       Set group identification                 C-Light Blue
       Sticky bit directory                     D-Light Magenta
       Sticky bit directory - other writable    E-Light Cyan
                                                F-White


                      Select 0 to F for desired foreground color

Initially, the cursor is positioned beside the "Command lines"
string. The cursor can be moved to each of the display types using
the up and down cursor keys, and each color can be set by using the
number of the desired color. The background color can be set by using
the ``Toggle`` command. Each of the types of lines are described
below.

``Command lines``
  The color of the top and bottom lines of each utility. These lines
  display the valid command, funcation keys and other global
  information.

``Display lines``
  The color for the main text lines in each utilities' display.

``Error messages``
  The color in which any error messages are displayed.

``Information lines``
  The color used to display general information such as the
  directory header information.

``Heading lines``
  The color used to display the headings for each column in
  ``show``.

``Danger lines``
  The color used to replace the informationa lines with a warning,
  such as when running as the root user.

``Selected block lines``
  The color of the selected block lines of the current file in the
  ``show`` utility.

``Highlight``
  The color of the command/function keys.

``Text input``
  The color of input text lines.

The following color settings are used when ``--color`` arguement is used.
They are used to differentiate object status:

``Directories``
``Symbolic links``
``Orphened symbolic links``
``Executable files``
``Set user identification``
``Set group identification``
``Sticky bit directory``
``Sticky bit directory - other writable``

After each of the colors have been changed to the desired color, the
theme must be saved with the ``Save`` command. All the commands are
described below.

``Load``
  Loads a theme file so it can be modified or used. The following
  promt requests the data file name.

  ``Load Colors - Enter file pathname:``

``Quit``
  Quit the color modification utility and return to ``show``.

``Save``
  Save the theme information in a data file for use by all the
  utilities. The following promt requests the data file name.

  ``Save Colors - Enter file pathname:``

``Toggle``
  Switches between foreground and background selection.

``Use``
  Sets the theme as the default to persist between sessions. The current
  theme needs to be saved before this command can be used.
