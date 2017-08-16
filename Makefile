TARGET = iZ80TVGame
OBJS = iZ80TVGame.o io80.o psg.o vdp.o z80emu.o libgpu.o

INCDIR = 
CFLAGS = -O3 -std=gnu99
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = iZ80TVGame
PSP_EBOOT_ICON = icon1.png

PSPSDK=$(shell psp-config --pspsdk-path)

PSPBIN = $(PSPSDK)/../bin
CFLAGS += $(shell $(PSPBIN)/sdl-config --cflags)
LIBS += $(shell $(PSPBIN)/sdl-config --libs)

include $(PSPSDK)/lib/build.mak
