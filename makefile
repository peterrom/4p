4p: 4p.c makefile
	c99 -g 4p.c -o 4p

.PHONY: check
check: test4p.py 4p
	./$<

.PHONY: clean
clean:
	rm -f 4p
