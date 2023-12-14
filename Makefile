
TARGET = syslog-redirector.so

# https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++
## TODO: fix warnings
CFLAGS ?= -O2 -Wall -Wformat=2 -Wconversion -Wtrampolines -Wimplicit-fallthrough \
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
-D_GLIBCXX_ASSERTIONS \
-fstrict-flex-arrays=3 \
-fstack-clash-protection -fstack-protector-strong \
-Wl,-z,nodlopen -Wl,-z,noexecstack \
-Wl,-z,relro -Wl,-z,now

LDFLAGS ?= -shared -fPIC -ldl -lpthread

.PHONY: all clean test

all: $(TARGET)

$(TARGET): src/syslog.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

test: $(TARGET)
	$(MAKE) -C $@ CFLAGS="$(CFLAGS)" CC="$(CC)"

clean:
	rm -f $(TARGET)
	$(MAKE) -C test clean

clang-format:
	for f in $(shell find . -name '*.c' -o -name '*.h'); do clang-format --verbose -i $$f; done
