all: ch1 ch2 ch3

ch1: ch1.min

ch1.min: ch1.min.c

ch1.min.c: ch1.c
	./preprocess.py ch1.c
	@wc -l ch1.min.c

ch2: ch2.min

ch2.min: ch2.min.c

ch2.min.c: ch2.c
	./preprocess.py ch2.c
	@wc -l ch2.min.c

ch3: ch3.min

ch3.min: ch3.min.c

ch3.min.c: ch3.c
	./preprocess.py ch3.c
	@wc -l ch3.min.c

clean:
	-rm -rf *~ ch*.md *.min.c *.min

.PHONY: all clean ch1 ch2 ch3
