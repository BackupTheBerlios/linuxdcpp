GXX = g++-3.3

OBJS = \
	adaptor.o  bind.o  class_slot.o  connection.o  method_slot.o  node.o  \
	object.o object_slot.o  signal.o  slot.o

all: $(OBJS)

%.o: %.cc
	$(GXX) -I.. -c $<
	
