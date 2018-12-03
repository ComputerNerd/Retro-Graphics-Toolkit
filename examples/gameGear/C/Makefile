CC=sdcc
SOURCE= main.c
OBJECTS=$(SOURCE:.c=.rel)
OUTPUT_BASE=main
HEX=$(OUTPUT_BASE).ihx
ROM=$(OUTPUT_BASE).gg

all: $(SOURCE) $(OBJECTS) $(HEX) $(ROM)

%.rel: %.c
	$(CC) -c -mz80 --peep-file peep-rules.txt -DTARGET_GG -o $@ $<
$(HEX): $(OBJECTS)
	$(CC) -o $(HEX) -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sms.rel $(OBJECTS) SMSlib_GG.lib
$(ROM): $(HEX)
	./ihx2sms $(HEX) $(ROM)
clean:
	rm -f $(OBJECTS) $(HEX) $(ROM)
