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
g++ >= 3.4 (Yes, this is REQUIRED for succesfull compilation!)

Compiling:
----------
cd /wherever/linuxdcpp
scons

Scons will tell you if something's missing in a clear and human-readable way. 
Hopefully. Fingers crossed. Knock on wood. Etc, etc. 

Known problems/rants =):
------------------------
Utf8 locales are currently very buggy/not supported. A workaround is to specify
a non utf8 locale when running the program, assuming you have a non utf8 
locale on your system. For example, I'm from Sweden so my utf8 locale would be
sv_SE.UTF8. Starting the program like this: prompt# LC_CTYPE=sv_SE ./dcpp
would run dcpp with a non utf8 locale.

Most settings in the "Advanced" and "Apperance" pages are not working. You can 
change the settings however you like but it won't ever change anything in the 
program.

Codepage is a bit of a headache. Files with local chars in filename are 
somethimes rehashed on startup, messages in the hub and nicks in the wrong
codepage may not be shown. Lots of pango warnings.

Memory leaks.

License:
--------
The GPL (couldn't be any other, could it?)
See License.txt for details.

Contact:
--------
Website: linuxdcpp.berlios.de
Irc: #linuxdc++ @ freenode.org


