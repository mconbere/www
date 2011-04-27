CHSRCS := $(shell find -E . -regex "./ch[0-9]+\.c" | tr '\n' ' ')
CHMINS := $(CHSRCS:%.c=%.min.c)
CHEXES := $(CHSRCS:%.c=%.min)
CHMDS := $(CHSRCS:%.c=%.md)

all: $(CHEXES)

%.min : %.min.c

%.min.c : %.c
	./preprocess.py $<
	@wc -l $@

clean:
	-rm -rf *~ $(CHMINS) $(CHEXES) $(CHMDS)

.PHONY: all clean 
