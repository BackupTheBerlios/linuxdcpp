Introduction
------------
This is a mostly working port of DC++ to Linux. (or Unix-likes in general, 
but there might be issues. It's only tested on Linux anyway) The name
is not really decided upon, Wulfor is kind of the development name (think
"Longhorn" from Microsoft) until we come up with something better (Linux DC++
or something like that is just too boring)

Dependencies:
-------------
Gtk+ 2.4
libglade 2.4
pthread
zlib
libbz2
scons
g++ 3.4 (Yes, this is REQUIRED for succesfull compilation!)

Compiling:
----------
cd /wherever/linuxdcpp
scons

Scons will tell you if something's missing in a clear and human-readable way. 
Hopefully. Fingers crossed. Knock on wood. Etc, etc. 

Known problems:
---------------
Fedora core 3 has issues with asm/atomic.h. Try compiling with scons atomic=0
to avoid including that file.

License:
--------
The GPL (couldn't be any other, could it?)

Contact:
--------
Website: linuxdcpp.berlios.de
Irc: #linuxdc++ @ freenode.org


