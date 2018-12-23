Installation
============

Pre Built Packages
------------------

Arch
^^^^

`Releases of DF-SHOW`_ are available in the AUR

.. code-block:: bash

    curl -L https://aur.archlinux.org/cgit/aur.git/snapshot/dfshow.tar.gz | tar xz
    cd dfshow
    makepkg -srci

The `latest cutting edge`_ versions from the ``master`` branch are also available in the AUR

.. code-block:: bash

    curl -L https://aur.archlinux.org/cgit/aur.git/snapshot/dfshow-git.tar.gz | tar xz
    cd dfshow-git
    makepkg -srci

.. _`Releases of DF-SHOW`: https://aur.archlinux.org/packages/dfshow/

.. _`latest cutting edge`: https://aur.archlinux.org/packages/dfshow-git/

Ubuntu
^^^^^^

Run the following to install DF-SHOW from the `Launchpad hosted PPA`_

.. code-block:: bash

    sudo add-apt-repository ppa:ian-hawdon/dfshow
    sudo apt-get update
    sudo apt-get install dfshow

.. _`Launchpad hosted PPA`: https://launchpad.net/~ian-hawdon/+archive/ubuntu/dfshow

RPM Based Distributions
^^^^^^^^^^^^^^^^^^^^^^^

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

Building from Source
--------------------

Ensure you have the following dependencies installed:

* Your system's build tools
* gcc
* ncurses development packages
* libconfig development packages

Download the project and prepare sources.

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
