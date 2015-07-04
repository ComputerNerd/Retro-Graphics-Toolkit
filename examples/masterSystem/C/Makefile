export PATH:=$(PATH):$(HOME)/sdcc/bin
CC=sdcc
SOURCE= main.c
OBJSRC=$(SOURCE:.c=.rel)
OUTPUT_BASE=main
HEX=$(OUTPUT_BASE).ihx
ROM=$(OUTPUT_BASE).sms

all: $(SOURCE) $(OBJSRC) $(ROM)

SMSlib.rel: SMSlib.c
	$(CC) -c -mz80 --peep-file peep-rules.txt -DNO_MD_PAD_SUPPORT SMSlib.c
$(OBJSRC): $(SOURCE) SMSlib.rel
	$(CC) -mz80  --data-loc 0xC000 --peep-file peep-rules.txt main.c SMSlib.rel
$(ROM): $(OBJSRC)
	ihx2sms $(HEX) $(ROM)
clean:
	rm -f $(OBJSRC) SMSlib.rel $(HEX) $(ROM)
