HS = ../bind3.h ../tunnel.h ../convert.h ../ref_slot.h


all: $(HS)

../%.h: %.h.m4
	m4 $< > $@
