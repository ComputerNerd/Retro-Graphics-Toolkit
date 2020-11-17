CC=sdcc
SOURCE= main.c SGlib.c
OBJECTS=$(SOURCE:.c=.rel)
OUTPUT_BASE=main
HEX=$(OUTPUT_BASE).ihx
ROM=$(OUTPUT_BASE).sg

all: $(SOURCE) $(OBJECTS) $(HEX) $(ROM)

%.rel: %.c
	$(CC) -c -mz80 --peep-file peep-rules.txt -o $@ $<
$(HEX): $(OBJECTS)
	$(CC) -o $(HEX) -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sg.rel $(OBJECTS)
$(ROM): $(HEX)
	./ihx2sms $(HEX) $(ROM)
clean:
	rm -f $(OBJECTS) $(HEX) $(ROM)
