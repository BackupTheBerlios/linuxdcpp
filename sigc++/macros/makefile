HS = 	../bind.h ../class_slot.h  ../method_slot.h  ../retype.h \
	../signal.h  ../bind_return.h  ../hide.h   ../object_slot.h  \
	../retype_return.h  ../slot.h

all: $(HS)

../%.h: %.h.m4
	m4 $< > $@
