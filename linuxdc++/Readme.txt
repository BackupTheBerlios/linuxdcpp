Introduction
------------
This is a broken but partially working port of DC++ to Unix-likes. The name
is in sort of flux right now, many of the sourch files says Wulfor but the
binary is called dcpp. I just don't know what to call it at the moment.
I have realized that DC-- probably would be the best name for it since
it's no way near feature complete compared to DC++.

Dependencies:
-------------
Gtkmm 2.2
libdl
pthread
zlib
libbz2
automake

Currently configures for, and successfully built using g++-3.3 but it will
probably work on other gcc versions as well. If you want to specify a specific
g++ version (if you have more than one installed) you can do like this:
CXX=g++-3.3 ./configure

Compiling:
----------
./configure
make
cd linux
./dcpp

The configure script will tell you if something's missing in a clear and
human-readable way. Hopefully. I'm not sure how well it will work, since this
is my first attempt at autoconf/automake.

If you want to generate a new configure script, do 
aclocal && automake --foreign && autoconf.
Not sure why you want to do that, but like I said I'm new to autoconf.

License:
--------
The GPL (couldn't be any other, could it?)

Contact:
--------
You can reach me at paskharen@spray.se if you have any questions.

