all: ch1 ch2

ch1: ch1.min

ch1.min: ch1.min.c

ch1.min.c: ch1.c
	./preprocess.py ch1.c

ch2: ch2.min

ch2.min: ch2.min.c

ch2.min.c: ch2.c
	./preprocess.py ch2.c

clean:
	-rm -rf *~ *.md *.min.c ch1 ch2

.PHONY: all clean ch1 ch2
