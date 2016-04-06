4p: 4p.c makefile
	c99 -g 4p.c -o 4p

.PHONY: check
check: 4p
	./test4p.py
