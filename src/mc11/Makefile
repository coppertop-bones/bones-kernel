BIN = ../minc11

CFLAGS += -g -Wall

$(BIN): ../mc99 minc11.y
	./yacc minc11.y
	$(CC) $(CFLAGS) -o $@ y.tab.c

clean:
	rm -f yacc minc11 y.*

.PHONY: clean
