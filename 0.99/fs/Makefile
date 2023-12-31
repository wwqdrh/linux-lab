#
# Makefile for the linux filesystem.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

SUBDIRS = minix ext msdos proc isofs nfs

ifdef CONFIG_MINIX_FS
FS_SUBDIRS := $(FS_SUBDIRS) minix
endif
ifdef CONFIG_EXT_FS
FS_SUBDIRS := $(FS_SUBDIRS) ext
endif
ifdef CONFIG_MSDOS_FS
FS_SUBDIRS := $(FS_SUBDIRS) msdos
endif
ifdef CONFIG_PROC_FS
FS_SUBDIRS := $(FS_SUBDIRS) proc
endif
ifdef CONFIG_ISO9660_FS
FS_SUBDIRS := $(FS_SUBDIRS) isofs
endif
ifdef CONFIG_NFS_FS
FS_SUBDIRS := $(FS_SUBDIRS) nfs
endif

.c.s:
	$(CC) $(CFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(AS) -o $*.o $<

OBJS=	open.o read_write.o inode.o file_table.o buffer.o super.o \
	block_dev.o stat.o exec.o pipe.o namei.o fcntl.o ioctl.o \
	select.o fifo.o locks.o filesystems.o

all: fs.o filesystems.a

fs.o: $(OBJS)
	$(LD) -r -o fs.o $(OBJS)

filesystems.a: dummy
	rm -f filesystems.a
	@for i in $(FS_SUBDIRS); do [ ! -d $$i ] || \
	(cd $$i && echo $$i && $(MAKE) && $(AR) rcs ../filesystems.a $$i.o) \
	|| exit; done

clean:
	rm -f core *.s *.o *.a
	for i in $(SUBDIRS); do ([ -d $$i ] && cd $$i && $(MAKE) clean); done

depend dep:
	$(CPP) -M *.c > .depend
	for i in $(SUBDIRS); do [ ! -d $$i ] || (cd $$i && $(MAKE) dep) || exit; done

dummy:

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
