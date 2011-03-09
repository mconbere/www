all: ch1

ch1: ch1.min

ch1.min: ch1.min.c

ch1.min.c: ch1.c
	./preprocess.py ch1.c

clean:
	-rm -rf *~ *.md *.min.c ch1 

.PHONY: all clean ch1