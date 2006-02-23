Introduction
------------
This is a mostly working port of DC++ to Linux (or Unix-likes in general, 
but there might be issues. It's only tested on Linux anyway). The name
is not really decided upon, Wulfor is kind of the development name (think
"Longhorn" from Microsoft) until we come up with something better (Linux DC++
or something like that is just too boring).

Dependencies:
-------------
Gtk+ 2.6
libglade 2.4
libgnome (Required by libglade on some systems it seems)
pthread
zlib
libbz2
scons
g++ >= 3.4 (Yes, this is REQUIRED for succesfull compilation!)

Compiling:
----------
cd /path/to/linuxdcpp
scons

Scons will tell you if something's missing in a clear and human-readable way. 
Hopefully. Fingers crossed. Knock on wood. Etc, etc. 

For compile options, look in the main SConstruct file.

Installing:
-----------
With binreloc:
Using binreloc, which is the default, you can just copy the binary and the
pixmaps and glade directories to wherever you like. Then you can add the program
to your menu or create a symlink in /bin for easy access. As long as the binary
and the pixmaps and glade directories are in the same place it will find them.

Using explicitly defined directory:
To install the program in a more traditional way by hard-coding data locations,
use scons PREFIX=/path/to/install/under to compile and
scons PREFIX=/path/to/install/under install to copy the files to the right
location.

Known problems/rants =):
------------------------
Utf8 locales are currently very buggy/not supported. A workaround is to specify
a non-utf8 locale when running the program, assuming you have a non-utf8 
locale on your system. For example, I'm from Sweden so my utf8 locale would be
sv_SE.UTF8. Starting the program like this: prompt# LANG=sv_SE.ISO-8859-1 ./ldcpp
would run ldcpp with a non-utf8 locale.

Most settings in the "Advanced" and "Appearance" tabs are not working. You can 
change the settings however you like but it won't ever change anything in the 
program.

Codepage is a bit of a headache. Files with local chars in their filename are 
sometimes rehashed on startup, messages in the hub and nicks in the wrong
codepage may not be shown, lots of pango warnings, etc.

Memory leaks.

License:
--------
The GPL (couldn't be any other, could it?)
See License.txt for details.

Contact:
--------
Website: linuxdcpp.berlios.de
IRC: #linuxdc++@freenode.org

