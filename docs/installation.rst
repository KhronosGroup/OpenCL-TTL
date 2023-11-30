Installation
============

Requirements
------------

In order to usefully install the TTL library your require and existing working opencl installation
such as `pocl <https://github.com/pocl/pocl.git>`_.


Configure & Build
-----------------

Building and installing is done the usual CMake way, and CMake version 3.3 or higher is required.

After downloading the OpenCCL-TTL repository, run the following:

.. code-block:: bash
  
  cd <directory-with-ttl-sources>
  mkdir build
  cd build
  cmake .. [OPTIONS]
  make install

CMake Options
-------------

- ``TTL_INSTALL_DIR`` (optional) defines the directory to install OpenCL-TTL inside.