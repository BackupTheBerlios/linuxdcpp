noinst_PROGRAMS	= dcpp
INCLUDES		= @GTKMM_CFLAGS_MANGLED@ -I..
dcpp_LDADD		= @GTKMM_LIBS_MANGLED@ -lbz2 -lz -lpthread \
	../sigc++/libsigc.a ../sigcx/libsigcx.a ../client/libdcpp.a

dcpp_SOURCES=\
accepter.cc     guiproxy.cc    privatemsg.hh      settingsdialog.hh \
accepter.hh     guiproxy.hh    publichubs.cc      sharebrowser.cc \
bookentry.cc    hub.cc         publichubs.hh      sharebrowser.hh \
bookentry.hh    hub.hh         wulfor.cc \
mainwindow.cc   search.cc      mainwindow.hh  search.hh \
guilistener.hh  privatemsg.cc  settingsdialog.cc \
util.hh		util.cc	       transfer.hh	  transfer.cc \
hashdialog.hh	hashdialog.cc
