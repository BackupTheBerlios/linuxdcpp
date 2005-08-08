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

Known problems/rants =):
------------------------
Fedora core 3 has issues with asm/atomic.h. Try compiling with scons atomic=0
to avoid including that file.

Most settings in the "Advanced" and "Apperance" pages are not working. You can 
change the settings however you like but it won't ever change anything in the 
program.

Codepage is a bit of a headache. Files with local chars in filename are 
somethimes rehashed on startup, messages in the hub and nicks in the wrong
codepage may not be shown. Lots of pango warnings.

The gui is slow when entering large hubs. This is because I had to make the 
locking less fine-grained than intended to avoid a deadlock. The basic
design of the program is a bit clunky anyway, so I would like to redo it. 
I haven't really got any ideas for how it should be done cleanly, but I would 
like to avoid the dispatcher currently used altogether. Perhaps putting 
gtk_threads_enter/leave around all the gtk calls, at least it's the KISS
way of solving the problem.

Memory leaks.

License:
--------
The GPL (couldn't be any other, could it?)
See License.txt for details.

Contact:
--------
Website: linuxdcpp.berlios.de
Irc: #linuxdc++ @ freenode.org


