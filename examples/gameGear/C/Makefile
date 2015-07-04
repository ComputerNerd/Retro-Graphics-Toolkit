export PATH:=$(PATH):$(HOME)/sdcc/bin
CC=sdcc
SOURCE= main.c SMSlib.c
OBJECTS=$(SOURCE:.c=.rel)
OUTPUT_BASE=main
HEX=$(OUTPUT_BASE).ihx
ROM=$(OUTPUT_BASE).gg

all: $(SOURCE) $(OBJECTS) $(HEX) $(ROM)

%.rel: %.c
	$(CC) -c -mz80 --peep-file peep-rules.txt -DTARGET_GG -o $@ $<
$(HEX): $(OBJECTS)
	$(CC) -mz80  --data-loc 0xC000 --peep-file peep-rules.txt $(OBJECTS)
$(ROM): $(HEX)
	ihx2sms $(HEX) $(ROM)
clean:
	rm -f $(OBJECTS) $(HEX) $(ROM)
