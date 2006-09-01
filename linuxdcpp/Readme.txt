Introduction
------------
LinuxDC++ is a Linux port of the Direct Connect client DC++. Though it is
primarily aimed at Linux, it has been shown to work on other Unix-based
operating systems as well. It is written in C++ and makes use of GTK+ for the
user-interface. LinuxDC++ is free and open source software licensed under the
GPL.

Direct connect is a peer-to-peer file-sharing protocol. The most popular Windows
client implementing this protocol currently is DC++. Direct connect clients
connect to a central hub where they can view a list of clients or users
connected to them. Users can search for files and download them from other
clients, as well as chat with other users.

This readme contains only the basic information. For more detailed instructions,
read the manual in the wiki:

http://openfacts.berlios.de/index-en.phtml?title=Ldcpp_Manual


Dependencies:
-------------
scons >= 0.96
pkg-config
g++ >= 3.4
gtk+-2.0 >= 2.6
gthread-2.0 >= 2.4
libglade-2.0 >= 2.4
pthread
zlib
libbz2


Compiling:
----------
$ cd /path/to/linuxdcpp
$ scons PREFIX=/path/to/install

Scons will output if something's missing in a clear and human-readable way.
The PREFIX parameter is the path that the program will be installed to. PREFIX
defaults to /usr/local, so it need only be specified if a different
directory is desired. Note the program doesn't have to be installed in order to
be ran. See the second part of the Running section for more details.

For compile options, type "scons --help".


Installing:
-----------
$ scons install

This command may need to be ran as root (sudo, su, etc.) depending upon
the PREFIX path that was supplied when compiling.


Running:
--------
If LinuxDC++ was installed using scons install, simply run the executable:

$ linuxdcpp

To run the program from the source directory instead of installing, first
navigate to that directory then run "./linuxdcpp". This is an important
point: scripts calling linuxdcpp in the source directory have to first cd to
that path in order to work (since it will look for pixmaps using the path
"./pixmaps" and this will fail if it's not in the correct directory).
Linuxdcpp no longer dynamically finds the location of the binary using
binreloc (since it wasn't portable).


Known problems:
------------------------
Utf8 locales are currently very buggy/not supported. If for example a lot of name
and description fields in the public hub list are empty you are suffering from this.
A workaround is to specify a non-utf8 locale when running the program, assuming
you have a non-utf8 locale on your system. For example, I'm from Sweden so my
utf8 locale would be sv_SE.UTF8. Starting the program like this:
prompt# LANG=sv_SE.ISO-8859-1 ./linuxdcpp would run LinuxDC++ with a non-utf8 locale.

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

