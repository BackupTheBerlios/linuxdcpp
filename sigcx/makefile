GXX = g++-3.3

FLAGS = -I.. `pkg-config --cflags gtk+-2.0`

OBJS = convert.o gtk_dispatch.o thread.o timeval.o \
	dispatch.o thread-pthreads.o thread_tunnel.o util.o
	
all: $(OBJS)

%.o: %.cc
	$(GXX) $(FLAGS) -c $<
