CXX = g++-3.3
CC = gcc-3.3

FLAGS = -g -Wall -DXTHREADS -I.. \
	-I/usr/include/gtkmm-2.0 -I/usr/lib/gtkmm-2.0/include \
    -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 \
    -I/usr/lib/glib-2.0/include -I/usr/lib/gtk-2.0/include \
    -I/usr/X11R6/include -I/usr/include/pango-1.0 \
    -I/usr/include/freetype2 -I/usr/include/atk-1.0 
OFLAGS = -g
LIBS = -lpthread \
	-Wl,--export-dynamic -lgtkmm-2.0 -lgdkmm-2.0 -latkmm-1.0 \
    -lgtk-x11-2.0 -lpangomm-1.0 -lglibmm-2.0 -lgdk-x11-2.0 \
    -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangoxft-1.0 -lpangox-1.0 \
    -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0

OBJS = wulfor.o mainwindow.o publichubs.o bookentry.o hub.o privatemsg.o \
	settingsdialog.o sharebrowser.o search.o accepter.o guiproxy.o

CLIENT_OBJS = \
../client/ADLSearch.o          ../client/ResourceManager.o \
../client/AdcHub.o             ../client/SFVReader.o \
../client/BZUtils.o            ../client/SearchManager.o \
../client/BufferedSocket.o     ../client/ServerSocket.o \
../client/Client.o             ../client/SettingsManager.o \
../client/ClientManager.o      ../client/ShareManager.o \
../client/ConnectionManager.o  ../client/SimpleXML.o \
../client/CryptoManager.o      ../client/Socket.o \
../client/DCPlusPlus.o         ../client/StringDefs.o \
../client/DirectoryListing.o   ../client/StringTokenizer.o \
../client/DownloadManager.o    ../client/Thread.o \
../client/Encoder.o            ../client/TigerHash.o \
../client/Exception.o          ../client/TimerManager.o \
../client/FinishedManager.o    ../client/UploadManager.o \
../client/HashManager.o        ../client/User.o \
../client/HttpConnection.o     ../client/UserConnection.o \
../client/HubManager.o         ../client/Util.o \
../client/LogManager.o         ../client/ZUtils.o \
../client/NmdcHub.o            ../client/stdinc.o \
../client/QueueManager.o

BZ2_OBJS = \
../bzip2/blocksort.o  ../bzip2/crctable.o    ../bzip2/randtable.o \
../bzip2/bzlib.o      ../bzip2/decompress.o \
../bzip2/compress.o   ../bzip2/huffman.o 

Z_OBJS = \
../zlib/adler32.o   ../zlib/deflate.o  ../zlib/inftrees.o  ../zlib/zutil.o \
../zlib/compress.o  ../zlib/inffast.o  ../zlib/trees.o \
../zlib/crc32.o     ../zlib/uncompr.o

#these are for the custom version of sigx. These are compiled with a
#SEPARATE makefile in sigcx/. In the future, the build process should
#probably be changed so that each part is built by it's own makefile
#and the main makefile does $(make) on the separate files
SIGCX_OBJS = ../sigcx/convert.o       ../sigcx/thread-pthreads.o  \
	../sigcx/timeval.o	../sigcx/dispatch.o      ../sigcx/thread.o \
	../sigcx/util.o		../sigcx/gtk_dispatch.o  ../sigcx/thread_tunnel.o

#See above =)
SIGC_OBJS = ../sigc++/adaptor.o     ../sigc++/method_slot.o  \
	../sigc++/signal.o	../sigc++/bind.o        ../sigc++/node.o \
    ../sigc++/slot.o	../sigc++/class_slot.o  ../sigc++/object.o \
	../sigc++/connection.o  ../sigc++/object_slot.o 


#tell make these are virtual targets and not files it should try to build
.PHONY: all clean distclean

all: wulfor

#this is for dynamic dependancy checking, ie. recompile a .cc/.cpp if
#a .h it includes has changed
include $(OBJS:.o=.d)
include $(CLIENT_OBJS:.o=.d)
include $(BZ2_OBJS:.o=.d)
include $(Z_OBJS:.o=.d)

wulfor: $(OBJS) $(CLIENT_OBJS) $(BZ2_OBJS) $(Z_OBJS)
	$(CXX) $(OBJS) $(CLIENT_OBJS) $(BZ2_OBJS) $(Z_OBJS) $(SIGC_OBJS) $(SIGCX_OBJS) $(LIBS) -o wulfor


#these are targets for making the .o and .d (dependency) files
%.o: %.cc
	$(CXX) -c $(FLAGS) $< -o $@
%.d: %.cc
	$(CXX) $(FLAGS) -MM $< > $@

%.o: %.cpp
	$(CXX) $(OFLAGS) -c $< -o $@
%.d: %.cpp
	$(CXX) -MM $< > $@

%.o: %.c
	$(CC) $(OFLAGS) -c $< -o $@
%.d: %.c
	$(CC) -MM $< > $@

clean:
	rm wulfor $(OBJS)
	
distclean:
	rm wulfor $(OBJS) $(CLIENT_OBJS) $(BZ2_OBJS) $(Z_OBJS) \
	$(OBJS:.o=.d) $(CLIENT_OBJS:.o=.d) $(BZ2_OBJS:.o=.d) $(Z_OBJS:.o=.d)

