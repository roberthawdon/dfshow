DF-SHOW
=======

|Maintenance yes| |GPLv3 license| |Travis Build|

.. |Maintenance yes| image:: https://img.shields.io/badge/Maintained%3F-yes-green.svg
   :target: https://github.com/roberthawdon/dfshow/graphs/commit-activity

.. |GPLv3 license| image:: https://img.shields.io/badge/License-GPLv3-blue.svg
   :target: https://raw.github.com/roberthawdon/dfshow/master/LICENSE

.. |Travis Build| image:: https://api.travis-ci.org/roberthawdon/dfshow.svg?branch=master
   :target: https://travis-ci.org/roberthawdon/dfshow

:Author: Robert Ian Hawdon (git@robertianhawdon.me.uk)
:Source: https://github.com/roberthawdon/dfshow
:Version: alpha

**DF-SHOW (Directory File Show) is a Unix-like rewrite of the SHOW application from Larry Kroeker's DF-EDIT (Directory File Editor) for MS-DOS and PC-DOS systems, based on the Version 2.3d release from 1986.**

* `Bug Tracker`_

.. _`Bug Tracker`: https://github.com/roberthawdon/dfshow/issues

Introduction
------------

The show application lets users view the names of files and directories on a disk with information about the files. Files can be copied, moved, viewed, and edited (in your system's default editor). The application is run using the ``show`` command. The output is similar to the ``ls`` command.

Features
--------

* **List files.** DF-SHOW's primary function is to display a list of files on a system, in an interactive display allowing users' to browse up and down large lists of files in a directory.
* **View files.** The ability to quickly peek into text files makes DF-SHOW a powerful tool for power users, this function uses the default pager defined in the user's system environment.
* **Edit files.** Whilst DF-SHOW doesn't come with a text editor like DF-EDIT (for the reason there are many good, well established text editors already available), it can launch the user's default text editor for any given selected file.
* **Copy files.** DF-SHOW can copy files to another destination.
* **Rename files.** Users' can move files to a different file name.
* **Delete files.** DF-SHOW also removes files at the user's request.
* **Create directories.** Users' are also able to create new directories from within the DF-SHOW interface.
* **Update permissions.** Owners, groups, and permissions can be set within the application.
* **Hunt within files.** Find files in a directory containing a string.
* **Execute applications.** Files with execution permissions can be launched from withing DF-SHOW.

Installing DF-SHOW
------------------

**Arch**

Development builds of DF-SHOW are available in the AUR

.. code-block:: bash

    curl -L https://aur.archlinux.org/cgit/aur.git/snapshot/dfshow-git.tar.gz | tar xz
    cd dfshow-git
    makepkg -srci

Building from Git
-----------------

Download the project and prepare sources with

.. code-block:: bash

    git clone https://github.com/roberthawdon/dfshow
    cd dfshow
    ./bootstrap

Build DF-SHOW with

.. code-block:: bash

    ./configure
    make

Install DF-SHOW

.. code-block:: bash

    sudo make install

Running Show
------------

The show directory utility is invoked with the following command:

    show <directory_names>

The <directory_name> parameter can by any directory the current user has access to on the system, similar to the ``ls`` command. Using ``/`` will take the user to the root directory of the system.

If the show directory utility is invoked without specifing a directory, then the current directory is displayed.

Screenshots
-----------

**Directory View - Linux**

.. image:: https://user-images.githubusercontent.com/4301139/43677749-24e2ced0-97ff-11e8-8fa7-5c4b428af2a3.png
   :alt: Directory Viewer Under Linux

DF-SHOW running under Linux showing:

* Colored entries
* Human readable file sizes

**Directory View - MacOS**

.. image:: https://user-images.githubusercontent.com/4301139/42421531-f09ca60e-82ce-11e8-94c9-ce4d8ad9226a.png
   :alt: Show running under MacOS

**Directory View - Debian HURD**

.. image:: https://user-images.githubusercontent.com/4301139/42421533-f39b6fe8-82ce-11e8-8975-068d82dc741f.png
   :alt: Show running under Debian HURD

DF-SHOW running under Debian HURD showing the Author item for each file.
