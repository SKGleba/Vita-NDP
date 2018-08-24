TITLE_ID = SKGPEXCHK
TARGET   = PCHK
OBJS     = main.o

LIBS = -lvita2d -lSceLibKernel_stub -lSceDisplay_stub -lSceGxm_stub -lSceIme_stub \
	-lSceSysmodule_stub -lSceCtrl_stub -lScePgf_stub -lSceAppMgr_stub -lSceAppUtil_stub -lkndp_stub \
	-lSceCommonDialog_stub -lfreetype -lpng -ljpeg -lz -lm -lc -lSceRegistryMgr_stub -lSceVshBridge_stub
PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -Wall -O3
ASFLAGS = $(CFLAGS)

all: eboot.bin
	  
eboot.bin: $(TARGET).velf
	vita-make-fself $< $@

%.velf: %.elf
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
