include .config

#
# ROOT_DEV specifies the default root-device when making the image.
# This can be either FLOPPY, /dev/xxxx or empty, in which case the
# default of FLOPPY is used by 'build'.
#

ROOT_DEV = /dev/hdb1

#
# uncomment the correct keyboard:
#
# The value of KBDFLAGS should be or'ed together from the following
# bits, depending on which features you want enabled.
# 0x80 - Off: the Alt key will set bit 7 if pressed together with
#             another key.
#        On:  the Alt key will NOT set the high bit; an escape
#             character is prepended instead.
# The least significant bits control if the following keys are "dead".
# The key is dead by default if the bit is on.
# 0x01 - backquote (`)
# 0x02 - accent acute
# 0x04 - circumflex (^)
# 0x08 - tilde (~)
# 0x10 - dieresis (umlaut)

KEYBOARD = -DKBD_FINNISH -DKBDFLAGS=0
# KEYBOARD = -DKBD_FINNISH_LATIN1 -DKBDFLAGS=0x9F
# KEYBOARD = -DKBD_US -DKBDFLAGS=0
# KEYBOARD = -DKBD_GR -DKBDFLAGS=0
# KEYBOARD = -DKBD_GR_LATIN1 -DKBDFLAGS=0x9F
# KEYBOARD = -DKBD_FR -DKBDFLAGS=0
# KEYBOARD = -DKBD_FR_LATIN1 -DKBDFLAGS=0x9F
# KEYBOARD = -DKBD_UK -DKBDFLAGS=0
# KEYBOARD = -DKBD_DK -DKBDFLAGS=0
# KEYBOARD = -DKBD_DK_LATIN1 -DKBDFLAGS=0x9F
# KEYBOARD = -DKBD_DVORAK -DKBDFLAGS=0
# KEYBOARD = -DKBD_SG -DKBDFLAGS=0
# KEYBOARD = -DKBD_SG_LATIN1 -DKBDFLAGS=0x9F
# KEYBOARD = -DKBD_SF -DKBDFLAGS=0
# KEYBOARD = -DKBD_SF_LATIN1 -DKBDFLAGS=0x9F
# KEYBOARD = -DKBD_NO -DKBDFLAGS=0

#
# If you want to preset the SVGA mode, uncomment the next line and
# set SVGA_MODE to whatever number you want.
# Set it to -DSVGA_MODE=NORMAL_VGA if you just want the EGA/VGA mode.
# The number is the same as you would ordinarily press at bootup.
#

SVGA_MODE=	-DSVGA_MODE=1

# 
# Edit the SOUND_SUPPORT line to suit your setup if you have configured
# the sound driver to be in the kernel (not really there yet).
#
# The DSP_BUFFSIZE defines size of the DMA buffer used for PCM voice I/O. 
# You should use one of the values 4096 (SB), 16384 (SB Pro), 32768 (PAS+) 
# or 65536 (PAS16). The SBC_IRQ defines the IRQ line used by SoundBlaster and
# the PAS_IRQ is the IRQ number for ProAudioSpectrum.
# NOTE! The ProAudioSpectrum support is not available yet.

SOUND_SUPPORT = -DKERNEL_SOUNDCARD -DDSP_BUFFSIZE=16384 -DSBC_IRQ=7 -DPAS_IRQ=5

#
# standard CFLAGS
#

CFLAGS = -Wall -O6 -fomit-frame-pointer

#
# if you want the ram-disk device, define this to be the
# size in blocks.
#

#RAMDISK = -DRAMDISK=512

AS86	=as86 -0 -a
LD86	=ld86 -0

AS	=as
LD	=ld
HOSTCC	=gcc
CC	=gcc -DKERNEL
MAKE	=make
CPP	=$(CC) -E
AR	=ar

