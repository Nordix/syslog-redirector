
TARGET = syslog-redirector.so

CFLAGS ?= -Wall -Werror
LDFLAGS ?= -shared -fPIC -ldl -lpthread

.PHONY: all clean test

all: $(TARGET)

$(TARGET): src/syslog.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

test: $(TARGET)
	$(MAKE) -C $@

clean:
	rm -f $(TARGET)
	$(MAKE) -C test clean
