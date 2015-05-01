.KEEP_STATE:
CC = cc

CFLAGS= -I. -g
LDLIBS=  -lc  -L. -ljpegll -ldmedia

LIBSRC=encodeScan.s encode.c 
INCLUDES=encode.h encodeP.h
LIBOBJS=encodeScan.o encode.o

LIB=libjpegllc.a
SRC= encodeScan.s encode.c encodemain.c
OBJS= encodeScan.o  encode.o encodemain.o

default: $(LIB) test pack
test: jEncoder

jEncoder: $(OBJS) $(LIB)
	cc -o jEncoder $(OBJS) $(LDFLAGS) $(LDLIBS)


encodeScan.o : encodeScan.s
	 as encodeScan.s

encodeUtil.o : encodeUtil.c


$(LIB): $(LIBOBJS)
	rm -f $(LIB)
	ar q $(LIB) $(LIBOBJS)

$(LIBOBJS): $(LIBSRC) $(INCLUDES)

pack: $(LIB) 
	tar -cvf libjpegll.tar $(LIB) encode.h README COPYRIGHT
clean:
	rm *.o

