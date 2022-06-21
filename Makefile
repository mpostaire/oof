CC=gbdk/bin/lcc -Wl-m -Wl-j -Wm-yS
SDIR=src
ODIR=bin
BIN=oof.gb
EMU=gbmulator

all: $(BIN)

%.gb: $(SDIR)/%.c
	$(CC) -o $@ $<

run: all
	$(EMU) $(BIN)

clean:
	rm -f *.o *.lst *.map *.gb *~ *.rel *.cdb *.ihx *.lnk *.sym *.asm *.noi

.PHONY: all clean run
