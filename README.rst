DF-SHOW
=======

|Maintenance yes| |GPLv3 license| |Travis Build| |Documentation|

.. |Maintenance yes| image:: https://img.shields.io/badge/Maintained%3F-yes-green.svg
   :target: https://github.com/roberthawdon/dfshow/graphs/commit-activity

.. |GPLv3 license| image:: https://img.shields.io/badge/License-GPLv3-blue.svg
   :target: https://raw.github.com/roberthawdon/dfshow/master/LICENSE

.. |Travis Build| image:: https://api.travis-ci.org/roberthawdon/dfshow.svg?branch=master
   :target: https://travis-ci.org/roberthawdon/dfshow

.. |Documentation| image:: https://readthedocs.org/projects/dfshow/badge/?version=latest
   :target: https://dfshow.readthedocs.io/en/latest/?badge=latest

:Author: Robert Ian Hawdon (git@robertianhawdon.me.uk)
:Source: https://github.com/roberthawdon/dfshow
:Version: beta

**DF-SHOW (Directory File Show) is a Unix-like rewrite of some of the applications from Larry Kroeker's DF-EDIT (Directory File Editor) for MS-DOS and PC-DOS systems, based on the Version 2.3d release from 1986.**

* `Bug Tracker`_

.. _`Bug Tracker`: https://github.com/roberthawdon/dfshow/issues

Introduction
------------

The show application lets users view the names of files and directories on a disk with information about the files. Files can be copied, moved, viewed, and edited (in your system's default editor). The application is run using the ``show`` command. The output is similar to the ``ls`` command. A file view is also included which can be invoked using the ``sf`` command.

Features
--------

* **List files.** DF-SHOW's primary function is to display a list of files on a system, in an interactive display allowing users' to browse up and down large lists of files in a directory.
* **View files.** The ability to quickly peek into text files makes DF-SHOW a powerful tool for power users, this function uses the default pager defined in the user's system environment or the included ``sf`` application.
* **Edit files.** Whilst DF-SHOW doesn't come with a text editor like DF-EDIT (for the reason there are many good, well established text editors already available), it can launch the user's default text editor for any given selected file.
* **Copy files.** DF-SHOW can copy files to another destination.
* **Create links.** Both Symbolic and Hard links can be created using DF-SHOW.
* **Rename files.** Users' can move files to a different file name.
* **Delete files.** DF-SHOW also removes files at the user's request.
* **Create directories.** Users' are also able to create new directories from within the DF-SHOW interface.
* **Touch files.** Timestamps on files can be changed, as well as the creation of new blank files.
* **Update permissions.** Owners, groups, and permissions can be set within the application.
* **Hunt within files.** Find files in a directory containing a string.
* **Execute applications.** Files with execution permissions can be launched from withing DF-SHOW.

Installing DF-SHOW
------------------

**Arch**

Releases of DF-SHOW are available in the AUR

.. code-block:: bash

    curl -L https://aur.archlinux.org/cgit/aur.git/snapshot/dfshow.tar.gz | tar xz
    cd dfshow
    makepkg -srci

The latest cutting edge versions from the ``master`` branch are also available in the AUR

.. code-block:: bash

    curl -L https://aur.archlinux.org/cgit/aur.git/snapshot/dfshow-git.tar.gz | tar xz
    cd dfshow-git
    makepkg -srci

**Ubuntu**

Run the following to install DF-SHOW from the offical PPA

.. code-block:: bash

    sudo add-apt-repository ppa:ian-hawdon/dfshow
    sudo apt-get update
    sudo apt-get install dfshow

**RPM Based Distributions**

Packages are available from the `Fedora Copr`_ build service:

If you’re using a version of Linux with dnf:

.. code-block:: bash

   sudo dnf copr enable roberthawdon/DF-SHOW

you need to have ``dnf-plugins-core`` installed

.. code-block:: bash

   sudo dnf install dfshow

If you have older distribution:

.. code-block:: bash

   sudo yum copr enable roberthawdon/DF-SHOW

you need to have ``yum-plugin-copr`` installed

.. code-block:: bash

   sudo yum install dfshow

For more information, please see the `roberthawdon/DF-SHOW`_ repository.

.. _`Fedora Copr`: https://copr.fedoraproject.org/coprs/

.. _`roberthawdon/DF-SHOW`: https://copr.fedorainfracloud.org/coprs/roberthawdon/DF-SHOW/

**macOS**

Versions of DF-SHOW for macOS are provided via Homebrew. Homebrew is available for macOS versions 10.6 (Snow Leopard) and above. Any version of macOS supporting Homebrew can be used to install DF-SHOW.

Please visit the `Homebrew homepage`_ for instructions for installing Homebrew on your Mac.

.. _`Homebrew homepage`: https://brew.sh/

To install DF-SHOW via Homebrew, run the following in your terminal:

.. code-block:: bash

  brew tap roberthawdon/dfshow
  brew install dfshow

Building from Git
-----------------

Download the project and prepare sources with

.. code-block:: bash

    git clone https://github.com/roberthawdon/dfshow
    cd dfshow
    ./bootstrap
    ./configure

On some BSD systems, you may need to pass extra parameters to configure and build DF-SHOW:

.. code-block:: bash

    ./configure LDFLAGS="-L/usr/local/lib" CFLAGS="-I/usr/local/include"

Build DF-SHOW with

.. code-block:: bash

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

Running SF
----------

The show file utility is invoked with the following command:

    sf <file>

The <file> parameter can be any file the current user has access to on the system. If the show file utility is invoked without specifing a file, then the user is prompted to load a file within the application.

Donations
---------

To support this project, you can make a donation to its current maintainer:

**PayPal**

|PayPal|

Donations are also greatly appreciated via the following crypto currencies:

.. raw:: html

   <details>
       <summary>
           <img src="misc/donate/logos/btc-logo.png" width="16" height="16"> Bitcoin
       </summary>
       <br>bc1qxgy074cwdsvekg98fkqzju99kgyxzcyuqgqm6p
       <br><img src="misc/donate/addresses/bitcoin-qr.png"><br>
   </details>

.. |PayPal| image:: https://user-images.githubusercontent.com/4301139/91666536-6fb15080-eaf5-11ea-974b-f0eb997615b4.gif
   :target: https://paypal.me/RIHawdon

Screenshots
-----------

**Show**

.. image:: https://user-images.githubusercontent.com/4301139/52901919-bd12e900-3201-11e9-9114-0ae23a6f9746.png
   :alt: Directory Viewer

**SF - Show File**

.. image:: https://user-images.githubusercontent.com/4301139/56430876-c121bc80-62bf-11e9-9ac3-d892d8f37acb.png
   :alt: Show File

**Colors (Built into Show)**

.. image:: https://user-images.githubusercontent.com/4301139/56430881-c2eb8000-62bf-11e9-978b-3fda6f8cb0f3.png
   :alt: Colors