ARCHIVES	=kernel/kernel.o mm/mm.o fs/fs.o net/net.o
FILESYSTEMS	=fs/filesystems.a
DRIVERS		=kernel/blk_drv/blk_drv.a kernel/chr_drv/chr_drv.a \
		 kernel/blk_drv/scsi/scsi.a kernel/chr_drv/sound/sound.a
MATH		=kernel/FPU-emu/math.a
LIBS		=lib/lib.a
SUBDIRS		=kernel mm fs net lib

KERNELHDRS	=/usr/src/linux/include

.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<
.s.o:
	$(AS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

all:	Version Image

lilo: Image
	if [ -f /vmlinux ]; then mv /vmlinux /vmlinux.old; fi
	cat Image > /vmlinux
	/etc/lilo/install

config:
	sh Configure < config.in

linuxsubdirs: dummy
	@for i in $(SUBDIRS); do (cd $$i && echo $$i && $(MAKE)) || exit; done

Version:
	@./makever.sh
	@echo \#define UTS_RELEASE \"0.99-`cat .version`\" > tools/version.h
	@echo \#define UTS_VERSION \"`date +%D`\" >> tools/version.h
	@echo \#define LINUX_COMPILE_TIME \"`date +%T`\" >> tools/version.h
	@echo \#define LINUX_COMPILE_BY \"`whoami`\" >> tools/version.h
	@echo \#define LINUX_COMPILE_HOST \"`hostname`\" >> tools/version.h

Image: boot/bootsect boot/setup tools/system tools/build
	cp tools/system system.tmp
	strip system.tmp
	tools/build boot/bootsect boot/setup system.tmp $(ROOT_DEV) > Image
	rm system.tmp
	sync

disk: Image
	dd bs=8192 if=Image of=/dev/fd0

tools/build: tools/build.c
	$(HOSTCC) $(CFLAGS) \
	-o tools/build tools/build.c

boot/head.o: boot/head.s

boot/head.s: boot/head.S
	$(CPP) -traditional boot/head.S -o boot/head.s

tools/version.o: tools/version.c tools/version.h

init/main.o: init/main.c
	$(CC) $(CFLAGS) $(PROFILING) -c -o $*.o $<

tools/system:	boot/head.o init/main.o tools/version.o linuxsubdirs
	$(LD) $(LDFLAGS) -M boot/head.o init/main.o tools/version.o \
		$(ARCHIVES) \
		$(FILESYSTEMS) \
		$(DRIVERS) \
		$(MATH) \
		$(LIBS) \
		-o tools/system > System.map

boot/setup: boot/setup.s
	$(AS86) -o boot/setup.o boot/setup.s
	$(LD86) -s -o boot/setup boot/setup.o

boot/setup.s:	boot/setup.S include/linux/config.h
	$(CPP) -traditional boot/setup.S -o boot/setup.s

boot/bootsect.s: boot/bootsect.S include/linux/config.h Makefile
	$(CPP) -traditional $(SVGA_MODE) $(RAMDISK) boot/bootsect.S -o boot/bootsect.s

boot/bootsect:	boot/bootsect.s
	$(AS86) -o boot/bootsect.o boot/bootsect.s
	$(LD86) -s -o boot/bootsect boot/bootsect.o

fs: dummy
	$(MAKE) linuxsubdirs SUBDIRS=fs

mm: dummy
	$(MAKE) linuxsubdirs SUBDIRS=mm

kernel: dummy
	$(MAKE) linuxsubdirs SUBDIRS=kernel

clean:
	rm -f Image System.map core boot/bootsect boot/setup \
		boot/bootsect.s boot/setup.s boot/head.s init/main.s
	rm -f init/*.o tools/system tools/build boot/*.o tools/*.o
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean); done

backup: clean
	cd .. && tar cf - linux | compress - > backup.Z
	sync

depend dep:
	for i in init/*.c;do echo -n "init/";$(CPP) -M $$i;done > .depend
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) dep) || exit; done

dummy:

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
