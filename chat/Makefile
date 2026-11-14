CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

test: framing_test
	./framing_test

framing_test: framing_test.c framing.c framing.h
	$(CC) $(CFLAGS) -o $@ framing_test.c framing.c

all flash clean-fw:
	$(MAKE) -C ../firmware $(subst clean-fw,clean,$@)

clean:
	rm -f framing_test

.PHONY: test all flash clean clean-fw
