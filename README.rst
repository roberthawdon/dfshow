DF-SHOW
=======

|Maintenance yes| |GPLv3 license| |Documentation| |Contributor Covenant|

.. |Maintenance yes| image:: https://img.shields.io/badge/Maintained%3F-yes-green.svg
   :target: https://github.com/roberthawdon/dfshow/graphs/commit-activity

.. |GPLv3 license| image:: https://img.shields.io/badge/License-GPLv3-blue.svg
   :target: https://raw.github.com/roberthawdon/dfshow/master/LICENSE

.. |Documentation| image:: https://readthedocs.org/projects/dfshow/badge/?version=latest
   :target: https://dfshow.readthedocs.io/en/latest/?badge=latest

.. |Contributor Covenant| image:: https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg
   :target: CODE_OF_CONDUCT.md

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
* **Update permissions.** Owners, groups, and permissions can be set within the application. SELinux security contexts can also be modified on supported systems.
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

On macOS with Homebrew, the following needs to be passed when configuring:

.. code-block:: bash

   ./configure LDFLAGS="-L$(brew --prefix)/lib" CFLAGS="-I$(brew --prefix)/include" CC=clang

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
   <details>
       <summary>
           <img src="misc/donate/logos/eth-logo.png" width="16" height="16"> Ethereum
       </summary>
       <br>0xF282bccB7FEdA4BaCc84F75478dA84D61980DDab
       <br><img src="misc/donate/addresses/erc20-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/lrc-logo.png" width="16" height="16"> Loopring (Layers 1 and 2)
       </summary>
       <br>0xF282bccB7FEdA4BaCc84F75478dA84D61980DDab
       <br><img src="misc/donate/addresses/erc20-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/imx-logo.png" width="16" height="16"> Immutable X (Layers 1 and 2)
       </summary>
       <br>0xF282bccB7FEdA4BaCc84F75478dA84D61980DDab
       <br><img src="misc/donate/addresses/erc20-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/bch-logo.png" width="16" height="16"> Bitcoin Cash
       </summary>
       <br>qpmmdn9xvurp8dt2j4sadnzulun8jvjsjv5ymt2uk4
       <br><img src="misc/donate/addresses/bitcoin-cash-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/doge-logo.png" width="16" height="16"> Dogecoin
       </summary>
       <br>D7jSRBKo9vBmwAjKZ6wZvZ8AGPDw4u9yoY
       <br><img src="misc/donate/addresses/dogecoin-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/bat-logo.png" width="16" height="16"> Basic Attention Token
       </summary>
       <br>0xF282bccB7FEdA4BaCc84F75478dA84D61980DDab
       <br><img src="misc/donate/addresses/erc20-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/matic-logo.png" width="16" height="16"> Polygon (MATIC)
       </summary>
       <br>0xF282bccB7FEdA4BaCc84F75478dA84D61980DDab
       <br><img src="misc/donate/addresses/erc20-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/ada-logo.png" width="16" height="16"> Cardano
       </summary>
       <br>addr1q8g9tdafpscdzhpfl7w94d9mlrmjzsx30wu0ag0f7r9u8ekyy2s96z0fymwaymh537zk3geedx89u54ac6em4lsnp8esyfefhg
       <br><img src="misc/donate/addresses/cardano-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/ltc-logo.png" width="16" height="16"> Litecoin
       </summary>
       <br>ltc1q00qzqhutjnlgarvpk3x6vq3yguntt2dm57k28q
       <br><img src="misc/donate/addresses/litecoin-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/xrp-logo.png" width="16" height="16"> Ripple (XRP)
       </summary>
       <br>raozHiwzd2mEQSWMTMefoBdAAuiw2okzCE
       <br><img src="misc/donate/addresses/ripple-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/xlm-logo.png" width="16" height="16"> Stellar
       </summary>
       <br>GCOIIDGU5BFVSPPCYPEPBVBSWZL7NYR2MJWVMJKWELCNFHRHLFFAL3G5
       <br><img src="misc/donate/addresses/stellar-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/vet-logo.png" width="16" height="16"> VeChain
       </summary>
       <br>0x45D9C3c345901D445B72eCd70852b6f06D8945D3
       <br><img src="misc/donate/addresses/vechain-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/akt-logo.png" width="16" height="16"> Akash
       </summary>
       <br>akash1v3rk372avjksx8v2tvj5g83686k24les67rjnt
       <br><img src="misc/donate/addresses/akash-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/akn-logo.png" width="16" height="16"> Akoin
       </summary>
       <br>GCOIIDGU5BFVSPPCYPEPBVBSWZL7NYR2MJWVMJKWELCNFHRHLFFAL3G5
       <br><img src="misc/donate/addresses/stellar-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/usdc-logo.png" width="16" height="16"> USD Coin
       </summary>
       <br>0xF282bccB7FEdA4BaCc84F75478dA84D61980DDab
       <br><img src="misc/donate/addresses/erc20-qr.png"><br>
   </details>
   <details>
       <summary>
           <img src="misc/donate/logos/usdt-logo.png" width="16" height="16"> Tether
       </summary>
       <br>0xF282bccB7FEdA4BaCc84F75478dA84D61980DDab
       <br><img src="misc/donate/addresses/erc20-qr.png"><br>
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
