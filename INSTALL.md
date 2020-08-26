Installing qDecoder
===================

## Get qDecoder Package

The latest version of qDecoder can be found at https://github.com/wolkykim/qdecoder

## Configure

Configure compile option by executing included "configure" script.

```
$ ./configure
```

qDecoder can be FastCGI enabled at the compile time, use --enable-fastcgi option and put the directory path where the "fcgi_stdio.h" is located.

```
$ ./configure --enable-fastcgi=/usr/local/include
```

By default qDecoder will be install on /usr/local/{include,lib}, so use --prefix option if you want to change the installation path.

```
$ ./configure --prefix=/usr/local/qdecoder
```

To see detailed configure options, use --help option.

```
$ ./configure --help
```

## Compile

Type make to compile the source codes.

```
$ make
```

## Install

This command will install qDecoder on your system. By default, the directory prefix is "/usr/local" so it will be install on /usr/local/include and /usr/local/lib.

```
$ make install
```

This will only install header files and library(archive; static and dynamic) files.
The Examples and Documentations will not be installed, so make a copy of it by yourself if you want to keep it.

## Uninstall

qDecoder can be completely removed from the system.

```
$ make uninstall
```

## Test

Sample sources and API document are provided with the package in the "examples" and "doc" directories.

The examples also can be compiled by typing "make" command in the "examples" directory.
And "index.html" file is provided for your tests.

Analyzing the example codes is probably best starting point.
But be aware that the examples were written as simple as it can be just to give you straight-forward ideas, so please refer included API document for more details.

## Windows

Windows native MinGW compiler/toolchain (i.e. not using Cygwin or MSYS/MSYS2) support has been added to qDecoder and tested with TDM64-GCC 4.7.1 in Windows 7 64-bit.

These Windows shell scripts will respectively build the 64-bit flavors of qdecoder.dll and the qDecoder's examples towards a "bin" directory. If you need or prefer to target a 32-bit architecture, simply replace "-m64" with "-m32" in these batch files.
```
$ build_dll.bat
$ build_examples.bat
```
