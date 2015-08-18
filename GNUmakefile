# config
-include Make.config
include mk/Variables.mk

# add our flags + libs
CFLAGS	+= -DVERSION='"$(VERSION)"'
LDLIBS	+= -lm

# build
TARGETS	:= lsinput input-events input-kbd input-send input-recv lircd.conf
HEADERS	:= EV.h REL.h ABS.h MSC.h LED.h SND.h REP.h KEY.h BTN.h BUS.h SW.h

# default target
all: build


#################################################################
# poor man's autoconf ;-)

include mk/Autoconf.mk

define make-config
LIB		:= $(LIB)
endef


########################################################################
# rules

build: $(TARGETS)

$(HEADERS): name.sh
	sh name.sh $* > $@

lircd.conf: lirc.sh 
	sh lirc.sh > $@

lsinput: lsinput.o input.o
input-events: input-events.o input.o
input-kbd: input-kbd.o input.o
input-send: input-send.o input.o tcp.o
input-recv: input-recv.o input.o tcp.o

input.o: input.c $(HEADERS)

install: build
	$(INSTALL_DIR) $(bindir) $(mandir)/man8
	$(INSTALL_BINARY) lsinput input-events input-kbd input-send input-recv $(bindir)
	$(INSTALL_DATA) lsinput.man $(mandir)/man8/lsinput.8
	$(INSTALL_DATA) input-kbd.man $(mandir)/man8/input-kbd.8
	$(INSTALL_DATA) input-events.man $(mandir)/man8/input-events.8


clean:
	-rm -f *.o $(depfiles)

realclean distclean: clean
	-rm -f Make.config
	-rm -f $(TARGETS) $(HEADERS) *~ xpm/*~ *.bak

#############################################

include mk/Compile.mk
include mk/Maintainer.mk
-include $(depfiles)

