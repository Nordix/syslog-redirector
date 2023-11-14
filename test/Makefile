
.PHONY: all clean

all: test_uds test_file test_syslog
	LD_PRELOAD=../syslog-redirector.so timeout 5s ./test_uds
	LD_PRELOAD=../syslog-redirector.so timeout 5s ./test_file
	LD_PRELOAD=../syslog-redirector.so SYSLOG_PATH=/dev/stdout timeout 5s ./test_syslog | grep -q "test message"
	LD_PRELOAD=../syslog-redirector.so SYSLOG_PATH=file:/dev/stderr timeout 5s ./test_syslog 2>&1 | grep -q "test message"

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f test_uds test_file test_syslog
