#
# Makefile for the linux minix-filesystem routines.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

.c.s:
	$(CC) $(CFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(AS) -o $*.o $<

OBJS=	bitmap.o truncate.o namei.o inode.o \
	file.o dir.o symlink.o blkdev.o chrdev.o fifo.o

minix.o: $(OBJS)
	$(LD) -r -o minix.o $(OBJS)

clean:
	rm -f core *.o *.a *.s

dep:
	$(CPP) -M *.c > .depend

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
